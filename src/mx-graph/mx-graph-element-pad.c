#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-graph-element.h"
#include "mx-graph-element-pad.h"
#include "mx-selectable.h"

enum
{
  PROP_0,

  PROP_NAME,
  PROP_BLURB,
  PROP_COMPATIBLE_FUNC,

  PROP_SELECTED
};

enum
{
  LINK_CREATION,

  LAST_SIGNAL
};
static guint pad_signals[LAST_SIGNAL] = { 0, };

struct _MxGraphElementPadPrivate
{
  gchar               *name;
  gchar               *short_desc;
  PadIsCompatibleFunc *is_compatible_func;
  ClutterActor        *rect;
  guint                rect_width;
  gulong               capture_event_signal_id;
  MxGraphElementLink  *link;
  gboolean             link_is_src;
  gulong               parent_allocation_changed_id;
  gboolean             is_selected;
  gboolean             pressed;
  ClutterActor        *graph;
};

static void mx_selectable_iface_init (MxSelectableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (MxGraphElementPad, 
                         mx_graph_element_pad, 
                         MX_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (MX_TYPE_SELECTABLE,
                                                mx_selectable_iface_init));

static void
mx_graph_element_pad_set_selected (MxSelectable *selectable,
                                   gboolean      selected)
{
  MxGraphElementPadPrivate *priv = MX_GRAPH_ELEMENT_PAD(selectable)->priv;
  priv->is_selected = selected;
}

static void 
mx_selectable_iface_init (MxSelectableInterface *iface)
{
  iface->set_selected = mx_graph_element_pad_set_selected;
}

static void
mx_graph_element_pad_paint (ClutterActor *actor)
{
  MxGraphElementPadPrivate *priv = MX_GRAPH_ELEMENT_PAD(actor)->priv;
  CLUTTER_ACTOR_CLASS (mx_graph_element_pad_parent_class)->paint (actor);
  clutter_actor_paint (priv->rect);
  mx_selectable_draw_around_actor (MX_SELECTABLE (actor), 
                                   CLUTTER_RECTANGLE (priv->rect));
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
  childbox.x1 = 1;
  childbox.y1 = 1;
  childbox.x2 = (box->x2 - box->x1)-1;
  childbox.y2 = (box->y2 - box->y1)-1;

  clutter_actor_allocate (priv->rect, &childbox, flags);
}

gboolean
mx_graph_element_pad_is_compatible(MxGraphElementPad *pad_orig,
                                   MxGraphElementPad *pad_dest)
{
  if (NULL == pad_orig->priv->is_compatible_func || 
      NULL == pad_dest->priv->is_compatible_func)
  {
    return TRUE;
  }
  else
  {
    return pad_orig->priv->is_compatible_func (pad_orig, pad_dest);
  }
}

static void
mx_graph_element_pad_on_link_destroy(MxGraphElementLink *link,
                                     MxGraphElementPad  *pad)
{
  MxGraphElementPadPrivate *priv = pad->priv;
  priv->link = NULL;
}

gboolean
mx_graph_element_pad_set_link(MxGraphElementPad  *pad, 
                              MxGraphElementLink *link,
                              gboolean            is_src)
{
  MxGraphElementPadPrivate *priv = pad->priv;
  ClutterActorBox box;
  clutter_actor_get_allocation_box (CLUTTER_ACTOR(pad), &box);
  if (NULL == priv->link)
  {
    priv->link = link;
    priv->link_is_src = is_src;
    gfloat xpos, ypos;
    clutter_actor_get_transformed_position (CLUTTER_ACTOR(pad),
        &xpos, &ypos);
    xpos += (box.x2-box.x1)/2.;
    ypos += (box.y2-box.y1)/2.;
    if (is_src)
    {
      mx_graph_element_link_set_orig_point (priv->link, xpos, ypos);
    }
    else
    {
      mx_graph_element_link_set_dest_point (priv->link, xpos, ypos);
    }
    g_signal_connect(priv->link, "destroy", 
        G_CALLBACK(mx_graph_element_pad_on_link_destroy), pad);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

static gboolean 
mx_graph_element_pad_leave (ClutterActor         *actor,
                            ClutterCrossingEvent *event)
{
  MxGraphElementPad *pad = MX_GRAPH_ELEMENT_PAD(actor);
  MxGraphElementPadPrivate *priv = pad->priv;
  priv->pressed = FALSE;
  CLUTTER_ACTOR_CLASS (mx_graph_element_pad_parent_class)->
    leave_event (actor, event);
  return FALSE;
}

static gboolean
mx_graph_element_pad_wait_for_desselection (ClutterActor      *stage,
                                       ClutterEvent      *event,
                                       MxGraphElementPad *actor)
{
  MxGraphElementPadPrivate *priv = actor->priv;
  switch (event->type)
  {
    case CLUTTER_BUTTON_PRESS:
      {
        gfloat x,y, w, h;
        clutter_actor_get_transformed_position (priv->graph,
                                                &x, &y);
        clutter_actor_get_size(priv->graph, &w, &h);
        gfloat xevt, yevt;
        clutter_event_get_coords(event, &xevt, &yevt);
        xevt -= x;
        yevt -= y;
        if (xevt>0 && xevt<w && yevt>0 && yevt<h)
        {
          mx_selectable_set_selected (MX_SELECTABLE(actor), FALSE);
          if (0 != priv->capture_event_signal_id)
          {
            g_signal_handler_disconnect (stage,
                priv->capture_event_signal_id);
            priv->capture_event_signal_id = 0;
          }
          clutter_actor_queue_redraw (CLUTTER_ACTOR(actor));
        }
        break;
      }
    default:
      break;
  }
  return FALSE;
}


static gboolean 
mx_graph_element_pad_button_release (ClutterActor       *actor, 
                                   ClutterButtonEvent *event)
{
  MxGraphElementPad *pad = MX_GRAPH_ELEMENT_PAD(actor);
  MxGraphElementPadPrivate *priv = pad->priv;
  if (priv->pressed)
  {
    mx_selectable_set_selected (MX_SELECTABLE(actor), TRUE);
    priv->capture_event_signal_id = g_signal_connect_after (
        clutter_stage_get_default(),
        "captured-event", 
        G_CALLBACK (mx_graph_element_pad_wait_for_desselection),
        actor);
    clutter_actor_queue_redraw (CLUTTER_ACTOR(actor));
  }
  priv->pressed = FALSE;
  return FALSE;
}

static gboolean 
mx_graph_element_pad_button_press (ClutterActor       *actor, 
                                   ClutterButtonEvent *event)
{
  MxGraphElementPad *pad = MX_GRAPH_ELEMENT_PAD(actor);
  MxGraphElementPadPrivate *priv = pad->priv;
  priv->pressed = TRUE;

  if (NULL == priv->link)
  {
    g_signal_emit (actor, pad_signals[LINK_CREATION], 0);
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
  clutter_actor_get_transformed_position (CLUTTER_ACTOR(pad), &posx, &posy);
  
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
    g_signal_connect (parent, "allocation-changed", 
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
      priv->name = g_strdup (g_value_get_string (value));
      txt = g_strdup_printf ("%s \n %s", priv->name, priv->short_desc);
      mx_widget_set_tooltip_text (MX_WIDGET(gobject), txt);
      g_free(txt);
      break;

    case PROP_BLURB:
      priv->short_desc = g_strdup (g_value_get_string (value));
      txt = g_strdup_printf ("%s \n %s", priv->name, priv->short_desc);
      mx_widget_set_tooltip_text (MX_WIDGET(gobject), txt);
      g_free(txt);
      break;

    case PROP_COMPATIBLE_FUNC:
      priv->is_compatible_func = g_value_get_pointer (value);
      break;

    case PROP_SELECTED:
      mx_selectable_set_selected (MX_SELECTABLE(gobject),
                                  g_value_get_boolean(value));
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
      g_value_set_string (value, priv->name);
      break;
    case PROP_BLURB:
      g_value_set_string (value, priv->short_desc);
      break;
    case PROP_COMPATIBLE_FUNC:
      g_value_set_pointer (value, priv->is_compatible_func);
      break;
    case PROP_SELECTED:
      g_value_set_boolean (value, priv->is_selected);
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
  actor_class->button_release_event = mx_graph_element_pad_button_release;
  actor_class->leave_event          = mx_graph_element_pad_leave;
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
  g_object_class_install_property (gobject_class,
                                   PROP_COMPATIBLE_FUNC,
                                   pspec);

  g_object_class_override_property (gobject_class,
                                    PROP_SELECTED,
                                    "selected");

  pad_signals[LINK_CREATION] =
    g_signal_new ("link-creation",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (MxGraphElementPadClass, link_creation),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 0);
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
  priv->rect_width         = 1;
  clutter_actor_set_parent (priv->rect, CLUTTER_ACTOR(actor));
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE(priv->rect), 
      priv->rect_width);
  priv->parent_allocation_changed_id = 0;
  priv->capture_event_signal_id = 0;

  priv->is_selected = FALSE;
}

MxGraphElementPad *
mx_graph_element_pad_new(gchar               *name, 
                         gchar               *short_desc,
                         PadIsCompatibleFunc *is_compatible_func)
{
  MxGraphElementPad *res = MX_GRAPH_ELEMENT_PAD(
      g_object_new (MX_TYPE_GRAPH_ELEMENT_PAD, 
                    "name", name,
                    "blurb", short_desc,
                    "is-compatible-func", is_compatible_func,
                    NULL));
  return res;
}

void
mx_graph_element_pad_set_graph (MxGraphElementPad *pad, 
                                ClutterActor      *graph)
{
  MxGraphElementPadPrivate *priv = pad->priv;
  priv->graph = graph;
}

