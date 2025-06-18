# contrib/plaudit/Makefile

MODULE_big = plaudit
OBJS = plaudit.o $(WIN32RES)

EXTENSION = plaudit
DATA = plaudit--1.0.sql
PGFILEDESC = "plaudit - An audit logging extension for PostgreSQL"

REGRESS = plaudit
REGRESS_OPTS = --temp-config=$(top_srcdir)/contrib/plaudit/plaudit.conf

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/plaudit
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

EXTRA_INSTALL += contrib/pg_stat_statements
