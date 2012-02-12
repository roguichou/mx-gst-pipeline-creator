#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-spin-button.h"
#include "mx-gobject-properties-editor.h"

#define MX_GOBJECT_PROPERTIES_GET_PRIVATE(obj)                     \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj),                             \
                                MX_TYPE_GOBJECT_PROPERTIES_EDITOR, \
                                MxGObjectPropertiesEditorPrivate))

enum
{
  PROP_0,
  PROP_OBJECT
};

struct _MxGObjectPropertiesEditorPrivate
{
  GObject  *object;
  guint     curr_line;
};

G_DEFINE_TYPE (MxGObjectPropertiesEditor,
               mx_gobject_properties_editor, MX_TYPE_TABLE);

static void 
_mx_gobj_props_editor_load_object(MxGObjectPropertiesEditor *editor);

static void
mx_gobj_props_editor_set_property (GObject      *gobject,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  MxGObjectPropertiesEditor *editor = MX_GOBJECT_PROPERTIES_EDITOR(gobject);
  MxGObjectPropertiesEditorPrivate *priv = editor->priv;

  switch (prop_id)
  {
    case PROP_OBJECT:
      priv->object = g_value_get_object(value);
      _mx_gobj_props_editor_load_object(editor);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
    }
}

static void
mx_gobj_props_editor_get_property (GObject    *gobject,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  MxGObjectPropertiesEditorPrivate *priv = 
    MX_GOBJECT_PROPERTIES_EDITOR(gobject)->priv;

  switch (prop_id)
  {
    case PROP_OBJECT:
      g_value_set_object(value, priv->object);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
  }
}

static void
mx_gobject_properties_editor_class_init (MxGObjectPropertiesEditorClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGObjectPropertiesEditorPrivate));
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->set_property = mx_gobj_props_editor_set_property;
  gobject_class->get_property = mx_gobj_props_editor_get_property;

  GParamSpec *pspec = NULL;
  pspec = g_param_spec_object ("object",
                               "Object",
                               "Object",
                               G_TYPE_OBJECT,
                               G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property (gobject_class, PROP_OBJECT, pspec);
}

static void
mx_gobject_properties_editor_init (MxGObjectPropertiesEditor *editor)
{
  MxGObjectPropertiesEditorPrivate *priv = editor->priv =
    MX_GOBJECT_PROPERTIES_GET_PRIVATE(editor);
  
  priv->curr_line = 0;
}

static void 
_prop_bool_changed (MxToggle          *toggle, 
                    GParamSpec        *pspec,
                    GObject           *object)
{
  GParamSpec *prop = g_object_get_data (G_OBJECT(toggle), "prop");
  g_object_set (object, prop->name, mx_toggle_get_active(toggle), NULL);
}

static ClutterActor *
_bool_prop_editor_new (MxGObjectPropertiesEditor *editor,
                       GParamSpec                *spec,
                       gboolean                   current)
{
  MxGObjectPropertiesEditorPrivate *priv = editor->priv;
  ClutterActor *prop_editor = mx_toggle_new();
  mx_toggle_set_active (MX_TOGGLE(prop_editor), current);
  g_object_set_data (G_OBJECT(prop_editor), "prop", spec);
  g_signal_connect (prop_editor, "notify::active", 
                    G_CALLBACK(_prop_bool_changed), priv->object);
  return prop_editor;
}

static void 
_prop_string_text_changed (ClutterText *c_txt, 
                           GObject     *object)
{
  GParamSpec *prop = g_object_get_data(G_OBJECT(c_txt), "prop");
  gchar *txt = g_strdup(clutter_text_get_text(c_txt));
  GValue val = {0,};
  g_value_init(&val, G_TYPE_STRING);
  g_value_set_string(&val, txt);
  gboolean changed = g_param_value_validate(prop, &val);
  g_object_set_property(object, prop->name, &val);
  if(changed)
  {
    clutter_text_set_text(c_txt, txt);
  }
  g_free(txt);
}

static ClutterActor *
_string_prop_editor_new (MxGObjectPropertiesEditor *editor,
                         GParamSpec                *spec,
                         const gchar               *current)
{
  MxGObjectPropertiesEditorPrivate *priv = editor->priv;
  ClutterActor *prop_editor = mx_entry_new_with_text (g_strdup (current));
  ClutterActor *c_txt = mx_entry_get_clutter_text (MX_ENTRY(prop_editor));
  g_object_set_data(G_OBJECT(c_txt), "prop", spec);
  g_signal_connect (c_txt, "text-changed", 
                    G_CALLBACK(_prop_string_text_changed), priv->object);
  return prop_editor;
}

