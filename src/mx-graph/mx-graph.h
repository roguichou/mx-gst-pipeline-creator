#ifndef _MX_GRAPH_H_
#define _MX_GRAPH_H_

#include <mx/mx.h>

G_BEGIN_DECLS

#define MX_TYPE_GRAPH            (mx_graph_get_type())

#define MX_GRAPH(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), \
  MX_TYPE_GRAPH, MxGraph))

#define MX_IS_GRAPH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
  MX_TYPE_GRAPH))

#define MX_GRAPH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_GRAPH, MxGraphClass))

#define MX_IS_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_GRAPH))

#define MX_GRAPH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),  \
  MX_TYPE_GRAPH, MxGraph))

typedef struct _MxGraph        MxGraph;
typedef struct _MxGraphClass   MxGraphClass;
typedef struct _MxGraphPrivate MxGraphPrivate;

struct _MxGraph
{
  ClutterGroup    parent_instance;
  MxGraphPrivate *priv;
};

struct _MxGraphClass
{
  ClutterGroupClass parent_class;
};

GType mx_graph_get_type (void) G_GNUC_CONST;

MxGraph *
mx_graph_new(void);

G_END_DECLS

#endif //_MX_GRAPH_H_
