-- Run ONCE on Development br-red-fog-a7mp3ux7/neondb as neondb_owner.
-- PRIVATE RESULT: neon_database_url contains the new password. Do not post it.
-- Existing pf_app/Worker connection/data are not changed by this script.
-- Keep the result open/copy it privately; rerunning refuses to rotate an active login.
BEGIN;
SET LOCAL statement_timeout = '5s';
SET LOCAL lock_timeout = '1s';
SET LOCAL password_encryption = 'scram-sha-256';
DO $pf_activate$
DECLARE
  app pg_catalog.pg_roles%ROWTYPE;
  checkpoint oid := to_regclass('public.pf_checkpoints');
  new_password text;
BEGIN
  IF current_database() <> 'neondb' OR current_user <> 'neondb_owner' THEN
    RAISE EXCEPTION 'PF_LOGIN_REFUSED: select Development/neondb and neondb_owner';
  END IF;
  SELECT * INTO app FROM pg_catalog.pg_roles WHERE rolname='pf_checkpoint_app';
  IF NOT FOUND THEN
    RAISE EXCEPTION 'PF_LOGIN_REFUSED: restricted staging role must be prepared first';
  END IF;
  IF app.rolcanlogin THEN
    RAISE EXCEPTION 'PF_LOGIN_REFUSED: login already enabled; do not rotate a live credential by rerunning';
  END IF;
  IF app.rolsuper OR app.rolcreatedb OR app.rolcreaterole OR app.rolreplication OR app.rolbypassrls
    OR EXISTS(SELECT 1 FROM pg_catalog.pg_auth_members WHERE member=app.oid)
    OR EXISTS(SELECT 1 FROM pg_catalog.pg_shdepend
      WHERE refclassid='pg_catalog.pg_authid'::regclass AND refobjid=app.oid AND deptype='o')
    OR NOT EXISTS(SELECT 1 FROM pg_catalog.pg_auth_members
      WHERE roleid=app.oid AND member=(SELECT oid FROM pg_catalog.pg_roles WHERE rolname=current_user) AND admin_option) THEN
    RAISE EXCEPTION 'PF_LOGIN_REFUSED: staging role privilege/ownership/administration differs';
  END IF;
  IF checkpoint IS NULL OR NOT EXISTS(SELECT 1 FROM pg_catalog.pg_class
      WHERE oid=checkpoint AND relkind='r' AND pg_get_userbyid(relowner)='neondb_owner'
        AND NOT relrowsecurity AND NOT relforcerowsecurity)
    OR NOT has_database_privilege(app.oid,current_database(),'CONNECT')
    OR has_database_privilege(app.oid,current_database(),'CREATE')
    OR NOT has_schema_privilege(app.oid,'public','USAGE')
    OR has_schema_privilege(app.oid,'public','CREATE')
    OR NOT has_table_privilege(app.oid,checkpoint,'SELECT')
    OR NOT has_table_privilege(app.oid,checkpoint,'INSERT')
    OR has_table_privilege(app.oid,checkpoint,'UPDATE,DELETE,TRUNCATE,REFERENCES,TRIGGER,MAINTAIN')
    OR has_any_column_privilege(app.oid,checkpoint,'UPDATE,REFERENCES')
    OR has_table_privilege(app.oid,checkpoint,'SELECT WITH GRANT OPTION,INSERT WITH GRANT OPTION') THEN
    RAISE EXCEPTION 'PF_LOGIN_REFUSED: effective checkpoint privileges differ from reviewed staging';
  END IF;
  -- Two random v4 UUIDs: 64 hex characters, 244 random bits. Built-in PG18,
  -- no extension or literal password in source/query history is required.
  new_password := replace(gen_random_uuid()::text || gen_random_uuid()::text, '-', '');
  EXECUTE format('ALTER ROLE pf_checkpoint_app LOGIN PASSWORD %L', new_password);
  -- Transaction-local only; cleared on commit/rollback, never session-persistent.
  PERFORM set_config('pf.private_checkpoint_url',
    'postgresql://pf_checkpoint_app:' || new_password ||
    '@ep-autumn-cell-a7d9uojb-pooler.ap-southeast-2.aws.neon.tech/neondb?sslmode=require', true);
END
$pf_activate$;
-- This is the sole private result row. Ensure COMMIT succeeds with no errors.
SELECT 'PF_CHECKPOINT_LOGIN_ENABLED' AS status,
       current_setting('pf.private_checkpoint_url') AS neon_database_url;
COMMIT;
