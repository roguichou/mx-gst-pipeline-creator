#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-graph.h"
#include "mx-graph-element.h"
#include "mx-graph-element-pad.h"
#include "mx-graph-element-link.h"

#define MX_GRAPH_GET_PRIVATE(obj)               \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj),          \
                                MX_TYPE_GRAPH,  \
                                MxGraphPrivate))

struct _MxGraphPrivate
{
  MxGraphElementLink *link;
  MxGraphElement     *elt;
  MxGraphElementPad  *pad;
  gulong              capture_event_signal_id;
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

static gboolean
mx_graph_element_pad_on_stage_capture (ClutterActor *stage,
                                       ClutterEvent *event,
                                       MxGraph      *graph)
{
  gfloat myx, myy;
  clutter_actor_get_transformed_position (CLUTTER_ACTOR(graph), &myx, &myy);
  MxGraphPrivate *priv = graph->priv;
  g_return_val_if_fail (NULL != priv->link, FALSE);
  switch (event->type)
  {
    case CLUTTER_BUTTON_RELEASE:
      {
        gfloat evtx, evty,
               padx, pady, 
               padw, padh;
        clutter_event_get_coords (event, &evtx, &evty);
        clutter_actor_get_transformed_position (CLUTTER_ACTOR(priv->pad),
            &padx, &pady);
        clutter_actor_get_size (CLUTTER_ACTOR(priv->pad), &padw, &padh);
        gfloat relx = evtx-padx,
               rely = evty-pady;
        if (relx>0 && relx<padw && rely>0 && rely<padh)
        {
          clutter_container_remove_actor (CLUTTER_CONTAINER (graph), 
              CLUTTER_ACTOR(priv->link));
          goto end_release;
        }
        ClutterActor *picked = clutter_stage_get_actor_at_pos (
            CLUTTER_STAGE(clutter_stage_get_default()),
            CLUTTER_PICK_REACTIVE, evtx, evty);
        if (NULL != picked && MX_IS_GRAPH_ELEMENT_PAD(picked)) 
        {
            if (mx_graph_element_pad_is_compatible (
                  priv->pad, MX_GRAPH_ELEMENT_PAD(picked)))
            {
              mx_graph_element_pad_set_link (priv->pad, priv->link, TRUE);
              mx_graph_element_pad_set_link (MX_GRAPH_ELEMENT_PAD(picked), 
                  priv->link, FALSE);
              if (NULL != priv->link)
              {
                clutter_actor_set_reactive (
                    CLUTTER_ACTOR(priv->link), TRUE);
              }
            }
        }
        else
        {
          clutter_container_remove_actor (CLUTTER_CONTAINER (graph), 
              CLUTTER_ACTOR(priv->link));
          goto end_release;
        }
end_release:
        mx_draggable_enable ((MxDraggable *)priv->elt);
        clutter_actor_queue_redraw (CLUTTER_ACTOR(priv->elt));

        priv->link = NULL;
        priv->elt = NULL;
        priv->pad = NULL;
        g_signal_handler_disconnect (clutter_stage_get_default(),
                                     priv->capture_event_signal_id);
        priv->capture_event_signal_id = 0;
      }
      break;
    case CLUTTER_MOTION:
      {
        gfloat x, y;
        clutter_event_get_coords (event, &x, &y);
        mx_graph_element_link_set_dest_point (priv->link, x, y);
      }
      break;
    default:
      break;
  }
  return FALSE;
}

static void
_link_creation (MxGraphElement    *elt,
                MxGraphElementPad *pad,
                MxGraph           *graph)
{
  MxGraphPrivate *priv = graph->priv;
  priv->capture_event_signal_id = g_signal_connect_after (
      clutter_stage_get_default(),
      "captured-event", 
      G_CALLBACK (mx_graph_element_pad_on_stage_capture),
      graph);
  priv->elt = elt;
  priv->pad = pad;

  ClutterActorBox box;
  clutter_actor_get_allocation_box (CLUTTER_ACTOR(pad), &box);
  gfloat xpos, ypos;
  clutter_actor_get_transformed_position (CLUTTER_ACTOR(pad), &xpos, &ypos);
  gfloat myx, myy;
  clutter_actor_get_transformed_position (CLUTTER_ACTOR(graph), &myx, &myy);
  gfloat x = xpos - myx + (box.x2-box.x1)/2.,
         y = ypos - myy + (box.y2-box.y1)/2.;
  priv->link = mx_graph_element_link_new (CLUTTER_CONTAINER (graph), x, y);
  mx_draggable_disable (MX_DRAGGABLE(elt));
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
    g_signal_connect (elt, "link-creation", G_CALLBACK(_link_creation), 
                      MX_GRAPH(container));
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

