#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-graph-element-pad.h"
#include "mx-graph-element-link.h"

struct _MxGraphElementPadPrivate
{
  gchar             *name;
  gchar             *short_desc;
  MxGraphElementPadIsCompatible *is_compatible_func;
  ClutterActor      *rect;

  gulong capture_event_signal_id;
  MxGraphElementLink *link;
};

G_DEFINE_TYPE (MxGraphElementPad, mx_graph_element_pad, MX_TYPE_WIDGET)

static void
mx_graph_element_pad_paint(ClutterActor *actor)
{
  MxGraphElementPadPrivate *priv = MX_GRAPH_ELEMENT_PAD(actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_graph_element_pad_parent_class)->paint (actor);

  clutter_actor_paint (priv->rect);
}

static void
mx_graph_element_pad_map (ClutterActor *self)
{
  MxGraphElementPadPrivate *priv = MX_GRAPH_ELEMENT_PAD(self)->priv;

  CLUTTER_ACTOR_CLASS (mx_graph_element_pad_parent_class)->map (self);

  clutter_actor_map (priv->rect);
}

static void
mx_graph_element_pad_unmap (ClutterActor *self)
{
  MxGraphElementPadPrivate *priv = MX_GRAPH_ELEMENT_PAD(self)->priv;

  CLUTTER_ACTOR_CLASS (mx_graph_element_pad_parent_class)->unmap (self);

  clutter_actor_unmap (priv->rect);
}

static void
mx_graph_element_pad_allocate (ClutterActor           *actor,
                    const ClutterActorBox  *box,
                    ClutterAllocationFlags  flags)
{
  MxGraphElementPadPrivate *priv = MX_GRAPH_ELEMENT_PAD(actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_graph_element_pad_parent_class)->allocate (
      actor, box, flags);

  ClutterActorBox childbox;
  childbox.x1 = 0;
  childbox.y1 = 0;
  childbox.x2 = (box->x2 - box->x1);
  childbox.y2 = (box->y2 - box->y1);

  clutter_actor_allocate (priv->rect, &childbox, flags);
}

static gboolean
mx_graph_element_pad_is_compatible(MxGraphElementPad *pad_orig,
    MxGraphElementPad *pad_dest)
{
  if(NULL == pad_orig->priv->is_compatible_func || 
      NULL == pad_dest->priv->is_compatible_func)
  {
    return TRUE;
  }
  else
  {
    return pad_orig->priv->is_compatible_func(pad_dest);
  }
}

static void
mx_graph_element_pad_on_link_destroy(MxGraphElementLink *link,
    MxGraphElementPad *pad)
{
  MxGraphElementPadPrivate *priv = pad->priv;
  priv->link = NULL;

}

static void
mx_graph_element_pad_set_link(MxGraphElementPad *pad, MxGraphElementLink *link)
{
  MxGraphElementPadPrivate *priv = pad->priv;
  ClutterActorBox box;
  clutter_actor_get_allocation_box (CLUTTER_ACTOR(pad), &box);
  if(NULL == priv->link)
  {
    priv->link = link;
    // the parent should be a GraphElement.
    // We want to add the link to the Container containing the GraphElement.
    gfloat xoffset = 0;
    gfloat yoffset = 0;
    ClutterActor * cont = clutter_actor_get_parent(CLUTTER_ACTOR(pad));
    while(NULL != cont && !CLUTTER_IS_CONTAINER(cont))
    {
      ClutterActorBox boxParent;
      clutter_actor_get_allocation_box (cont, &boxParent);

      xoffset += boxParent.x1;
      yoffset += boxParent.y1;
      cont = clutter_actor_get_parent(cont);
    }
    if(NULL != cont)
    {
      mx_graph_element_link_set_dest_point (priv->link, 
          xoffset + box.x1 + (box.x2-box.x1)/2., 
          yoffset + box.y1 + (box.y2-box.y1)/2.);

      g_signal_connect(priv->link, "destroy", 
          G_CALLBACK(mx_graph_element_pad_on_link_destroy), pad);
    }
  }
  else
  {
    ClutterContainer *cont = CLUTTER_CONTAINER(
        clutter_actor_get_parent(CLUTTER_ACTOR(link)));
    clutter_container_remove_actor(cont, CLUTTER_ACTOR(link));
  }
}

