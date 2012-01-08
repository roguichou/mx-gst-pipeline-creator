#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include "mx-gst-graph-element.h"
#include "mx-gst-graph-element-pad.h"
#include "mx-spin-button.h"

struct _MxGstGraphElementPrivate
{
  GstPlugin         *plugin;
  GstElement        *element;
  GstElementFactory *factory;

  gfloat             xpress,
                     ypress;
  ClutterActor      *dialog;
};

static void _load_gst_element(MxGstGraphElement *actor);

G_DEFINE_TYPE(MxGstGraphElement, mx_gst_graph_element, MX_TYPE_GRAPH_ELEMENT);

static gboolean 
mx_gst_graph_element_button_press (ClutterActor       *actor, 
                                   ClutterButtonEvent *event)
{
  MxGstGraphElementPrivate *priv = MX_GST_GRAPH_ELEMENT(actor)->priv;
  clutter_event_get_coords((ClutterEvent *)event, 
      &priv->xpress, &priv->ypress);
  return FALSE;
}

static gboolean 
mx_gst_graph_element_button_release(ClutterActor       *actor, 
                                   ClutterButtonEvent *event)
{
  MxGstGraphElementPrivate *priv = MX_GST_GRAPH_ELEMENT(actor)->priv;
  gfloat x, y;
  clutter_event_get_coords((ClutterEvent *)event, &x, &y);
  if( fabs(priv->xpress-x) < 5. && fabs(priv->ypress-y) < 5.)
  {
    clutter_actor_show(priv->dialog); 
    gfloat x, y,
           w, h,
           stage_w, stage_h;
    clutter_actor_get_transformed_position(actor, &x, &y);
    clutter_actor_get_size(priv->dialog, &w, &h);
    clutter_actor_get_size(CLUTTER_ACTOR(clutter_stage_get_default()),
      &stage_w, &stage_h);
    if(x+w > stage_w)
    {
      x = stage_w - w;
    }
    if(y+h > stage_h)
    {
      y = stage_h - h;
    }
    clutter_actor_set_position(priv->dialog, x, y);
  }
  return FALSE;
}

static void
mx_gst_graph_element_class_init (MxGstGraphElementClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGstGraphElementPrivate));
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  actor_class->button_press_event   = mx_gst_graph_element_button_press;
  actor_class->button_release_event = mx_gst_graph_element_button_release;
}

static void
mx_gst_graph_element_init (MxGstGraphElement *actor)
{
  actor->priv = G_TYPE_INSTANCE_GET_PRIVATE(
      actor, MX_TYPE_GST_GRAPH_ELEMENT, MxGstGraphElementPrivate);
}

static void 
mx_gst_graph_element_close_dialog(gpointer unused,
                                      ClutterActor *dialog)
{
  clutter_actor_hide (dialog);
  //TODO props

}

MxGstGraphElement *
mx_gst_graph_element_new(GstElementFactory *factory)
{
  //desc
  GstPlugin *plugin = NULL;
  gchar *short_desc = NULL;
  GstElement *element = NULL;
  if (NULL != GST_PLUGIN_FEATURE (factory)->plugin_name) 
  {
    plugin = gst_registry_find_plugin (gst_registry_get_default (),
        GST_PLUGIN_FEATURE (factory)->plugin_name);
    if (NULL != plugin) 
    {
      short_desc = g_strdup_printf("%s\n%s", 
          GST_PLUGIN_FEATURE_NAME(factory), 
          gst_element_factory_get_longname(factory));
    }
  }

  //load element
  factory = GST_ELEMENT_FACTORY (gst_plugin_feature_load (
        GST_PLUGIN_FEATURE (factory)));

  if (NULL == factory) 
  {
    g_warning ("element plugin loading failed\n");
    return NULL;
  }
 
  element = gst_element_factory_create (factory, NULL);
  if (NULL == element) 
  {
    g_warning ("element construction failed\n");
    return NULL;
  }
  gchar *name = NULL;
  g_object_get (element, "name", &name, NULL);

  MxGstGraphElement *res = MX_GST_GRAPH_ELEMENT(
      g_object_new(MX_TYPE_GST_GRAPH_ELEMENT,
        "name", name,
        "blurb", short_desc, NULL));

  MxGstGraphElementPrivate *priv = res->priv;
  priv->plugin  = plugin;
  priv->element = element;
  priv->factory = factory;

  _load_gst_element(res);

  return res;
}

