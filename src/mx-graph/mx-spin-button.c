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

enum
{
  VALUE_CHANGED,

  LAST_SIGNAL
};

static guint spin_button_signals[LAST_SIGNAL] = { 0, };

struct _MxSpinButtonPrivate
{
  ClutterActor *entry;
  GType         type;
  GValue        curr_value;
  GValue        value_min;
  GValue        value_max;
  ClutterActor *down_stepper;
  ClutterActor *up_stepper;
  guint         stepper_source;
  gint          delta;
  gboolean      is_valid;
};

static void mx_focusable_iface_init (MxFocusableIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxSpinButton, mx_spin_button, MX_TYPE_WIDGET,
    G_IMPLEMENT_INTERFACE (MX_TYPE_FOCUSABLE, mx_focusable_iface_init));

static MxFocusable*
mx_spin_button_accept_focus (MxFocusable *focusable, 
                             MxFocusHint  hint)
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
  clutter_actor_map (priv->down_stepper);
  clutter_actor_map (priv->up_stepper);
}

static void
mx_spin_button_unmap (ClutterActor *self)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(self)->priv;

  CLUTTER_ACTOR_CLASS (mx_spin_button_parent_class)->unmap (self);

  clutter_actor_unmap (priv->entry);
  clutter_actor_unmap (priv->down_stepper);
  clutter_actor_unmap (priv->up_stepper);
}

static void
mx_spin_button_paint (ClutterActor *actor)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_spin_button_parent_class)->paint (actor);

  clutter_actor_paint (priv->entry);
  clutter_actor_paint (priv->down_stepper);
  clutter_actor_paint (priv->up_stepper);
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
  
  gfloat h = (box->y2 - box->y1)/2.;

  childbox.y1 = 0;
  childbox.y2 = (box->y2 - box->y1);
  childbox.x2 = (box->x2 - box->x1)-h;
  clutter_actor_allocate (priv->entry, &childbox, flags);
  
  childbox.x1 = childbox.x2;
  childbox.x2 = childbox.x1 + h;
  childbox.y1 = 0;
  childbox.y2 = h;
  clutter_actor_allocate (priv->up_stepper, &childbox, flags);
  

  childbox.y1 = h;
  childbox.y2 = 2.*h;
  clutter_actor_allocate (priv->down_stepper, &childbox, flags);
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
      g_value_init(&priv->value_min, priv->type);
      g_value_init(&priv->value_max, priv->type);
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

  clutter_actor_paint (priv->down_stepper);
  clutter_actor_paint (priv->up_stepper);
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


  spin_button_signals[VALUE_CHANGED] =
    g_signal_new ("value-changed",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (MxSpinButtonClass, value_changed),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);
  
}

static void
notify_value_changed_valid (MxSpinButton *spin_button)
{
  MxSpinButtonPrivate *priv = spin_button->priv;
  gchar *txt = NULL;

  switch (priv->type) 
  {
    case G_TYPE_ULONG:
      txt = g_strdup_printf("%lu", g_value_get_ulong(&priv->curr_value));
      break;
    case G_TYPE_LONG:
      txt = g_strdup_printf("%ld", g_value_get_long(&priv->curr_value));
      break;
    case G_TYPE_UINT:
      txt = g_strdup_printf("%u", g_value_get_uint(&priv->curr_value));
      break;
    case G_TYPE_INT:
      txt = g_strdup_printf("%d", g_value_get_int(&priv->curr_value));
      break;
    case G_TYPE_UINT64:
      txt = g_strdup_printf("%"G_GUINT64_FORMAT,
          g_value_get_uint64(&priv->curr_value));
      break;
    case G_TYPE_INT64:
      txt = g_strdup_printf("%"G_GINT64_FORMAT,
          g_value_get_int64(&priv->curr_value));
      break;
    case G_TYPE_FLOAT:
      txt = g_strdup_printf("%15.7g",
          g_value_get_float(&priv->curr_value));
      break;
    case G_TYPE_DOUBLE:
      txt = g_strdup_printf("%15.7g",
          g_value_get_double(&priv->curr_value));
      break;
    default:
      g_assert_not_reached();
      break;
  }
  mx_entry_set_text(MX_ENTRY(priv->entry), txt);
  
  g_signal_emit (spin_button, spin_button_signals[VALUE_CHANGED], 0, 
                 &priv->curr_value);
  g_free(txt);
}

