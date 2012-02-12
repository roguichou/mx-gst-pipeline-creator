#include <gst/gst.h>
#include <mx/mx.h>
#include "mx-gst-graph-element.h"
#include "mx-gst-pipeline.h"

GstElementFactory *st_videotestsrc_factory, 
                  *st_tee_factory;

void _gst_init()
{

  GList *plugins = gst_default_registry_get_plugin_list();
  while(plugins)
  {
    GstPlugin *plugin = (GstPlugin *)(plugins->data);
    GList *features, *orig_features;
    orig_features = features =
      gst_registry_get_feature_list_by_plugin (gst_registry_get_default (),
          plugin->desc.name);
    while (features)
    {
      GstPluginFeature *feature;

      if (G_UNLIKELY (features->data == NULL))
        goto next;
      feature = GST_PLUGIN_FEATURE (features->data);

      if (GST_IS_ELEMENT_FACTORY (feature)) 
      {
        GstElementFactory *factory;

        factory = GST_ELEMENT_FACTORY (feature);
        if(0 == g_strcmp0(GST_PLUGIN_FEATURE_NAME (factory), "tee"))
        {
          st_tee_factory = factory;
          g_print ("%s:  %s: %s\n", plugin->desc.name,
              GST_PLUGIN_FEATURE_NAME (factory),
              gst_element_factory_get_longname (factory));
          g_print("###########################################\n");
          const GList *pads_templates = 
            gst_element_factory_get_static_pad_templates(factory);
          GList *pads_iter = (GList *)pads_templates;
          while(pads_iter)
          {
            GstStaticPadTemplate *pad = (GstStaticPadTemplate *)(
                pads_iter->data);
            gchar *dirS = "unknown";
            if(GST_PAD_SRC == pad->direction)
            {
              dirS = "src";
            }
            else if(GST_PAD_SINK == pad->direction)
            {
              dirS = "sink";
            }

            gchar *presence = "always";
            if(GST_PAD_SOMETIMES == pad->presence)
            {
              presence = "sometimes";
            }
            else if(GST_PAD_REQUEST == pad->presence)
            {
              presence = "request";
            }

            g_print("Pad : %s (%s) : %s\n", pad->name_template, dirS,
                presence);
            pads_iter = g_list_next(pads_iter);
          }
          g_print("###########################################\n");
        }
        else if(0 == g_strcmp0(GST_PLUGIN_FEATURE_NAME (factory), 
          "videotestsrc"))
        {
          st_videotestsrc_factory = factory;
        }
      } 

next:
      features = g_list_next (features);
    }

    gst_plugin_feature_list_free (orig_features);


    plugins = g_list_next(plugins);
  }
  gst_plugin_list_free (plugins);
}

ClutterActor *detailsExpander = NULL;
ClutterActor *propsExpander = NULL;

static void init_side_bar()
{
  ClutterActor *vbox = mx_box_layout_new();
  mx_box_layout_set_orientation (MX_BOX_LAYOUT(vbox),
                                 MX_ORIENTATION_VERTICAL);
  clutter_actor_set_size(vbox, 350, 768);
  
  detailsExpander = mx_expander_new();
  mx_expander_set_label(MX_EXPANDER(detailsExpander), "Details");
  clutter_container_add_actor (CLUTTER_CONTAINER(vbox),
                               detailsExpander);
  
  propsExpander = mx_expander_new();
  mx_expander_set_label(MX_EXPANDER(propsExpander), "Properties");
  clutter_container_add_actor (CLUTTER_CONTAINER(vbox),
                               propsExpander);
  
  clutter_container_add_actor (
      CLUTTER_CONTAINER(clutter_stage_get_default()),
      vbox);
}

int main(int argc, char **argv)
{
  if(CLUTTER_INIT_SUCCESS != clutter_init(&argc, &argv))
  {
    g_error("Clutter Init failed");
  }

  gst_init(&argc, &argv);
  _gst_init();
 
  ClutterActor *stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 1024, 768);

  init_side_bar();

  ClutterActor *pipeline = CLUTTER_ACTOR(mx_gst_pipeline_new());
  clutter_container_add_actor (CLUTTER_CONTAINER(stage), pipeline);
  clutter_actor_set_size(pipeline, 1024-350, 768);
  clutter_actor_set_position(pipeline, 350, 0);

  MxGstGraphElement    *elt;
  elt = mx_gst_graph_element_new(st_videotestsrc_factory);
  clutter_actor_set_size(CLUTTER_ACTOR(elt), 125, 100);
  clutter_actor_set_position(CLUTTER_ACTOR(elt), 50, 50);
  clutter_container_add_actor (CLUTTER_CONTAINER(pipeline), 
                               CLUTTER_ACTOR(elt));
  mx_draggable_set_axis (MX_DRAGGABLE (elt), MX_DRAG_AXIS_NONE);
  mx_draggable_enable (MX_DRAGGABLE (elt));

  elt = mx_gst_graph_element_new(st_tee_factory);
  clutter_actor_set_size(CLUTTER_ACTOR(elt), 125, 100);
  clutter_actor_set_position(CLUTTER_ACTOR(elt), 250, 50);
  clutter_container_add_actor (CLUTTER_CONTAINER(pipeline),
                               CLUTTER_ACTOR(elt));
  mx_draggable_set_axis (MX_DRAGGABLE (elt), MX_DRAG_AXIS_NONE);
  mx_draggable_enable (MX_DRAGGABLE (elt));

  clutter_actor_show(stage);
  clutter_main();
  return 0;
}
