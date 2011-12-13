#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-graph-element.h"

struct _MxGraphElementPrivate
{
  gchar             *name;
  gchar             *short_desc;
  ClutterActor      *rect;

  gint pad_size; //TODO : define as property

  GList             *northPads;
  GList             *southPads;
  GList             *eastPads;
  GList             *westPads;
};

G_DEFINE_TYPE (MxGraphElement, mx_graph_element, MX_TYPE_WIDGET)

static void
mx_graph_element_paint(ClutterActor *actor)
{
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_graph_element_parent_class)->paint (actor);

  clutter_actor_paint (priv->rect);

  g_list_foreach(priv->northPads, (GFunc)clutter_actor_paint, NULL);
  g_list_foreach(priv->southPads, (GFunc)clutter_actor_paint, NULL);
  g_list_foreach(priv->eastPads, (GFunc)clutter_actor_paint, NULL);
  g_list_foreach(priv->westPads, (GFunc)clutter_actor_paint, NULL);

}

static void
mx_graph_element_pick (ClutterActor       *actor,
                       const ClutterColor *color)
{
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(actor)->priv;
  CLUTTER_ACTOR_CLASS (mx_graph_element_parent_class)->pick (actor, color);
  
  g_list_foreach(priv->northPads, (GFunc)clutter_actor_paint, NULL);
  g_list_foreach(priv->southPads, (GFunc)clutter_actor_paint, NULL);
  g_list_foreach(priv->eastPads, (GFunc)clutter_actor_paint, NULL);
  g_list_foreach(priv->westPads, (GFunc)clutter_actor_paint, NULL);
}

static void
mx_graph_element_map (ClutterActor *self)
{
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(self)->priv;

  CLUTTER_ACTOR_CLASS (mx_graph_element_parent_class)->map (self);

  clutter_actor_map (priv->rect);
  g_list_foreach(priv->northPads, (GFunc)clutter_actor_map, NULL);
  g_list_foreach(priv->southPads, (GFunc)clutter_actor_map, NULL);
  g_list_foreach(priv->eastPads, (GFunc)clutter_actor_map, NULL);
  g_list_foreach(priv->westPads, (GFunc)clutter_actor_map, NULL);
}

static void
mx_graph_element_unmap (ClutterActor *self)
{
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(self)->priv;

  CLUTTER_ACTOR_CLASS (mx_graph_element_parent_class)->unmap (self);

  clutter_actor_unmap (priv->rect);
  g_list_foreach(priv->northPads, (GFunc)clutter_actor_unmap, NULL);
  g_list_foreach(priv->southPads, (GFunc)clutter_actor_unmap, NULL);
  g_list_foreach(priv->eastPads, (GFunc)clutter_actor_unmap, NULL);
  g_list_foreach(priv->westPads, (GFunc)clutter_actor_unmap, NULL);
}