static gboolean
_value_change_cb (MxSpinButton *sp)
{
  MxSpinButtonPrivate *priv = sp->priv;
  if(!priv->is_valid)
  {
    return TRUE;
  }

  switch (priv->type) 
  {
    case G_TYPE_ULONG:
      {
        gulong val = g_value_get_ulong(&priv->curr_value);
        gulong maxval = g_value_get_ulong(&priv->value_max);
        gulong minval = g_value_get_ulong(&priv->value_min);
        if(val > 0 && priv->delta < 0)
        {
          val = (gulong)((glong)val + (glong)priv->delta);
        }
        else if(priv->delta > 0)
        {
          val += (gulong)priv->delta;
        }
        if(val >= minval && val <= maxval)
        {
          g_value_set_ulong(&priv->curr_value, (gulong)val);
        }
        break;
      }
    case G_TYPE_LONG:
      {
        glong val = g_value_get_long(&priv->curr_value) +
          (glong)priv->delta;
        glong maxval = g_value_get_long(&priv->value_max);
        glong minval = g_value_get_long(&priv->value_min);
        if(val >= minval && val <= maxval)
        {
          g_value_set_long(&priv->curr_value, val);
        }
        break;
      }
    case G_TYPE_UINT:
      {
        guint val = g_value_get_uint(&priv->curr_value);
        guint maxval = g_value_get_uint(&priv->value_max);
        guint minval = g_value_get_uint(&priv->value_min);
        if(val > 0 && priv->delta < 0)
        {
          val = (guint)((gint)val + (gint)priv->delta);
        }
        else if(priv->delta > 0)
        {
          val += (guint)priv->delta;
        }
        if(val >= minval && val <= maxval)
        {
          g_value_set_uint(&priv->curr_value, (guint)val);
        }
        break;
      }
    case G_TYPE_INT:
      {
        gint val = g_value_get_int(&priv->curr_value) + (gint)priv->delta;
        gint maxval = g_value_get_int(&priv->value_max);
        gint minval = g_value_get_int(&priv->value_min);
        if(val >= minval && val <= maxval)
        {
          g_value_set_int(&priv->curr_value, val);
        }
        break;
      }
    case G_TYPE_UINT64:
      {
        guint64 val = g_value_get_uint64(&priv->curr_value);
        guint64 maxval = g_value_get_uint64(&priv->value_max);
        guint64 minval = g_value_get_uint64(&priv->value_min);
        if(val > 0 && priv->delta < 0)
        {
          val = (guint64)((gint64)val + (gint64)priv->delta);
        }
        else if(priv->delta > 0)
        {
          val += (guint64)priv->delta;
        }
        if(val >= minval && val <= maxval)
        {
          g_value_set_uint64(&priv->curr_value, (guint64)val);
        }
        break;
      }
    case G_TYPE_INT64:
      {
        gint64 val = g_value_get_int64(&priv->curr_value) + 
          (gint64)priv->delta;
        gint64 maxval = g_value_get_int64(&priv->value_max);
        gint64 minval = g_value_get_int64(&priv->value_min);
        if(val >= minval && val <= maxval)
        {
          g_value_set_int64(&priv->curr_value, val);
        }
        break;
      }
    case G_TYPE_FLOAT:
      {
        gfloat val = g_value_get_float(&priv->curr_value) + 
          (gfloat)priv->delta;
        gfloat maxval = g_value_get_float(&priv->value_max);
        gfloat minval = g_value_get_float(&priv->value_min);
        if(val >= minval && val <= maxval)
        {
          g_value_set_float(&priv->curr_value, val);
        }
        break;
      }
    case G_TYPE_DOUBLE:
      {
        gdouble val = g_value_get_double(&priv->curr_value) + 
          (gdouble)priv->delta;
        gdouble maxval = g_value_get_double(&priv->value_max);
        gdouble minval = g_value_get_double(&priv->value_min);
        if(val >= minval && val <= maxval)
        {
          g_value_set_double(&priv->curr_value, val);
        }
        break;
      }
    default:
      g_assert_not_reached();
      break;
  }

  notify_value_changed_valid(sp);
  return TRUE;
}


static gboolean
stepper_button_press_event_cb (ClutterActor       *actor,
                               ClutterButtonEvent *event,
                               MxSpinButton       *sp)
{
  MxSpinButtonPrivate *priv = sp->priv;
  if(CLUTTER_ACTOR(actor) == priv->up_stepper)
  {
    priv->delta = 1;
  }
  else
  {
    priv->delta = -1;
  }
  _value_change_cb(sp);
  priv->stepper_source = g_timeout_add(500, (GSourceFunc) 
    _value_change_cb, sp);
  return FALSE;
}

static gboolean
stepper_button_release_cb (ClutterActor       *actor,
                           ClutterButtonEvent *event,
                           MxSpinButton       *sp)
{
  MxSpinButtonPrivate *priv = sp->priv;
  if(0 != priv->stepper_source)
  {
    g_source_remove(priv->stepper_source);
  }
  priv->stepper_source = 0;
  return FALSE;
}

