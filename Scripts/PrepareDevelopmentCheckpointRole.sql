-- Run only on Development br-red-fog-a7mp3ux7/neondb as neondb_owner.
-- Creates a restricted SQL-owned staging role, NOLOGIN and no password.
-- Existing managed pf_app/Worker connection/data remain unchanged.
-- SQL cannot prove the Neon branch; verify it in the Console first.
BEGIN;
SET LOCAL statement_timeout = '5s';
SET LOCAL lock_timeout = '1s';
DO $pf_prepare$
DECLARE
  app pg_catalog.pg_roles%ROWTYPE;
  checkpoint oid := to_regclass('public.pf_checkpoints');
BEGIN
  IF current_database() <> 'neondb' OR current_user <> 'neondb_owner' THEN
    RAISE EXCEPTION 'PF_ROLE_REFUSED: select Development/neondb and neondb_owner';
  END IF;
  IF checkpoint IS NULL OR NOT EXISTS (
    SELECT 1 FROM pg_catalog.pg_class WHERE oid=checkpoint AND relkind='r'
      AND pg_get_userbyid(relowner)='neondb_owner' AND NOT relrowsecurity AND NOT relforcerowsecurity
  ) THEN
    RAISE EXCEPTION 'PF_ROLE_REFUSED: checkpoint table differs from audited ownership/type/RLS';
  END IF;
  SELECT * INTO app FROM pg_catalog.pg_roles WHERE rolname='pf_checkpoint_app';
  IF NOT FOUND THEN
    CREATE ROLE pf_checkpoint_app NOLOGIN NOCREATEDB NOCREATEROLE;
  ELSE
    IF app.rolsuper OR app.rolcreatedb OR app.rolcreaterole OR app.rolreplication OR app.rolbypassrls OR app.rolcanlogin
      OR EXISTS(SELECT 1 FROM pg_catalog.pg_auth_members WHERE member=app.oid)
      OR EXISTS(SELECT 1 FROM pg_catalog.pg_shdepend
        WHERE refclassid='pg_catalog.pg_authid'::regclass AND refobjid=app.oid AND deptype='o')
      OR NOT EXISTS(SELECT 1 FROM pg_catalog.pg_auth_members
        WHERE roleid=app.oid AND member=(SELECT oid FROM pg_catalog.pg_roles WHERE rolname=current_user) AND admin_option) THEN
      RAISE EXCEPTION 'PF_ROLE_REFUSED: existing staging role is active, privileged or not managed by this owner';
    END IF;
  END IF;
  GRANT CONNECT ON DATABASE neondb TO pf_checkpoint_app;
  GRANT USAGE ON SCHEMA public TO pf_checkpoint_app;
  GRANT SELECT, INSERT ON TABLE public.pf_checkpoints TO pf_checkpoint_app;

  SELECT * INTO app FROM pg_catalog.pg_roles WHERE rolname='pf_checkpoint_app';
  IF app.rolsuper OR app.rolcreatedb OR app.rolcreaterole OR app.rolreplication OR app.rolbypassrls OR app.rolcanlogin
    OR EXISTS(SELECT 1 FROM pg_catalog.pg_auth_members WHERE member=app.oid)
    OR NOT has_database_privilege(app.oid,current_database(),'CONNECT')
    OR has_database_privilege(app.oid,current_database(),'CREATE')
    OR NOT has_schema_privilege(app.oid,'public','USAGE')
    OR has_schema_privilege(app.oid,'public','CREATE')
    OR NOT has_table_privilege(app.oid,checkpoint,'SELECT')
    OR NOT has_table_privilege(app.oid,checkpoint,'INSERT')
    OR has_table_privilege(app.oid,checkpoint,'UPDATE,DELETE,TRUNCATE,REFERENCES,TRIGGER,MAINTAIN')
    OR has_any_column_privilege(app.oid,checkpoint,'UPDATE,REFERENCES')
    OR has_table_privilege(app.oid,checkpoint,'SELECT WITH GRANT OPTION,INSERT WITH GRANT OPTION') THEN
    RAISE EXCEPTION 'PF_ROLE_REFUSED: effective staging privileges differ; roll back instead of broad revocation';
  END IF;
END
$pf_prepare$;
COMMIT;
SELECT 'PF_RESTRICTED_ROLE_PREPARED' AS status, rolname AS app_role,
       rolcanlogin AS login_enabled, rolcreatedb AS can_create_database, rolcreaterole AS can_create_roles,
       has_table_privilege(oid,'public.pf_checkpoints','SELECT') AS can_read,
       has_table_privilege(oid,'public.pf_checkpoints','INSERT') AS can_insert,
       has_table_privilege(oid,'public.pf_checkpoints','UPDATE') AS can_update,
       has_table_privilege(oid,'public.pf_checkpoints','DELETE') AS can_delete
FROM pg_catalog.pg_roles WHERE rolname='pf_checkpoint_app';
