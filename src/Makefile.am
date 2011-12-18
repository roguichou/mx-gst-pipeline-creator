NULL =

SUBDIRS=mx-graph

bin_PROGRAMS = mx-gpc
mx_gpc_SOURCES = main.c

mx_gpc_LDADD = \
			 -Lmx-graph/.libs/ -lmx-graph \
			 $(DEPS_LIBS)

mx_gpc_CFLAGS = \
				-Imx-graph/ \
			  $(DEPS_CFLAGS) \
			  -O0 -g3 -Wall -Werror