static void
mx_graph_element_allocate (ClutterActor           *actor,
                    const ClutterActorBox  *box,
                    ClutterAllocationFlags  flags)
{
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_graph_element_parent_class)->allocate (
      actor, box, flags);

  ClutterActorBox globalChildbox;
  ClutterActorBox childbox;
  globalChildbox.x1 = 0;
  globalChildbox.y1 = 0;
  globalChildbox.x2 = (box->x2 - box->x1);
  globalChildbox.y2 = (box->y2 - box->y1);

  clutter_actor_allocate (priv->rect, &globalChildbox, flags);

  gfloat available_h = globalChildbox.y2;
  
  GList *iter = priv->westPads;
  childbox.x1 = globalChildbox.x1 - priv->pad_size/2.;
  childbox.x2 = childbox.x1 + priv->pad_size;
  gfloat stepy = available_h/(g_list_length(iter)+1);
  childbox.y1 = stepy - priv->pad_size/2.;
  childbox.y2 = childbox.y1 + priv->pad_size;
  while (NULL != iter)
  {
    clutter_actor_allocate (CLUTTER_ACTOR(iter->data), &childbox, flags);

    iter = g_list_next(iter);
    childbox.y1 += stepy;
    childbox.y2 += stepy;
  }

  iter = priv->eastPads;
  childbox.x1 = globalChildbox.x2 - priv->pad_size/2.;
  childbox.x2 = childbox.x1 + priv->pad_size;
  stepy = available_h/(g_list_length(iter)+1);
  childbox.y1 = stepy - priv->pad_size/2.;
  childbox.y2 = childbox.y1 + priv->pad_size;
  while (NULL != iter)
  {
    clutter_actor_allocate (CLUTTER_ACTOR(iter->data), &childbox, flags);

    iter = g_list_next(iter);
    childbox.y1 += stepy;
    childbox.y2 += stepy;
  }





  gfloat available_w = globalChildbox.x2;

  iter = priv->southPads;
  childbox.y1 = globalChildbox.y2 - priv->pad_size/2.;
  childbox.y2 = childbox.y1 + priv->pad_size;
  gfloat stepx = available_w/(g_list_length(iter)+1);
  childbox.x1 = stepx - priv->pad_size/2.;
  childbox.x2 = childbox.x1 + priv->pad_size;
  while (NULL != iter)
  {
    clutter_actor_allocate (CLUTTER_ACTOR(iter->data), &childbox, flags);

    iter = g_list_next(iter);
    childbox.x1 += stepx;
    childbox.x2 += stepx;
  }

  iter = priv->northPads;
  childbox.y1 = globalChildbox.y1 - priv->pad_size/2.;
  childbox.y2 = childbox.y1 + priv->pad_size;
  stepx = available_w/(g_list_length(iter)+1);
  childbox.x1 = stepx - priv->pad_size/2.;
  childbox.x2 = childbox.x1 + priv->pad_size;
  while (NULL != iter)
  {
    clutter_actor_allocate (CLUTTER_ACTOR(iter->data), &childbox, flags);

    iter = g_list_next(iter);
    childbox.x1 += stepx;
    childbox.x2 += stepx;
  }
}

static void
mx_graph_element_class_init (MxGraphElementClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGraphElementPrivate));
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  actor_class->paint                = mx_graph_element_paint;
  actor_class->pick                 = mx_graph_element_pick;
  actor_class->map                  = mx_graph_element_map;
  actor_class->unmap                = mx_graph_element_unmap;
  actor_class->allocate             = mx_graph_element_allocate;
}

static void
mx_graph_element_init (MxGraphElement *actor)
{
  MxGraphElementPrivate *priv = actor->priv = G_TYPE_INSTANCE_GET_PRIVATE(
      actor, MX_TYPE_GRAPH_ELEMENT, MxGraphElementPrivate);
  priv->name               = NULL;
  priv->short_desc         = NULL;
  priv->rect               = clutter_rectangle_new();
  clutter_actor_set_parent(priv->rect, CLUTTER_ACTOR(actor));
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE(priv->rect), 2);

  priv->pad_size  = 10;
  priv->northPads = NULL;
  priv->southPads = NULL;
  priv->eastPads  = NULL;
  priv->westPads  = NULL;
}

MxGraphElement *mx_graph_element_new(gchar *name, gchar *short_desc)
{
  MxGraphElement *res = MX_GRAPH_ELEMENT(
      g_object_new(MX_TYPE_GRAPH_ELEMENT, NULL));
  MxGraphElementPrivate *priv = res->priv;
  priv->name               = name;
  priv->short_desc         = short_desc;

  gchar *txt = g_strdup_printf("%s \n %s", name, short_desc);
  mx_widget_set_tooltip_text(MX_WIDGET(res), txt);
  g_free(txt);

  return res;
}

void
mx_graph_element_add_pad(MxGraphElement *elt, MxGraphElementPad *pad, 
    MxGraphElementPadPosition position)
{
  MxGraphElementPrivate *priv = elt->priv;
  switch (position)
  {
    case PAD_POSITION_WEST:
      priv->westPads = g_list_append(priv->westPads, pad);
      break;
    case PAD_POSITION_EAST:
      priv->eastPads = g_list_append(priv->eastPads, pad);
      break;
    case PAD_POSITION_SOUTH:
      priv->southPads = g_list_append(priv->southPads, pad);
      break;
    case PAD_POSITION_NORTH:
      priv->northPads = g_list_append(priv->northPads, pad);
      break;
  }
  clutter_actor_set_parent(CLUTTER_ACTOR(pad), CLUTTER_ACTOR(elt));
  clutter_actor_set_size(CLUTTER_ACTOR(pad), priv->pad_size, priv->pad_size);
  clutter_actor_queue_relayout(CLUTTER_ACTOR(elt));
}
