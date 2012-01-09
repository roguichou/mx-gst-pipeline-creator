#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-graph-element.h"
#include "mx-graph-element-pad.h"
#include "mx-graph-element-link.h"

enum
{
  PROP_0,

  PROP_NAME,
  PROP_BLURB,
  PROP_COMPATIBLE_FUNC
};

struct _MxGraphElementPadPrivate
{
  gchar               *name;
  gchar               *short_desc;
  PadIsCompatibleFunc *is_compatible_func;
  ClutterActor        *rect;
  gulong               capture_event_signal_id;
  MxGraphElementLink  *link;
  gboolean             link_is_src;
  gulong               parent_allocation_changed_id;
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
    return pad_orig->priv->is_compatible_func(pad_orig, pad_dest);
  }
}

static void
mx_graph_element_pad_on_link_destroy(MxGraphElementLink *link,
                                     MxGraphElementPad  *pad)
{
  MxGraphElementPadPrivate *priv = pad->priv;
  priv->link = NULL;

}

static void
mx_graph_element_pad_set_link(MxGraphElementPad  *pad, 
                              MxGraphElementLink *link)
{
  MxGraphElementPadPrivate *priv = pad->priv;
  ClutterActorBox box;
  clutter_actor_get_allocation_box (CLUTTER_ACTOR(pad), &box);
  if(NULL == priv->link)
  {
    priv->link = link;
    priv->link_is_src = FALSE;
    gfloat xpos, ypos;
    clutter_actor_get_transformed_position(CLUTTER_ACTOR(pad),
        &xpos, &ypos);
    mx_graph_element_link_set_dest_point (priv->link, 
        xpos + (box.x2-box.x1)/2., ypos + (box.y2-box.y1)/2.);

    g_signal_connect(priv->link, "destroy", 
        G_CALLBACK(mx_graph_element_pad_on_link_destroy), pad);
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
            CLUTTER_PICK_REACTIVE, x, y);
        if((NULL != picked && MX_IS_GRAPH_ELEMENT_PAD(picked)) &&
            mx_graph_element_pad_is_compatible(MX_GRAPH_ELEMENT_PAD(actor),
              MX_GRAPH_ELEMENT_PAD(picked)))
        {
          mx_graph_element_pad_set_link(MX_GRAPH_ELEMENT_PAD(picked), 
              priv->link);
          if(NULL != priv->link)
          {
            clutter_actor_set_reactive(CLUTTER_ACTOR(priv->link), TRUE);
          }
        }
        else
        {      
          ClutterContainer *cont = CLUTTER_CONTAINER(
              clutter_actor_get_parent(CLUTTER_ACTOR(priv->link)));
          clutter_container_remove_actor(cont, CLUTTER_ACTOR(priv->link));
        }
        ClutterActor *parent = clutter_actor_get_parent(CLUTTER_ACTOR(actor));
        if(MX_IS_DRAGGABLE(parent))
        {
          mx_draggable_enable((MxDraggable *)parent);
        }
      }
      break;
    case CLUTTER_MOTION:
      {
        gfloat x, y;
        clutter_event_get_coords(event, &x, &y);
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
        "captured-event", 
        G_CALLBACK (mx_graph_element_pad_on_stage_capture),
        actor);

    ClutterActorBox box;
    clutter_actor_get_allocation_box (actor, &box);
    // the parent should be a GraphElement.
    // We want to add the link to the Container containing the GraphElement.
    gfloat xpos, ypos;
    clutter_actor_get_transformed_position(actor, &xpos, &ypos);
    ClutterActor *cont = clutter_actor_get_parent(actor);
    while(NULL != cont && !CLUTTER_IS_CONTAINER(cont))
    {
      cont = clutter_actor_get_parent(cont);
    }
    if(NULL != cont)
    {
      priv->link_is_src = TRUE;
      priv->link = mx_graph_element_link_new(CLUTTER_CONTAINER(cont),
          xpos + (box.x2-box.x1)/2., ypos + (box.y2-box.y1)/2.);

      g_signal_connect(priv->link, "destroy", 
          G_CALLBACK(mx_graph_element_pad_on_link_destroy), pad);

      ClutterActor *parent = clutter_actor_get_parent(actor);
      if(MX_IS_DRAGGABLE(parent))
      {
        mx_draggable_disable((MxDraggable *)parent);
      }
    }
  }
  return FALSE;
}