static void 
_prop_value_changed (MxSpinButton *sp,
                     GValue       *value,
                     GObject      *object)
{
  GParamSpec *prop = g_object_get_data(G_OBJECT(sp), "prop");

  GValue *val = g_malloc0(sizeof(GValue)); 
  g_value_init(val, G_VALUE_TYPE(value));
  g_value_copy(value, val);

  gboolean changed = g_param_value_validate(prop, val);
  g_object_set_property (object, prop->name, val);
  if(changed)
  {
    mx_spin_button_set_value(sp, *val);
  }
}

static ClutterActor *
_num_prop_editor_new (MxGObjectPropertiesEditor *editor,
                      GParamSpec                *spec)
{
  MxGObjectPropertiesEditorPrivate *priv = editor->priv;

  GValue value     = {0, }, 
         value_min = {0, }, 
         value_max = {0, };
  g_value_init (&value, spec->value_type);
  g_value_init (&value_min, spec->value_type);
  g_value_init (&value_max, spec->value_type);

  ClutterActor *prop_editor = NULL;

  switch (G_VALUE_TYPE (&value)) 
  {
    case G_TYPE_ULONG:
      {
        GParamSpecULong *pulong = G_PARAM_SPEC_ULONG (spec);
        prop_editor = mx_spin_button_new(G_TYPE_ULONG);
        g_value_set_ulong(&value_min, pulong->minimum);
        g_value_set_ulong(&value_max, pulong->maximum);
        break;
      }
    case G_TYPE_LONG:
      {
        GParamSpecLong *plong = G_PARAM_SPEC_LONG (spec);
        prop_editor = mx_spin_button_new(G_TYPE_LONG);
        g_value_set_long(&value_min, plong->minimum);
        g_value_set_long(&value_max, plong->maximum);
        break;
      }
    case G_TYPE_UINT:
      {
        GParamSpecUInt *puint = G_PARAM_SPEC_UINT (spec);
        prop_editor = mx_spin_button_new(G_TYPE_UINT);
        g_value_set_uint(&value_min, puint->minimum);
        g_value_set_uint(&value_max, puint->maximum);
        break;
      }
    case G_TYPE_INT:
      {
        GParamSpecInt *pint = G_PARAM_SPEC_INT (spec);
        prop_editor = mx_spin_button_new(G_TYPE_INT);
        g_value_set_int(&value_min, pint->minimum);
        g_value_set_int(&value_max, pint->maximum);
        break;
      }
    case G_TYPE_UINT64:
      {
        GParamSpecUInt64 *puint64 = G_PARAM_SPEC_UINT64 (spec);
        prop_editor = mx_spin_button_new(G_TYPE_UINT64);
        g_value_set_uint64(&value_min, puint64->minimum);
        g_value_set_uint64(&value_max, puint64->maximum);
        break;
      }
    case G_TYPE_INT64:
      {
        GParamSpecInt64 *pint64 = G_PARAM_SPEC_INT64 (spec);
        prop_editor = mx_spin_button_new(G_TYPE_INT64);
        g_value_set_int64(&value_min, pint64->minimum);
        g_value_set_int64(&value_max, pint64->maximum);
        break;
      }
    case G_TYPE_FLOAT:
      {
        GParamSpecFloat *pfloat = G_PARAM_SPEC_FLOAT (spec);
        prop_editor = mx_spin_button_new(G_TYPE_FLOAT);
        g_value_set_float(&value_min, pfloat->minimum);
        g_value_set_float(&value_max, pfloat->maximum);
        break;
      }
    case G_TYPE_DOUBLE:
      {
        GParamSpecDouble *pdouble = G_PARAM_SPEC_DOUBLE (spec);
        prop_editor = mx_spin_button_new(G_TYPE_DOUBLE);
        g_value_set_double(&value_min, pdouble->minimum);
        g_value_set_double(&value_max, pdouble->maximum);
        break;
      }
    default:
      g_assert_not_reached();
      return NULL;
      break;
  }

  mx_spin_button_set_value (MX_SPIN_BUTTON(prop_editor), value);
  mx_spin_button_set_range (MX_SPIN_BUTTON(prop_editor), 
                            &value_min, &value_max);
  g_object_set_data (G_OBJECT(prop_editor), "prop", spec);
  g_signal_connect (prop_editor, "value-changed",
                    G_CALLBACK(_prop_value_changed), priv->object);
  return prop_editor;
}

ClutterActor *
_combobox_new_from_enum (GEnumClass *klass, 
                         int         cur_value)
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
_prop_enum_changed (MxComboBox                *cbox,
                    GParamSpec                *pspec,
                    MxGObjectPropertiesEditor *editor)
{
  MxGObjectPropertiesEditorPrivate *priv = editor->priv;
  GParamSpec *prop = g_object_get_data(G_OBJECT(cbox), "prop");
  
  GHashTable *enum_hash = g_object_get_data(G_OBJECT(cbox), "hash-table");

  g_object_set(priv->object, prop->name, GPOINTER_TO_INT(g_hash_table_lookup(
          enum_hash, mx_combo_box_get_active_text(cbox))), NULL);
}

