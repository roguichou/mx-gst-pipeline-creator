#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include "mx-gst-graph-element.h"
#include "mx-gst-graph-element-pad.h"
#include "mx-spin-button.h"
#include "mx-gobject-properties-editor.h"

struct _MxGstGraphElementPrivate
{
  GstPlugin         *plugin;
  GstElement        *element;
  GstElementFactory *factory;
  gfloat             xpress,
                     ypress;
  ClutterActor      *props_editor;
};

enum
{
  DETAILS_CHANGED,
  PROPS_EDITOR_CHANGED,

  LAST_SIGNAL
};

static guint element_signals[LAST_SIGNAL] = { 0, };

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
    g_signal_emit(actor, element_signals[PROPS_EDITOR_CHANGED], 0, 
      priv->props_editor);
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
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  
  element_signals[DETAILS_CHANGED] =
    g_signal_new ("details-changed",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (MxGstGraphElementClass, details_changed),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1, G_TYPE_STRING);

  element_signals[PROPS_EDITOR_CHANGED] =
    g_signal_new ("props-editor-changed",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (MxGstGraphElementClass, 
                    props_editor_changed),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);
  
}

static void
mx_gst_graph_element_init (MxGstGraphElement *actor)
{
  actor->priv = G_TYPE_INSTANCE_GET_PRIVATE(
      actor, MX_TYPE_GST_GRAPH_ELEMENT, MxGstGraphElementPrivate);
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

static void
_pad_selection_has_changed (MxGstGraphElementPad *pad, 
                            GParamSpec           *pspec,
                            MxGstGraphElement    *actor)
{
  gboolean selected;
  g_object_get(pad, "selected", &selected, NULL);
  gchar *details = NULL;
  if (selected)
  {
    details = mx_gst_pad_get_details(pad);
  }
  g_signal_emit(actor, element_signals[DETAILS_CHANGED], 0, details);
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
    g_signal_connect (G_OBJECT(eltPad), 
                      "notify::selected", 
                      G_CALLBACK(_pad_selection_has_changed), 
                      actor);
    pads_iter = g_list_next(pads_iter);
  }

  priv->props_editor = 
    mx_gobject_properties_editor_new (G_OBJECT(priv->element));
  g_object_ref (priv->props_editor);

}

