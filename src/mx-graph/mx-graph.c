#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-graph.h"
#include "mx-graph-element.h"

#define MX_GRAPH_GET_PRIVATE(obj)               \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj),          \
                                MX_TYPE_GRAPH,  \
                                MxGraphPrivate))

struct _MxGraphPrivate
{
  gpointer dummy;
};

static void _container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxGraph, mx_graph, CLUTTER_TYPE_GROUP,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                _container_iface_init));

static void
_real_remove (ClutterContainer *container,
              ClutterActor     *actor)
{
  ClutterContainerIface *iface_class, *old_iface_class;

  iface_class = G_TYPE_INSTANCE_GET_INTERFACE(container, 
                                              CLUTTER_TYPE_CONTAINER,
                                              ClutterContainerIface);
  old_iface_class = g_type_interface_peek_parent(iface_class);

  old_iface_class->remove(container, actor);

  //.TODO.
}

static void
_real_add (ClutterContainer *container,
           ClutterActor     *actor)
{
  ClutterContainerIface *iface_class, *old_iface_class;

  iface_class = G_TYPE_INSTANCE_GET_INTERFACE(container, 
                                              CLUTTER_TYPE_CONTAINER,
                                              ClutterContainerIface);
  old_iface_class = g_type_interface_peek_parent(iface_class);

  old_iface_class->add(container, actor);

  if (MX_IS_GRAPH_ELEMENT(actor))
  {
    MxGraphElement *elt = MX_GRAPH_ELEMENT(actor);
    mx_graph_element_set_graph(elt, CLUTTER_ACTOR(container));
  }

  //.TODO.
}

static void
_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = _real_add;
  iface->remove = _real_remove;
}

static void
mx_graph_class_init (MxGraphClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGraphPrivate));
  //.TODO.
}

static void
mx_graph_init (MxGraph *graph)
{
  graph->priv = MX_GRAPH_GET_PRIVATE(graph);
  //.TODO.
}