static ClutterActor *
_enum_prop_editor_new (MxGObjectPropertiesEditor *editor,
                       GParamSpec                *spec,
                       int                        current)
{
  ClutterActor *prop_editor = _combobox_new_from_enum(
      G_ENUM_CLASS(g_type_class_ref (spec->value_type)), current);
  g_object_set_data(G_OBJECT(prop_editor), "prop", spec);
  g_signal_connect(prop_editor, "notify::active-text", 
      G_CALLBACK(_prop_enum_changed), editor);
  return prop_editor;
}

static void
mx_gobject_properties_editor_add_prop (MxGObjectPropertiesEditor *editor,
                                       GParamSpec                *spec)
{
  MxGObjectPropertiesEditorPrivate *priv = editor->priv;

  priv->curr_line++;

  GValue value = {0, };
  g_value_init (&value, spec->value_type);

  ClutterActor *titre = mx_label_new_with_text (g_param_spec_get_name (spec));

  mx_table_add_actor_with_properties (MX_TABLE(editor), 
                                      titre, 
                                      priv->curr_line, 
                                      1,
                                      "x-align", MX_ALIGN_END,
                                      "x-expand", TRUE,
                                      "x-fill", FALSE,
                                      "y-align", MX_ALIGN_MIDDLE,
                                      "y-expand", TRUE,
                                      "y-fill", FALSE,
                                      NULL);

  mx_widget_set_tooltip_text(MX_WIDGET(titre), g_param_spec_get_blurb (spec));

  g_object_get_property (G_OBJECT (priv->object), spec->name, &value);

  ClutterActor *prop_editor = NULL;

  switch (G_VALUE_TYPE (&value)) 
  {
    case G_TYPE_STRING:
      prop_editor = _string_prop_editor_new (editor, spec, 
          g_value_get_string (&value));
      break;
    case G_TYPE_BOOLEAN:
      prop_editor = _bool_prop_editor_new (editor, spec, 
          g_value_get_boolean (&value));
      break;
    case G_TYPE_ULONG:
    case G_TYPE_LONG:
    case G_TYPE_UINT:
    case G_TYPE_INT:
    case G_TYPE_UINT64:
    case G_TYPE_INT64:
    case G_TYPE_FLOAT:
    case G_TYPE_DOUBLE:
      prop_editor = _num_prop_editor_new (editor, spec);
      break;
    default:
      if (G_IS_PARAM_SPEC_ENUM (spec)) 
      {
        prop_editor = _enum_prop_editor_new (editor, spec, 
            g_value_get_enum (&value));
      }
#if 0
      else if (GST_IS_PARAM_SPEC_FRACTION (param)) 
      {
        //TODO
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
#endif
      else
      {
        prop_editor = mx_label_new();
        mx_label_set_text(MX_LABEL(prop_editor), 
            "Unsupported type ... Yet...");
      }
      break;
  }
  g_value_reset (&value);

  clutter_actor_set_width (CLUTTER_ACTOR(prop_editor), 200);

  mx_table_add_actor(MX_TABLE(editor), prop_editor, priv->curr_line, 2);
}

static void 
_mx_gobj_props_editor_load_object (MxGObjectPropertiesEditor *editor)
{
  MxGObjectPropertiesEditorPrivate *priv = editor->priv;

  GList *children = 
    clutter_container_get_children (CLUTTER_CONTAINER(editor));
  GList *iter = children;
  while(NULL != iter)
  {
    clutter_container_remove_actor (CLUTTER_CONTAINER(editor), 
                                    CLUTTER_ACTOR(iter->data));
    iter = g_list_next(iter);
  }
  g_list_free(children);

  guint num_properties, i;
  GParamSpec **property_specs = 
    g_object_class_list_properties (G_OBJECT_GET_CLASS (priv->object),
                                    &num_properties);

  for (i = 0; i < num_properties; i++) 
  {
    if(property_specs[i]->flags & G_PARAM_WRITABLE)
    {
      mx_gobject_properties_editor_add_prop (editor, property_specs[i]);
    }
  }
  g_free (property_specs);
}

ClutterActor *
mx_gobject_properties_editor_new (GObject *object)
{
  return CLUTTER_ACTOR(g_object_new (MX_TYPE_GOBJECT_PROPERTIES_EDITOR,
                                     "object", object,
                                     "column-spacing", 5,
                                     "row-spacing", 2,
                                     NULL));
}

