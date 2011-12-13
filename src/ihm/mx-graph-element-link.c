#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-graph-element-pad.h"
#include "mx-graph-element-link.h"
#include <math.h>

/**
 * Private structure.
 */
struct _MxGraphElementLinkPrivate
{
  /** coords {(x,y)*}. */
  gfloat *coords;
  /** nb points in coords (=length_of(coords)/2)*/
  gint nb_pts;
};

G_DEFINE_TYPE (MxGraphElementLink, mx_graph_element_link, MX_TYPE_WIDGET)


static void
mx_graph_element_link_paint(ClutterActor *actor)
{
  MxGraphElementLinkPrivate *priv = MX_GRAPH_ELEMENT_LINK(actor)->priv;

  cogl_push_matrix ();

  cogl_path_new();
  cogl_set_source_color4ub (0, 0, 0, 255);
  cogl_path_polyline(priv->coords, priv->nb_pts);
  cogl_path_stroke();

  cogl_pop_matrix ();
}

static void
mx_graph_element_link_dispose(GObject *object)
{
  MxGraphElementLinkPrivate *priv = MX_GRAPH_ELEMENT_LINK(object)->priv;
  if(NULL != priv->coords)
  {
    g_free(priv->coords);
  }
  G_OBJECT_CLASS (mx_graph_element_link_parent_class)->dispose (object);
}

static void
mx_graph_element_link_class_init (MxGraphElementLinkClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGraphElementLinkPrivate));
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass *g_class= G_OBJECT_CLASS(klass);
  actor_class->paint = mx_graph_element_link_paint;
  g_class->dispose   = mx_graph_element_link_dispose;
}

static void
mx_graph_element_link_init (MxGraphElementLink *actor)
{
  actor->priv = G_TYPE_INSTANCE_GET_PRIVATE(actor, MX_TYPE_GRAPH_ELEMENT_LINK,
    MxGraphElementLinkPrivate);
  //TODO
}

/**
 * mx_graph_element_link_new:
 *
 * Creates a new link with an origin
 *
 * @xorig -in-: x coord of first point(relative to the link parent).
 * @yorig -in-: y coord of first point(relative to the link parent).
 *
 * @return a new #MxGraphElementLink
 */
MxGraphElementLink *mx_graph_element_link_new(gfloat xorig, gfloat yorig)
{
  MxGraphElementLink *res = MX_GRAPH_ELEMENT_LINK(
      g_object_new(MX_TYPE_GRAPH_ELEMENT_LINK, NULL));

  MxGraphElementLinkPrivate *priv = res->priv;
  
  //Initialization : a degenerated line with the 2 same points.
  priv->coords = g_malloc(4*sizeof(gfloat));
  priv->coords[0] = priv->coords[2] = xorig;
  priv->coords[1] = priv->coords[3] = yorig;
  priv->nb_pts = 2;
  //so that the actor is drawn
  clutter_actor_set_size(CLUTTER_ACTOR(res),1,1);

  return res;
}

/**
 * mx_graph_element_link_set_dest_point:
 *
 * @actor -in- a #MxGraphElementLink
 *
 * @xdest -in- new x coord of the link's last point 
 *          (relative to the link parent).
 *
 * @ydest -in- new y coord of the link's last point 
 *          (relative to the link parent).
 */
void
mx_graph_element_link_set_dest_point (MxGraphElementLink *actor, 
    gfloat xdest, gfloat ydest)
{
  MxGraphElementLinkPrivate *priv = actor->priv;
  gint idx_last_pt = 2*(priv->nb_pts - 1);
  priv->coords[idx_last_pt  ] = xdest;
  priv->coords[idx_last_pt+1] = ydest;
 
  clutter_actor_queue_relayout(
      clutter_actor_get_parent(CLUTTER_ACTOR(actor)));
}

/**
 * mx_graph_element_link_set_orig_point:
 *
 * @actor -in- a #MxGraphElementLink
 *
 * @xdest -in- new x coord of the link's first point 
 *          (relative to the link parent).
 *
 * @ydest -in- new y coord of the link's first point 
 *          (relative to the link parent).
 */
void
mx_graph_element_link_set_orig_point (MxGraphElementLink *actor, 
    gfloat xori, gfloat yori)
{
  MxGraphElementLinkPrivate *priv = actor->priv;
  priv->coords[0] = xori;
  priv->coords[1] = yori;
 
  clutter_actor_queue_relayout(
      clutter_actor_get_parent(CLUTTER_ACTOR(actor)));
}