static void
mx_graph_element_pad_on_parent_allocation_changed(
    ClutterActor           *parent,
    const ClutterActorBox  *pbox,
    ClutterAllocationFlags  flags,
    MxGraphElementPad      *pad)
{
  MxGraphElementPadPrivate *priv = pad->priv;
  if(NULL == priv->link)
  {
    return;
  }
  gfloat posx, posy;
  clutter_actor_get_transformed_position(CLUTTER_ACTOR(pad), &posx, &posy);
  
  ClutterActorBox box;
  clutter_actor_get_allocation_box (CLUTTER_ACTOR(pad), &box);
  if(!priv->link_is_src)
  {
    mx_graph_element_link_set_dest_point (priv->link, 
          posx + (box.x2-box.x1)/2., posy + (box.y2-box.y1)/2.);
  }
  else
  {
    mx_graph_element_link_set_orig_point (priv->link, 
          posx + (box.x2-box.x1)/2., posy + (box.y2-box.y1)/2.);
  }
}

static void
mx_graph_element_pad_parent_set(ClutterActor *actor,
                                ClutterActor *old_parent)
{
  MxGraphElementPad *pad = MX_GRAPH_ELEMENT_PAD(actor);
  MxGraphElementPadPrivate *priv = pad->priv;

  ClutterActor *parent = clutter_actor_get_parent(actor);
  priv->parent_allocation_changed_id = 
    g_signal_connect(parent, "allocation-changed", 
        G_CALLBACK(mx_graph_element_pad_on_parent_allocation_changed), actor);
}


static void
mx_graph_element_pad_set_property (GObject      *gobject,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  MxGraphElementPadPrivate *priv = MX_GRAPH_ELEMENT_PAD(gobject)->priv;
  gchar *txt = NULL;
  switch (prop_id)
  {
    case PROP_NAME:
      priv->name = g_strdup(g_value_get_string(value));
      txt = g_strdup_printf("%s \n %s", priv->name, priv->short_desc);
      mx_widget_set_tooltip_text(MX_WIDGET(gobject), txt);
      g_free(txt);
      break;

    case PROP_BLURB:
      priv->short_desc = g_strdup(g_value_get_string(value));
      txt = g_strdup_printf("%s \n %s", priv->name, priv->short_desc);
      mx_widget_set_tooltip_text(MX_WIDGET(gobject), txt);
      g_free(txt);
      break;

    case PROP_COMPATIBLE_FUNC:
      priv->is_compatible_func = g_value_get_pointer(value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_graph_element_pad_get_property (GObject    *gobject,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  MxGraphElementPadPrivate *priv = MX_GRAPH_ELEMENT_PAD(gobject)->priv;

  switch (prop_id)
  {
    case PROP_NAME:
      g_value_set_string(value, priv->name);
      break;
    case PROP_BLURB:
      g_value_set_string(value, priv->short_desc);
      break;
    case PROP_COMPATIBLE_FUNC:
      g_value_set_pointer(value, priv->is_compatible_func);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
  }
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
  actor_class->parent_set           = mx_graph_element_pad_parent_set;

  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->set_property = mx_graph_element_pad_set_property;
  gobject_class->get_property = mx_graph_element_pad_get_property;

  GParamSpec *pspec = NULL;
  pspec = g_param_spec_string ("name",
                               "Name",
                               "Name of the Pad",
                               NULL,
                               G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property (gobject_class, PROP_NAME, pspec);

  pspec = g_param_spec_string ("blurb",
                               "Blurb",
                               "Short description of the Pad",
                               NULL,
                               G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property (gobject_class, PROP_BLURB, pspec);


  pspec = g_param_spec_pointer("is-compatible-func",
                               "IsCompatibleFunc",
                               "Function to tell whether a pad"
                               "is compatible with this one or not",
                               G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject_class, PROP_COMPATIBLE_FUNC, pspec);

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
  priv->parent_allocation_changed_id = 0;
  priv->capture_event_signal_id = 0;
}

MxGraphElementPad *
mx_graph_element_pad_new(gchar               *name, 
                         gchar               *short_desc,
                         PadIsCompatibleFunc *is_compatible_func)
{
  MxGraphElementPad *res = MX_GRAPH_ELEMENT_PAD(
      g_object_new(MX_TYPE_GRAPH_ELEMENT_PAD, 
        "name", name,
        "blurb", short_desc,
        "is-compatible-func", is_compatible_func,
        NULL));
  return res;
}

