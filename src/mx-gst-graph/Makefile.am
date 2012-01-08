NULL = 

lib_LTLIBRARIES = libmx-gst-graph.la

libmx_gst_graph_la_SOURCES =      \
			   mx-gst-graph-element.c   \
			   mx-gst-graph-element-pad.c

libmx_gst_graph_la_LIBADD = \
			 -L../mx-graph/.libs/ -lmx-graph\
			 $(DEPS_LIBS)

libmx_gst_graph_la_CFLAGS = \
			  $(DEPS_CFLAGS) \
				-I../mx-graph/ \
			  -O0 -g3 -Wall -Werror