static void
_text_changed (ClutterText  *c_txt, 
               MxSpinButton *sp)
{
  MxSpinButtonPrivate *priv = sp->priv;
  const gchar *txt = clutter_text_get_text(c_txt);
  gchar *pattern;
  switch (priv->type) 
  {
    case G_TYPE_LONG:
    case G_TYPE_INT:
    case G_TYPE_INT64:
        pattern = "-?[0-9]+";
        break;
    case G_TYPE_ULONG:
    case G_TYPE_UINT:
    case G_TYPE_UINT64:
        pattern = "[0-9]+";
        break;
    case G_TYPE_FLOAT:
    case G_TYPE_DOUBLE:
        pattern = "[0-9]+\\.?[0-9]*";
        break;
    default:
      g_assert_not_reached();
      break;
  }
  gboolean valid = FALSE;
  GRegex *regex = g_regex_new(pattern, 0, 0, NULL);
  GMatchInfo *match_info;
  g_regex_match(regex, txt, 0, &match_info);
  if (g_match_info_matches (match_info))
  {
    gchar *word = g_match_info_fetch (match_info, 0);
    valid = (0 == g_strcmp0(txt, word));
    g_free (word);
  }
  g_match_info_free (match_info);
  g_regex_unref (regex);

  priv->is_valid = valid;
  if(!valid)
  {
    clutter_text_set_color(c_txt, 
        clutter_color_get_static(CLUTTER_COLOR_RED));
  }
  else
  {
    clutter_text_set_color(c_txt, 
        clutter_color_get_static(CLUTTER_COLOR_BLACK));
  }
}

static void
mx_spin_button_init (MxSpinButton *spin_button)
{
  MxSpinButtonPrivate *priv = spin_button->priv = 
    MX_SPIN_BUTTON_GET_PRIVATE(spin_button);
  priv->stepper_source = 0;
  priv->entry = mx_entry_new_with_text("0");

  ClutterActor *c_txt = mx_entry_get_clutter_text (MX_ENTRY(priv->entry));
  g_signal_connect(c_txt, "text-changed", G_CALLBACK(_text_changed),
    spin_button);

  priv->is_valid = TRUE;
  clutter_actor_set_parent(priv->entry, CLUTTER_ACTOR(spin_button));
  
  priv->down_stepper = (ClutterActor *) mx_button_new ();
  mx_stylable_set_style_class (MX_STYLABLE (priv->down_stepper),
                               "down-stepper");
  clutter_actor_set_parent (CLUTTER_ACTOR (priv->down_stepper),
                            CLUTTER_ACTOR (spin_button));
  g_signal_connect (priv->down_stepper, "button-press-event",
      G_CALLBACK (stepper_button_press_event_cb), spin_button);
  g_signal_connect (priv->down_stepper, "button-release-event",
      G_CALLBACK (stepper_button_release_cb), spin_button);
  g_signal_connect (priv->down_stepper, "leave-event",
      G_CALLBACK (stepper_button_release_cb), spin_button);

  priv->up_stepper = (ClutterActor *) mx_button_new ();
  mx_stylable_set_style_class (MX_STYLABLE (priv->up_stepper),
                               "up-stepper");
  clutter_actor_set_parent (CLUTTER_ACTOR (priv->up_stepper),
                            CLUTTER_ACTOR (spin_button));
  g_signal_connect (priv->up_stepper, "button-press-event",
      G_CALLBACK (stepper_button_press_event_cb), spin_button);
  g_signal_connect (priv->up_stepper, "button-release-event",
      G_CALLBACK (stepper_button_release_cb), spin_button);
  g_signal_connect (priv->up_stepper, "leave-event",
      G_CALLBACK (stepper_button_release_cb), spin_button);
}

ClutterActor *
mx_spin_button_new(GType type)
{
  return g_object_new(MX_TYPE_SPIN_BUTTON, "type", type, NULL);
}

void 
mx_spin_button_set_value (MxSpinButton *spin_button, 
                          GValue        value)
{
  MxSpinButtonPrivate *priv = spin_button->priv;
  g_value_copy(&value, &priv->curr_value);
  notify_value_changed_valid(spin_button);
}

void 
mx_spin_button_set_range (MxSpinButton *spin_button, 
                          GValue       *value_min, 
                          GValue       *value_max)
{
  MxSpinButtonPrivate *priv = MX_SPIN_BUTTON(spin_button)->priv;

  //Someday, FIXME
  mx_entry_set_secondary_icon_from_file (MX_ENTRY(priv->entry),
      "/usr/share/icons/gnome/16x16/status/dialog-information.png");

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

  mx_entry_set_secondary_icon_tooltip_text (MX_ENTRY(priv->entry),
                                            tooltip);
  g_free(tooltip);

  g_value_copy(value_min, &priv->value_min);
  g_value_copy(value_max, &priv->value_max);
}