ClutterActor *
_combobox_new_from_enum(GEnumClass *klass, int cur_value)
{
  ClutterActor *res = mx_combo_box_new();
  GHashTable *enum_hash = g_hash_table_new_full(
      g_str_hash, g_str_equal, g_free, NULL);

  g_object_set_data(G_OBJECT(res), "hash-table", enum_hash);

  GEnumValue *values = klass->values;
  const gchar *cur_txt = NULL;

  guint j = 0;
  while (values[j].value_name) 
  {
    if (values[j].value == cur_value)
    {
      cur_txt = values[j].value_name;
    }
    g_hash_table_insert(enum_hash, g_strdup(values[j].value_name),
        GINT_TO_POINTER(values[j].value));
    
    mx_combo_box_append_text(MX_COMBO_BOX(res), values[j].value_name);
    j++;
  }
  mx_combo_box_set_active_text (MX_COMBO_BOX(res), cur_txt);

  return res;
} 

static void
_load_gst_element(MxGstGraphElement *actor)
{
  MxGstGraphElementPrivate *priv = actor->priv;
  const GList *pads_templates = 
    gst_element_factory_get_static_pad_templates(priv->factory);
  GList *pads_iter = (GList *)pads_templates;
  while(pads_iter)
  {
    MxGstGraphElementPad *eltPad = mx_gst_graph_element_pad_new(
        (GstStaticPadTemplate *)(pads_iter->data), priv->element);

    mx_graph_element_add_pad(MX_GRAPH_ELEMENT(actor), 
        MX_GRAPH_ELEMENT_PAD(eltPad), 
        mx_gst_graph_element_pad_get_position(eltPad));

    pads_iter = g_list_next(pads_iter);
  }

  priv->dialog = mx_dialog_new();
  MxAction *action_quit = mx_action_new_full("close", "Close",
      G_CALLBACK(mx_gst_graph_element_close_dialog), priv->dialog);
  mx_dialog_add_action(MX_DIALOG(priv->dialog), action_quit);

  ClutterActor *scroll_view = mx_scroll_view_new();
  ClutterActor *kinetic = mx_kinetic_scroll_view_new ();
  mx_bin_set_child (MX_BIN (priv->dialog), scroll_view);
  clutter_actor_set_size (scroll_view, 400, 350);

  ClutterActor *view = mx_viewport_new ();
  mx_viewport_set_sync_adjustments (MX_VIEWPORT (view), FALSE);
  clutter_container_add_actor (CLUTTER_CONTAINER (kinetic), view);
  clutter_container_add_actor (CLUTTER_CONTAINER (scroll_view), kinetic);

  clutter_container_add_actor(
      CLUTTER_CONTAINER(clutter_stage_get_default()),
      priv->dialog); 

  ClutterActor *table = mx_table_new();
  clutter_container_add_actor(CLUTTER_CONTAINER(view), table); 

  guint num_properties, i;
  GParamSpec **property_specs = g_object_class_list_properties
      (G_OBJECT_GET_CLASS (priv->element), &num_properties);

  for (i = 0; i < num_properties; i++) 
  {
    GValue value     = {0, }, 
           value_min = {0, }, 
           value_max = {0, };
    GParamSpec *param = property_specs[i];
    g_value_init (&value, param->value_type);
    g_value_init (&value_min, param->value_type);
    g_value_init (&value_max, param->value_type);

    ClutterActor *titre = 
      mx_label_new_with_text(g_param_spec_get_name(param));
    mx_table_add_actor(MX_TABLE(table), titre, i+1, 1);
    mx_widget_set_tooltip_text(MX_WIDGET(titre),
        g_param_spec_get_blurb (param));

    g_object_get_property (G_OBJECT (priv->element), param->name, &value);

    ClutterActor *prop_editor = NULL;

    switch (G_VALUE_TYPE (&value)) 
    {
      case G_TYPE_STRING:
        prop_editor = mx_entry_new_with_text(
            g_strdup(g_value_get_string (&value)));
        break;
      case G_TYPE_BOOLEAN:
        prop_editor = mx_toggle_new();
        mx_toggle_set_active(MX_TOGGLE(prop_editor), 
            g_value_get_boolean (&value));
        break;
      case G_TYPE_ULONG:
      {
        GParamSpecULong *pulong = G_PARAM_SPEC_ULONG (param);
        prop_editor = mx_spin_button_new(G_TYPE_ULONG);
        g_value_set_ulong(&value_min, pulong->minimum);
        g_value_set_ulong(&value_max, pulong->maximum);
        break;
      }
      case G_TYPE_LONG:
      {
        GParamSpecLong *plong = G_PARAM_SPEC_LONG (param);
        prop_editor = mx_spin_button_new(G_TYPE_LONG);
        g_value_set_long(&value_min, plong->minimum);
        g_value_set_long(&value_max, plong->maximum);
        break;
      }
      case G_TYPE_UINT:
      {
        GParamSpecUInt *puint = G_PARAM_SPEC_UINT (param);
        prop_editor = mx_spin_button_new(G_TYPE_UINT);
        g_value_set_uint(&value_min, puint->minimum);
        g_value_set_uint(&value_max, puint->maximum);
        break;
      }
      case G_TYPE_INT:
      {
        GParamSpecInt *pint = G_PARAM_SPEC_INT (param);
        prop_editor = mx_spin_button_new(G_TYPE_INT);
        g_value_set_int(&value_min, pint->minimum);
        g_value_set_int(&value_max, pint->maximum);
        break;
      }
      case G_TYPE_UINT64:
      {
        GParamSpecUInt64 *puint64 = G_PARAM_SPEC_UINT64 (param);
        prop_editor = mx_spin_button_new(G_TYPE_UINT64);
        g_value_set_uint64(&value_min, puint64->minimum);
        g_value_set_uint64(&value_max, puint64->maximum);
        break;
      }
      case G_TYPE_INT64:
      {
        GParamSpecInt64 *pint64 = G_PARAM_SPEC_INT64 (param);
        prop_editor = mx_spin_button_new(G_TYPE_INT64);
        g_value_set_int64(&value_min, pint64->minimum);
        g_value_set_int64(&value_max, pint64->maximum);
        break;
      }
      case G_TYPE_FLOAT:
      {
        GParamSpecFloat *pfloat = G_PARAM_SPEC_FLOAT (param);
        prop_editor = mx_spin_button_new(G_TYPE_FLOAT);
        g_value_set_float(&value_min, pfloat->minimum);
        g_value_set_float(&value_max, pfloat->maximum);
        break;
      }
      case G_TYPE_DOUBLE:
      {
        GParamSpecDouble *pdouble = G_PARAM_SPEC_DOUBLE (param);
        prop_editor = mx_spin_button_new(G_TYPE_DOUBLE);
        g_value_set_double(&value_min, pdouble->minimum);
        g_value_set_double(&value_max, pdouble->maximum);
        break;
      }
      default:
        if (G_IS_PARAM_SPEC_ENUM (param)) 
        {
          prop_editor = _combobox_new_from_enum(
              G_ENUM_CLASS(g_type_class_ref (param->value_type)),
              g_value_get_enum (&value));
        }
        else if (GST_IS_PARAM_SPEC_FRACTION (param)) 
        {
          prop_editor = mx_label_new();
          GstParamSpecFraction *pfraction = GST_PARAM_SPEC_FRACTION (param);
          gchar *tooltip = g_strdup_printf("Range : %d/%d - %d/%d",
              pfraction->min_num, pfraction->min_den,
              pfraction->max_num, pfraction->max_den);
          gchar *txt_val = g_strdup_printf("%d/%d",
              gst_value_get_fraction_numerator (&value),
              gst_value_get_fraction_denominator (&value));
          mx_label_set_text(MX_LABEL(prop_editor), txt_val);
          g_free(txt_val);
          mx_widget_set_tooltip_text(MX_WIDGET(prop_editor), tooltip);
          g_free(tooltip);
        }
        else
        {
          prop_editor = mx_label_new();
          mx_label_set_text(MX_LABEL(prop_editor), 
              "Unsupported type ... Yet...");
        }
        break;
   }
   if(MX_IS_SPIN_BUTTON(prop_editor))
   {
     mx_spin_button_set_value(MX_SPIN_BUTTON(prop_editor), value);
     mx_spin_button_set_range(MX_SPIN_BUTTON(prop_editor), 
         &value_min, &value_max);
   }

   g_value_reset (&value);
   g_value_reset (&value_min);
   g_value_reset (&value_max);
   mx_table_add_actor(MX_TABLE(table), prop_editor, i+1, 2);
  }
  g_free (property_specs);

  MxAdjustment *vadjust, *hadjust;
  mx_scrollable_get_adjustments (MX_SCROLLABLE (view), &hadjust, &vadjust);
  mx_adjustment_set_values (vadjust,
      0, 0, clutter_actor_get_height(table)-60, 60, 120, 200);
  mx_adjustment_set_values (hadjust,
      0, 0, clutter_actor_get_width(table), 60, 120, 200);
}

