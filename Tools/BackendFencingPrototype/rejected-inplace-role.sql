-- REJECTED FOR NEON: owner result shows unmanageable cloud_admin grantor.
-- Offline failure-safety fixture ONLY. Never run this on the live project.
-- OPERATOR: run ONLY on Development br-red-fog-a7mp3ux7/neondb as neondb_owner.
-- SQL cannot prove the Neon branch; verify it in the Console before running.
-- Transactional privilege correction, not a migration, password reset or data edit.
-- Tested locally; Neon-specific administrator capabilities still require live verification.
BEGIN;
SET LOCAL statement_timeout = '5s';
SET LOCAL lock_timeout = '1s';
DO $pf_role$
DECLARE
  app pg_catalog.pg_roles%ROWTYPE;
  checkpoint oid := to_regclass('public.pf_checkpoints');
  membership record;
BEGIN
  IF current_database() <> 'neondb' OR current_user <> 'neondb_owner' THEN
    RAISE EXCEPTION 'PF_ROLE_REFUSED: select Development/neondb and neondb_owner';
  END IF;
  SELECT * INTO app FROM pg_catalog.pg_roles WHERE rolname = 'pf_app';
  IF NOT FOUND OR app.rolsuper OR app.rolreplication OR app.rolbypassrls OR NOT app.rolcanlogin THEN
    RAISE EXCEPTION 'PF_ROLE_REFUSED: missing role or unexpected super/replication/bypass/login flags';
  END IF;
  IF checkpoint IS NULL OR NOT EXISTS (
    SELECT 1 FROM pg_catalog.pg_class WHERE oid = checkpoint AND relkind = 'r'
      AND pg_get_userbyid(relowner) = 'neondb_owner'
      AND NOT relrowsecurity AND NOT relforcerowsecurity
  ) THEN
    RAISE EXCEPTION 'PF_ROLE_REFUSED: checkpoint table ownership/type/RLS differs from audit';
  END IF;
  IF EXISTS (
    SELECT 1 FROM pg_catalog.pg_shdepend
    WHERE refclassid = 'pg_catalog.pg_authid'::regclass AND refobjid = app.oid AND deptype = 'o'
  ) THEN
    RAISE EXCEPTION 'PF_ROLE_REFUSED: pf_app owns objects; inventory dependencies before changing grants';
  END IF;
  IF EXISTS (
    SELECT 1 FROM pg_catalog.pg_auth_members m
    JOIN pg_catalog.pg_roles parent ON parent.oid = m.roleid
    WHERE m.member = app.oid AND parent.rolname <> 'neon_superuser'
  ) THEN
    RAISE EXCEPTION 'PF_ROLE_REFUSED: unexpected app membership; review instead of broad revocation';
  END IF;

  FOR membership IN
    SELECT m.grantor FROM pg_catalog.pg_auth_members m
    JOIN pg_catalog.pg_roles parent ON parent.oid = m.roleid
    WHERE m.member = app.oid AND parent.rolname = 'neon_superuser'
  LOOP
    IF NOT pg_has_role(current_user, membership.grantor, 'SET') THEN
      RAISE EXCEPTION 'PF_ROLE_REFUSED: membership grantor cannot be managed by this inspection role';
    END IF;
  END LOOP;

  ALTER ROLE pf_app NOCREATEDB NOCREATEROLE;
  FOR membership IN
    SELECT m.grantor FROM pg_catalog.pg_auth_members m
    JOIN pg_catalog.pg_roles parent ON parent.oid = m.roleid
    WHERE m.member = app.oid AND parent.rolname = 'neon_superuser'
  LOOP
    EXECUTE format('REVOKE neon_superuser FROM pf_app GRANTED BY %I RESTRICT', pg_get_userbyid(membership.grantor));
  END LOOP;
  -- RESTRICT: never cascade away another role's dependent grants.
  REVOKE ALL PRIVILEGES ON TABLE public.pf_checkpoints FROM pf_app RESTRICT;
  GRANT SELECT, INSERT ON TABLE public.pf_checkpoints TO pf_app;
  GRANT CONNECT ON DATABASE neondb TO pf_app;
  GRANT USAGE ON SCHEMA public TO pf_app;

  SELECT * INTO app FROM pg_catalog.pg_roles WHERE rolname = 'pf_app';
  IF app.rolcreatedb OR app.rolcreaterole OR app.rolsuper OR app.rolreplication OR app.rolbypassrls
    OR EXISTS (SELECT 1 FROM pg_catalog.pg_auth_members WHERE member = app.oid)
    OR NOT has_database_privilege(app.oid, current_database(), 'CONNECT')
    OR has_database_privilege(app.oid, current_database(), 'CREATE')
    OR NOT has_schema_privilege(app.oid, 'public', 'USAGE')
    OR has_schema_privilege(app.oid, 'public', 'CREATE')
    OR NOT has_table_privilege(app.oid, checkpoint, 'SELECT')
    OR NOT has_table_privilege(app.oid, checkpoint, 'INSERT')
    OR has_table_privilege(app.oid, checkpoint, 'UPDATE,DELETE,TRUNCATE,REFERENCES,TRIGGER,MAINTAIN')
    OR has_any_column_privilege(app.oid, checkpoint, 'UPDATE,REFERENCES')
    OR has_table_privilege(app.oid, checkpoint, 'SELECT WITH GRANT OPTION,INSERT WITH GRANT OPTION') THEN
    RAISE EXCEPTION 'PF_ROLE_REFUSED: effective privilege postcondition failed; transaction must roll back'
      USING DETAIL = jsonb_build_object(
        'createdb', app.rolcreatedb, 'createrole', app.rolcreaterole,
        'memberships', (SELECT count(*) FROM pg_catalog.pg_auth_members WHERE member = app.oid),
        'db_connect', has_database_privilege(app.oid,current_database(),'CONNECT'),
        'db_create', has_database_privilege(app.oid,current_database(),'CREATE'),
        'schema_usage', has_schema_privilege(app.oid,'public','USAGE'),
        'schema_create', has_schema_privilege(app.oid,'public','CREATE'),
        'read', has_table_privilege(app.oid,checkpoint,'SELECT'),
        'insert', has_table_privilege(app.oid,checkpoint,'INSERT'),
        'mutation', has_table_privilege(app.oid,checkpoint,'UPDATE,DELETE,TRUNCATE,REFERENCES,TRIGGER,MAINTAIN'),
        'column_mutation', has_any_column_privilege(app.oid,checkpoint,'UPDATE,REFERENCES'),
        'grant_option', has_table_privilege(app.oid,checkpoint,'SELECT WITH GRANT OPTION,INSERT WITH GRANT OPTION')
      )::text;
  END IF;
END
$pf_role$;
COMMIT;
SELECT 'PF_ROLE_RESTRICTED' AS status,
       rolname AS app_role, rolcreatedb AS can_create_database, rolcreaterole AS can_create_roles,
       has_table_privilege(oid, 'public.pf_checkpoints', 'SELECT') AS can_read,
       has_table_privilege(oid, 'public.pf_checkpoints', 'INSERT') AS can_insert,
       has_table_privilege(oid, 'public.pf_checkpoints', 'UPDATE') AS can_update,
       has_table_privilege(oid, 'public.pf_checkpoints', 'DELETE') AS can_delete,
       has_table_privilege(oid, 'public.pf_checkpoints', 'TRUNCATE') AS can_truncate
FROM pg_catalog.pg_roles WHERE rolname = 'pf_app';
