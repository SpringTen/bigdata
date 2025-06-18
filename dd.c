## Introduction

The goal of the plaudit is to provide PolarDB users with capability to produce audit logs often required to comply with government, financial, or ISO certifications.

## Usage Considerations

Depending on settings, it is possible for plaudit to generate an enormous volume of logging. Be careful to determine exactly what needs to be audit logged in your environment to avoid logging too much.

## Compile and Install

Go to plaudit directory:
```
cd plaudit
```
Build and install plaudit:
```
make install USE_PGXS=1 PG_CONFIG=/usr/pgsql/bin/pg_config
```

## Settings

Settings may be modified only by a superuser. Allowing common users to change their settings would defeat the point of an audit log.

Settings can be specified globally (in `postgresql.conf` or using `ALTER SYSTEM ... SET`), at the database level (using `ALTER DATABASE ... SET`), or at the role level (using `ALTER ROLE ... SET`). Note that settings are not inherited through normal role inheritance and `SET ROLE` will not alter a user's plaudit settings. This is a limitation of the roles system and not inherent to plaudit.

The plaudit extension must be loaded in [shared_preload_libraries]. Otherwise, an error will be raised at load time and no audit logging will occur.

In addition, `CREATE EXTENSION plaudit` must be called before `plaudit.log` is set to ensure proper plaudit functionality.

If the `plaudit` extension is dropped and needs to be recreated then `plaudit.log` must be unset first otherwise an error will be raised.

### plaudit.log

Specifies which classes of statements will be logged. Possible values are:

- **READ**: `SELECT` and `COPY` when the source is a relation or a query.

- **WRITE**: `INSERT`, `UPDATE`, `DELETE`, `TRUNCATE`, and `COPY` when the destinations is a relation.

- **FUNCTION**: Function calls and `DO` blocks.

- **ROLE**: Statements related to roles and privileges: `GRANT`, `REVOKE`, `CREATE/ALTER/DROP ROLE`.

- **DDL**: All `DDL` that is not included in the `ROLE` class.

- **MISC**: Miscellaneous commands, e.g. `DISCARD`, `FETCH`, `CHECKPOINT`, `VACUUM`, `SET`.

- **MISC_SET**: Miscellaneous `SET` commands, e.g. `SET ROLE`.

- **ALL**: Include all of the above.

Multiple classes can be provided using a comma-separated list and classes can be subtracted by prefacing the class with a `-` label.

The default value is `none`.

### plaudit.log_catalog

Specifies that logging should be enabled in the case where all relations in a statement are in pg_catalog. Disabling this setting will reduce noise in the log from tools like psql and PgAdmin that query the catalog heavily.

The default value is `on`.

### plaudit.log_relation

Specifies whether audit logging should create a separate log entry for each relation (`TABLE`, `VIEW`, etc.) referenced in a `SELECT` or `DML` statement.

The default value is `off`.

### plaudit.role

Specifies the master role to use for audit logging. Multiple audit roles can be defined by granting the master role to them. Only master role or its members can write the audit log.

There is no default value.
