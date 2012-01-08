#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-spin-button.h"

#define MX_SPIN_BUTTON_GET_PRIVATE(obj)              \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj),               \
                                MX_TYPE_SPIN_BUTTON, \
                                MxSpinButtonPrivate))
enum
{
  PROP_0,

  PROP_TYPE
};

struct _MxSpinButtonPrivate
{
  ClutterActor *entry;
  GType         type;
  GValue        curr_value;
  ClutterActor *icon;
};

static void mx_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxSpinButton, mx_spin_button, MX_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE, mx_focusable_iface_init));

static MxFocusable*
mx_spin_button_accept_focus (MxFocusable *focusable, MxFocusHint hint)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(focusable)->priv;
  mx_focusable_accept_focus(MX_FOCUSABLE (priv->entry), hint);

  return focusable;
}

static void
mx_focusable_iface_init (MxFocusableIface *iface)
{
  iface->accept_focus = mx_spin_button_accept_focus;
}

static void
mx_spin_button_map (ClutterActor *self)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(self)->priv;

  CLUTTER_ACTOR_CLASS (mx_spin_button_parent_class)->map (self);

  clutter_actor_map (priv->entry);
  clutter_actor_map (priv->icon);
}

static void
mx_spin_button_unmap (ClutterActor *self)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(self)->priv;

  CLUTTER_ACTOR_CLASS (mx_spin_button_parent_class)->unmap (self);

  clutter_actor_unmap (priv->entry);
  clutter_actor_unmap (priv->icon);
}

static void
mx_spin_button_paint(ClutterActor *actor)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_spin_button_parent_class)->paint (actor);

  clutter_actor_paint (priv->entry);
  clutter_actor_paint (priv->icon);
}

static void
mx_spin_button_allocate (ClutterActor           *actor,
                               const ClutterActorBox  *box,
                               ClutterAllocationFlags  flags)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_spin_button_parent_class)->allocate (
      actor, box, flags);

  ClutterActorBox childbox;
  childbox.x1 = 0;
  childbox.y1 = 0;

  if(NULL != priv->icon)
  {
    gint sz = mx_icon_get_icon_size(MX_ICON(priv->icon));
    childbox.x2 = sz;
    childbox.y1 = ((box->y2 - box->y1)-sz)/2;
    childbox.y2 = childbox.y1 + sz;
    clutter_actor_allocate (priv->icon, &childbox, flags);
    childbox.x1 = sz+2;
  }

  childbox.y1 = 0;
  childbox.y2 = (box->y2 - box->y1);
  childbox.x2 = (box->x2 - box->x1);
  clutter_actor_allocate (priv->entry, &childbox, flags);
}

static void
mx_spin_button_set_property (GObject      *gobject,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(gobject)->priv;
  switch (prop_id)
  {
    case PROP_TYPE:
      priv->type = g_value_get_gtype(value);
      g_value_init(&priv->curr_value, priv->type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_spin_button_get_property (GObject    *gobject,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(gobject)->priv;

  switch (prop_id)
  {
    case PROP_TYPE:
      g_value_set_gtype(value, priv->type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
  }
}

static void
mx_spin_button_get_preferred_width (ClutterActor *actor,
                              gfloat        for_height,
                              gfloat       *min_width_p,
                              gfloat       *natural_width_p)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(actor)->priv;

  clutter_actor_get_preferred_width (priv->entry, for_height,
                                     min_width_p,
                                     natural_width_p);

}

static void
mx_spin_button_get_preferred_height (ClutterActor *actor,
                               gfloat        for_width,
                               gfloat       *min_height_p,
                               gfloat       *natural_height_p)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(actor)->priv;

  clutter_actor_get_preferred_height (priv->entry, for_width,
                                      min_height_p,
                                      natural_height_p);
}

static void
mx_spin_button_pick (ClutterActor       *actor,
               const ClutterColor *c)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_spin_button_parent_class)->pick (actor, c);

  clutter_actor_paint (priv->entry);

  if (NULL != priv->icon)
  {
    clutter_actor_paint (priv->icon);
  }
}

