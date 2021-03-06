#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-graph-element.h"
#include "mx-selectable.h"

enum
{
  PROP_0,
  //Props for draggable overriding
  PROP_DRAG_THRESHOLD,
  PROP_AXIS,
  PROP_ENABLED,
  PROP_ACTOR,
  //EigenProps
  PROP_NAME,
  PROP_BLURB,
  PROP_SELECTED
};

enum
{
  LINK_CREATION,

  LAST_SIGNAL
};
static guint elt_signals[LAST_SIGNAL] = { 0, };

struct _MxGraphElementPrivate
{
  gchar        *name;
  gchar        *short_desc;
  ClutterActor *rect;
  guint         rect_width;
  ClutterActor *name_actor;

  gint          pad_size;

  GList        *northPads;
  GList        *southPads;
  GList        *eastPads;
  GList        *westPads;

  guint         threshold;
  MxDragAxis    axis;
  gboolean      is_enabled;

  gboolean      is_selected;
  gdouble       motion;
  gulong        capture_event_signal_id;
  ClutterActor *graph;
};

static void mx_graph_element_draggable_iface_init (MxDraggableIface *iface);
static void mx_selectable_iface_init (MxSelectableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (MxGraphElement, mx_graph_element, MX_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE (MX_TYPE_DRAGGABLE, 
      mx_graph_element_draggable_iface_init)
    G_IMPLEMENT_INTERFACE (MX_TYPE_SELECTABLE,
      mx_selectable_iface_init));

static void
mx_graph_element_set_selected (MxSelectable *selectable,
                                   gboolean      selected)
{
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(selectable)->priv;
  priv->is_selected = selected;
}

static void 
mx_selectable_iface_init (MxSelectableInterface *iface)
{
  iface->set_selected = mx_graph_element_set_selected;
}

static void
mx_graph_element_drag_begin (MxDraggable        *draggable,
                             gfloat               event_x,
                             gfloat               event_y,
                             gint                 event_button,
                             ClutterModifierType  modifiers)
{
  ClutterActor *actor = CLUTTER_ACTOR(draggable);
  ClutterContainer *parent = 
    CLUTTER_CONTAINER(clutter_actor_get_parent (actor));
  clutter_container_raise_child (parent, actor, NULL);
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(draggable)->priv;
  priv->motion = 0;

  // .TODO. raise links above
}

static void
mx_graph_element_drag_motion (MxDraggable *draggable,
                              gfloat         delta_x,
                              gfloat         delta_y)
{
  gfloat posx,posy;
  clutter_actor_get_position(CLUTTER_ACTOR(draggable), &posx, &posy);
  posx += delta_x;
  posy += delta_y;
  if(posx < 0) posx = 0;
  if(posy < 0) posy = 0;
  clutter_actor_set_position(CLUTTER_ACTOR(draggable), posx, posy);
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(draggable)->priv;
  priv->motion += delta_x*delta_x + delta_y*delta_y;
}

static gboolean
mx_graph_element_wait_for_desselection (ClutterActor   *stage,
                                        ClutterEvent   *event,
                                        MxGraphElement *actor)
{
  MxGraphElementPrivate *priv = actor->priv;
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

static void
mx_graph_element_drag_end (MxDraggable *draggable,
                           gfloat         event_x,
                           gfloat         event_y)
{
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(draggable)->priv;
  if (priv->motion < 25)
  {
    mx_selectable_set_selected (MX_SELECTABLE(draggable), TRUE);
    priv->capture_event_signal_id = g_signal_connect_after (
        clutter_stage_get_default(),
        "captured-event", 
        G_CALLBACK (mx_graph_element_wait_for_desselection),
        draggable);
    clutter_actor_queue_redraw (CLUTTER_ACTOR(draggable));
  }
}

static void
mx_graph_element_draggable_iface_init (MxDraggableIface *iface)
{
  iface->drag_begin = mx_graph_element_drag_begin;
  iface->drag_motion = mx_graph_element_drag_motion;
  iface->drag_end = mx_graph_element_drag_end;
}

static void
mx_graph_element_paint(ClutterActor *actor)
{
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_graph_element_parent_class)->paint (actor);

  clutter_actor_paint (priv->rect);
  clutter_actor_paint (priv->name_actor);

  g_list_foreach(priv->northPads, (GFunc)clutter_actor_paint, NULL);
  g_list_foreach(priv->southPads, (GFunc)clutter_actor_paint, NULL);
  g_list_foreach(priv->eastPads, (GFunc)clutter_actor_paint, NULL);
  g_list_foreach(priv->westPads, (GFunc)clutter_actor_paint, NULL);

  mx_selectable_draw_around_actor (MX_SELECTABLE (actor), 
                                   CLUTTER_RECTANGLE (priv->rect));
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
  clutter_actor_map (priv->name_actor);
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
  clutter_actor_unmap (priv->name_actor);
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
  globalChildbox.x1 = priv->pad_size/2.;
  globalChildbox.y1 = priv->pad_size/2.;
  globalChildbox.x2 = (box->x2 - box->x1)-priv->pad_size/2.;
  globalChildbox.y2 = (box->y2 - box->y1)-priv->pad_size/2.;

  clutter_actor_allocate (priv->rect, &globalChildbox, flags);
  
  childbox.x1 = childbox.y1 = priv->pad_size+1;
  clutter_actor_get_preferred_width(
      priv->name_actor, -1, NULL, &childbox.x2);
  clutter_actor_get_preferred_height(
      priv->name_actor, -1, NULL, &childbox.y2);
  childbox.x2 += childbox.x1;
  childbox.y2 += childbox.y1;
  clutter_actor_allocate (priv->name_actor, &childbox, flags);

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
mx_graph_element_set_property (GObject      *gobject,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(gobject)->priv;

  switch (prop_id)
    {
    case PROP_DRAG_THRESHOLD:
      priv->threshold = g_value_get_uint (value);
      break;

    case PROP_AXIS:
      priv->axis = g_value_get_enum (value);
      break;

    case PROP_ENABLED:
      priv->is_enabled = g_value_get_boolean (value);
      if (priv->is_enabled)
        mx_draggable_enable (MX_DRAGGABLE (gobject));
      else
        mx_draggable_disable (MX_DRAGGABLE (gobject));
      break;

    case PROP_ACTOR:
      break;

    case PROP_NAME:
      priv->name = g_strdup(g_value_get_string(value));
      clutter_text_set_text(CLUTTER_TEXT(priv->name_actor), priv->name);
      break;

    case PROP_BLURB:
      priv->short_desc = g_strdup(g_value_get_string(value));
      mx_widget_set_tooltip_text(MX_WIDGET(gobject), priv->short_desc);
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
mx_graph_element_get_property (GObject    *gobject,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  MxGraphElementPrivate *priv = MX_GRAPH_ELEMENT(gobject)->priv;

  switch (prop_id)
    {
    case PROP_DRAG_THRESHOLD:
      g_value_set_uint (value, priv->threshold);
      break;

    case PROP_AXIS:
      g_value_set_enum (value, priv->axis);
      break;

    case PROP_ENABLED:
      g_value_set_boolean (value, priv->is_enabled);
      break;

    case PROP_ACTOR:
        g_value_set_object (value, gobject);
      break;

    case PROP_NAME:
        g_value_set_string(value, priv->name);
        break;
    case PROP_BLURB:
        g_value_set_string(value, priv->short_desc);
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
mx_graph_element_class_init (MxGraphElementClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGraphElementPrivate));
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  actor_class->paint             = mx_graph_element_paint;
  actor_class->pick              = mx_graph_element_pick;
  actor_class->map               = mx_graph_element_map;
  actor_class->unmap             = mx_graph_element_unmap;
  actor_class->allocate          = mx_graph_element_allocate;

  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->set_property = mx_graph_element_set_property;
  gobject_class->get_property = mx_graph_element_get_property;

  g_object_class_override_property (gobject_class,
                                    PROP_DRAG_THRESHOLD,
                                    "drag-threshold");
  g_object_class_override_property (gobject_class,
                                    PROP_AXIS,
                                    "axis");
  g_object_class_override_property (gobject_class,
                                    PROP_ENABLED,
                                    "drag-enabled");
  g_object_class_override_property (gobject_class,
                                    PROP_ACTOR,
                                    "drag-actor");

  g_object_class_override_property (gobject_class,
                                    PROP_SELECTED,
                                    "selected");
  GParamSpec *pspec = NULL;
  pspec = g_param_spec_string ("name",
                               "Name",
                               "Name of the Element",
                               NULL,
                               G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property (gobject_class, PROP_NAME, pspec);

  pspec = g_param_spec_string ("blurb",
                               "Blurb",
                               "Short description of the Element",
                               NULL,
                               G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property (gobject_class, PROP_BLURB, pspec);

  elt_signals[LINK_CREATION] =
    g_signal_new ("link-creation",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (MxGraphElementClass, link_creation),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1, MX_TYPE_GRAPH_ELEMENT_PAD);
}

static void
mx_graph_element_init (MxGraphElement *actor)
{
  MxGraphElementPrivate *priv = actor->priv = G_TYPE_INSTANCE_GET_PRIVATE(
      actor, MX_TYPE_GRAPH_ELEMENT, MxGraphElementPrivate);
  priv->name               = NULL;
  priv->short_desc         = NULL;
  priv->rect               = clutter_rectangle_new();
  priv->rect_width         = 2;
  clutter_actor_set_parent(priv->rect, CLUTTER_ACTOR(actor));
  clutter_rectangle_set_border_width (CLUTTER_RECTANGLE(priv->rect), 
                                      priv->rect_width);

  priv->name_actor = clutter_text_new();
  clutter_actor_set_parent(priv->name_actor, CLUTTER_ACTOR(actor));

  priv->pad_size  = 10;
  priv->northPads = NULL;
  priv->southPads = NULL;
  priv->eastPads  = NULL;
  priv->westPads  = NULL;

  priv->is_selected = FALSE;
}

MxGraphElement *
mx_graph_element_new(gchar *name, 
                     gchar *short_desc)
{
  return MX_GRAPH_ELEMENT(g_object_new (MX_TYPE_GRAPH_ELEMENT, 
                                        "name",  name,
                                        "blurb", short_desc,
                                        NULL));
}

static void
_pad_link_creation (MxGraphElementPad *pad,
                    MxGraphElement    *elt)
{
    g_signal_emit (G_OBJECT(elt), elt_signals[LINK_CREATION], 0, pad);
}

void
mx_graph_element_add_pad(MxGraphElement            *elt, 
                         MxGraphElementPad         *pad, 
                         MxGraphElementPadPosition  position)
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
  g_signal_connect (G_OBJECT(pad), "link-creation", 
                    G_CALLBACK(_pad_link_creation), 
                    elt);
  mx_graph_element_pad_set_graph (pad, priv->graph);
}

void
mx_graph_element_set_graph (MxGraphElement *elt, 
                            ClutterActor   *graph)
{
  MxGraphElementPrivate *priv = elt->priv;
  priv->graph = graph;

  if (NULL != priv->westPads)
  {
    g_list_foreach (priv->westPads, 
        (GFunc)mx_graph_element_pad_set_graph,
        priv->graph);
  }
  if (NULL != priv->eastPads)
  {
    g_list_foreach (priv->eastPads, 
        (GFunc)mx_graph_element_pad_set_graph,
        priv->graph);
  }
  if (NULL != priv->southPads)
  {
    g_list_foreach (priv->southPads, 
        (GFunc)mx_graph_element_pad_set_graph,
        priv->graph);
  }
  if (NULL != priv->northPads)
  {
    g_list_foreach (priv->northPads, 
        (GFunc)mx_graph_element_pad_set_graph,
        priv->graph);
  }
}

