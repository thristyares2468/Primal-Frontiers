-- Read-only metadata only. Owner selects Development br-red-fog-a7mp3ux7/neondb.
SELECT
    pg_get_userbyid(m.roleid) AS granted_role,
    pg_get_userbyid(m.member) AS member,
    pg_get_userbyid(m.grantor) AS granted_by,
    m.admin_option,
    m.inherit_option,
    m.set_option
FROM pg_catalog.pg_auth_members m
WHERE m.member = (SELECT oid FROM pg_catalog.pg_roles WHERE rolname = 'pf_app')
   OR m.roleid = (SELECT oid FROM pg_catalog.pg_roles WHERE rolname = 'pf_app')
   OR (
       m.member = (SELECT oid FROM pg_catalog.pg_roles WHERE rolname = 'neondb_owner')
       AND m.roleid = (SELECT oid FROM pg_catalog.pg_roles WHERE rolname = 'neon_superuser')
   );
