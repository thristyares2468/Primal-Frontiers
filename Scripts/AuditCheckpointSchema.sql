-- Prepared metadata-only audit; not a migration. Run only on the intended development branch.
-- PostgreSQL18 references: /docs/18/functions-info.html and /docs/18/sql-set-transaction.html.
-- No saved payloads, credentials, grants, DDL, writes or arbitrary functions are selected.
BEGIN TRANSACTION READ ONLY;
SET LOCAL statement_timeout = '5s';
SET LOCAL lock_timeout = '1s';

SELECT current_database() AS database_name,
       current_user AS inspection_role,
       current_setting('server_version') AS server_version,
       current_setting('transaction_read_only') AS transaction_read_only,
       to_regclass('public.pf_checkpoints') AS checkpoint_table;

SELECT n.nspname AS schema_name, c.relname AS relation_name, c.relkind AS relation_kind,
       pg_get_userbyid(c.relowner) AS owner_role,
       c.relrowsecurity AS row_security, c.relforcerowsecurity AS forced_row_security
FROM pg_catalog.pg_class c
JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
WHERE n.nspname = 'public' AND c.relname IN ('pf_checkpoints', 'pf_world_leases')
ORDER BY c.relname;

SELECT a.attname AS column_name, format_type(a.atttypid, a.atttypmod) AS data_type,
       a.attnotnull AS not_null, pg_get_expr(d.adbin, d.adrelid) AS default_expression
FROM pg_catalog.pg_attribute a
LEFT JOIN pg_catalog.pg_attrdef d ON d.adrelid = a.attrelid AND d.adnum = a.attnum
WHERE a.attrelid = to_regclass('public.pf_checkpoints')
  AND a.attnum > 0 AND NOT a.attisdropped
ORDER BY a.attnum;

SELECT conname AS constraint_name, contype AS constraint_type,
       pg_get_constraintdef(oid, true) AS definition
FROM pg_catalog.pg_constraint
WHERE conrelid = to_regclass('public.pf_checkpoints')
ORDER BY conname;

SELECT indexname, indexdef
FROM pg_catalog.pg_indexes
WHERE schemaname = 'public' AND tablename = 'pf_checkpoints'
ORDER BY indexname;

SELECT rolname, rolsuper, rolcreatedb, rolcreaterole, rolcanlogin
FROM pg_catalog.pg_roles
WHERE rolname IN (current_user, 'pf_app')
ORDER BY rolname;

-- OID overloads avoid throwing if the expected role/table is absent in a wrong branch.
SELECT r.rolname,
       has_schema_privilege(r.oid, n.oid, 'USAGE') AS schema_usage,
       has_schema_privilege(r.oid, n.oid, 'CREATE') AS schema_create,
       has_table_privilege(r.oid, c.oid, 'SELECT') AS checkpoint_select,
       has_table_privilege(r.oid, c.oid, 'INSERT') AS checkpoint_insert,
       has_table_privilege(r.oid, c.oid, 'UPDATE') AS checkpoint_update,
       has_table_privilege(r.oid, c.oid, 'DELETE') AS checkpoint_delete,
       has_table_privilege(r.oid, c.oid, 'TRUNCATE') AS checkpoint_truncate
FROM pg_catalog.pg_roles r
JOIN pg_catalog.pg_namespace n ON n.nspname = 'public'
LEFT JOIN pg_catalog.pg_class c ON c.oid = to_regclass('public.pf_checkpoints')
WHERE r.rolname IN (current_user, 'pf_app')
ORDER BY r.rolname;

SELECT polname AS policy_name, polcmd AS command_type, polpermissive AS permissive,
       cardinality(polroles) AS role_count
FROM pg_catalog.pg_policy
WHERE polrelid = to_regclass('public.pf_checkpoints')
ORDER BY polname;

COMMIT;
