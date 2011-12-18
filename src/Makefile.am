NULL =

SUBDIRS=ihm

bin_PROGRAMS = mx-gpc
mx_gpc_SOURCES = main.c

mx_gpc_LDADD = \
			 -Lihm/.libs/ -lmx-gpc-ihm \
			 $(DEPS_LIBS)

mx_gpc_CFLAGS = \
				-Iihm/ \
			  $(DEPS_CFLAGS) \
			  -O0 -g3 -Wall -Werror