static gboolean
mx_graph_element_pad_on_stage_capture (ClutterActor      *stage,
                                       ClutterEvent      *event,
                                       MxGraphElementPad *actor)
{
  MxGraphElementPadPrivate *priv = actor->priv;
  switch (event->type)
  {
    case CLUTTER_BUTTON_RELEASE:
      {
        if(0 != priv->capture_event_signal_id)
        {
          g_signal_handler_disconnect(stage, priv->capture_event_signal_id);
          priv->capture_event_signal_id = 0;
        }
        gfloat x, y;
        clutter_event_get_coords(event, &x, &y);
        ClutterActor *picked = clutter_stage_get_actor_at_pos(
            CLUTTER_STAGE(clutter_stage_get_default()),
            CLUTTER_PICK_ALL, x, y);
        if(NULL != picked && MX_IS_GRAPH_ELEMENT_PAD(picked))
        {
          if(mx_graph_element_pad_is_compatible(MX_GRAPH_ELEMENT_PAD(actor),
                MX_GRAPH_ELEMENT_PAD(picked)))
          {
            mx_graph_element_pad_set_link(MX_GRAPH_ELEMENT_PAD(picked), 
                priv->link);
          }
        }
        else
        {      
          ClutterContainer *cont = CLUTTER_CONTAINER(
              clutter_actor_get_parent(CLUTTER_ACTOR(priv->link)));
          clutter_container_remove_actor(cont, CLUTTER_ACTOR(priv->link));
        }
      }
      break;
    case CLUTTER_MOTION:
      {
        ClutterActorBox box;
        clutter_actor_get_allocation_box (CLUTTER_ACTOR(actor), &box);

        gfloat x, y, xparent, yparent;

        clutter_event_get_coords(event, &x, &y);

        clutter_actor_get_transformed_position(clutter_actor_get_parent(
              CLUTTER_ACTOR(priv->link)), &xparent, &yparent);

        x -= xparent;
        y -= yparent;
        mx_graph_element_link_set_dest_point (priv->link, x, y);
      }
      break;
    default:
      break;
  }
  return FALSE;
}

static gboolean 
mx_graph_element_pad_button_press (ClutterActor       *actor, 
                                   ClutterButtonEvent *event)
{
  MxGraphElementPad *pad = MX_GRAPH_ELEMENT_PAD(actor);
  MxGraphElementPadPrivate *priv = pad->priv;

  if(NULL == priv->link)
  {
    priv->capture_event_signal_id = g_signal_connect_after (
        clutter_stage_get_default(),
        "captured-event", G_CALLBACK (mx_graph_element_pad_on_stage_capture),
        actor);

    ClutterActorBox box;
    clutter_actor_get_allocation_box (actor, &box);

    // the parent should be a GraphElement.
    // We want to add the link to the Container containing the GraphElement.
    gfloat xoffset = 0;
    gfloat yoffset = 0;
    ClutterActor * cont = clutter_actor_get_parent(actor);
    while(NULL != cont && !CLUTTER_IS_CONTAINER(cont))
    {
      ClutterActorBox boxParent;
      clutter_actor_get_allocation_box (cont, &boxParent);

      xoffset += boxParent.x1;
      yoffset += boxParent.y1;
      cont = clutter_actor_get_parent(cont);
    }
    if(NULL != cont)
    {
      priv->link = mx_graph_element_link_new(
          xoffset + box.x1 + (box.x2-box.x1)/2., 
          yoffset + box.y1 + (box.y2-box.y1)/2.);

      clutter_container_add_actor( CLUTTER_CONTAINER(cont),
          CLUTTER_ACTOR(priv->link));

      g_signal_connect(priv->link, "destroy", 
          G_CALLBACK(mx_graph_element_pad_on_link_destroy), pad);
    }
  }
  return FALSE;
}

static void
mx_graph_element_pad_class_init (MxGraphElementPadClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGraphElementPadPrivate));
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  actor_class->paint                = mx_graph_element_pad_paint;
  actor_class->map                  = mx_graph_element_pad_map;
  actor_class->unmap                = mx_graph_element_pad_unmap;
  actor_class->allocate             = mx_graph_element_pad_allocate;
  actor_class->button_press_event   = mx_graph_element_pad_button_press;
}

static void
mx_graph_element_pad_init (MxGraphElementPad *actor)
{
  MxGraphElementPadPrivate *priv = actor->priv = G_TYPE_INSTANCE_GET_PRIVATE(
    actor, MX_TYPE_GRAPH_ELEMENT_PAD, MxGraphElementPadPrivate);
  priv->name               = NULL;
  priv->short_desc         = NULL;
  priv->is_compatible_func = NULL;
  priv->rect               = clutter_rectangle_new();
  clutter_actor_set_parent(priv->rect, CLUTTER_ACTOR(actor));
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE(priv->rect), 1);
}

MxGraphElementPad *mx_graph_element_pad_new(gchar *name, gchar *short_desc,
  MxGraphElementPadIsCompatible *is_compatible_func)
{
  MxGraphElementPad *res = MX_GRAPH_ELEMENT_PAD(
      g_object_new(MX_TYPE_GRAPH_ELEMENT_PAD, NULL));
  MxGraphElementPadPrivate *priv = res->priv;
  priv->name               = name;
  priv->short_desc         = short_desc;
  priv->is_compatible_func = is_compatible_func;

  gchar *txt = g_strdup_printf("%s \n %s", name, short_desc);
  mx_widget_set_tooltip_text(MX_WIDGET(res), txt);
  g_free(txt);

  return res;
}