static void
mx_spin_button_class_init (MxSpinButtonClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxSpinButtonPrivate));
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  actor_class->map                  = mx_spin_button_map;
  actor_class->unmap                = mx_spin_button_unmap;
  actor_class->paint                = mx_spin_button_paint;
  actor_class->allocate             = mx_spin_button_allocate;
  actor_class->get_preferred_width  = mx_spin_button_get_preferred_width;
  actor_class->get_preferred_height = mx_spin_button_get_preferred_height;
  actor_class->pick                 = mx_spin_button_pick;

  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->set_property = mx_spin_button_set_property;
  gobject_class->get_property = mx_spin_button_get_property;

  GParamSpec *pspec = NULL;
  pspec = g_param_spec_gtype ("type",
                              "Type",
                              "GType",
                              G_TYPE_NONE,
                              G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property (gobject_class, PROP_TYPE, pspec);

  //TODO
}

static void
mx_spin_button_init (MxSpinButton *spin_button)
{
  MxSpinButtonPrivate *priv = spin_button->priv = 
    MX_SPIN_BUTTON_GET_PRIVATE(spin_button);
  priv->icon = NULL;
  priv->entry = mx_entry_new();
  clutter_actor_set_parent(priv->entry, CLUTTER_ACTOR(spin_button));
  //TODO
}

ClutterActor *mx_spin_button_new(GType type)
{
  return g_object_new(MX_TYPE_SPIN_BUTTON, "type", type, NULL);
}

void mx_spin_button_set_value(MxSpinButton *spin_button, GValue value)
{
  MxSpinButtonPrivate *priv = spin_button->priv;
  g_value_copy(&value, &priv->curr_value);
}

void mx_spin_button_set_range(MxSpinButton *spin_button, 
    GValue *value_min, GValue *value_max)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(spin_button)->priv;
  priv->icon = mx_icon_new();
  mx_icon_set_icon_name(MX_ICON(priv->icon), "gtk-dialog-info");
  mx_icon_set_icon_size(MX_ICON(priv->icon), 16);
  clutter_actor_set_parent(priv->icon, CLUTTER_ACTOR(spin_button));

  gchar *tooltip = NULL;
  switch (priv->type) 
  {
    case G_TYPE_ULONG:
      tooltip = g_strdup_printf("Range : %lu - %lu",
          g_value_get_ulong(value_min), g_value_get_ulong(value_max));
      break;
    case G_TYPE_LONG:
      tooltip = g_strdup_printf("Range : %ld - %ld",
          g_value_get_long(value_min), g_value_get_long(value_max));
      break;
    case G_TYPE_UINT:
      tooltip = g_strdup_printf("Range : %u - %u",
          g_value_get_uint(value_min), g_value_get_uint(value_max));
      break;
    case G_TYPE_INT:
      tooltip = g_strdup_printf("Range : %d - %d",
          g_value_get_int(value_min), g_value_get_int(value_max));
      break;
    case G_TYPE_UINT64:
      tooltip = g_strdup_printf("Range : %"G_GUINT64_FORMAT
          " - %"G_GUINT64_FORMAT, g_value_get_uint64(value_min), 
          g_value_get_uint64(value_max));
      break;
    case G_TYPE_INT64:
      tooltip = g_strdup_printf("Range : %"G_GINT64_FORMAT
          " - %"G_GINT64_FORMAT, g_value_get_int64(value_min), 
          g_value_get_int64(value_max));
      break;
    case G_TYPE_FLOAT:
      tooltip = g_strdup_printf("Range : %15.7g - %15.7g",
          g_value_get_float(value_min), g_value_get_float(value_max));
      break;
    case G_TYPE_DOUBLE:
      tooltip = g_strdup_printf("Range : %15.7g - %15.7g",
          g_value_get_double(value_min), g_value_get_double(value_max));
      break;
    default:
      tooltip = g_strdup_printf("Range : %p - %p",
          g_value_get_pointer(value_min), 
          g_value_get_pointer(value_max));
      g_assert_not_reached();
      break;
  }

  mx_widget_set_tooltip_text(MX_WIDGET(priv->icon), tooltip);
  g_free(tooltip);
}

