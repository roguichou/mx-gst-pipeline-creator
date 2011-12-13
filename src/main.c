#include <gst/gst.h>
#include <mx/mx.h>
#include "mx-graph-element.h"
#include "mx-graph-element-pad.h"


int main(int argc, char **argv)
{
  if(CLUTTER_INIT_SUCCESS != clutter_init(&argc, &argv))
  {
    g_error("Clutter Init failed");
  }

  gst_init(&argc, &argv);

  ClutterActor *stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 640, 480);
/*
   ClutterActor *pad = 
  clutter_actor_set_size(pad, 10, 10);
  clutter_actor_set_position(pad, 50, 50);
  clutter_container_add_actor(CLUTTER_CONTAINER(stage), pad);

  clutter_actor_set_size(pad, 10, 10);
  clutter_actor_set_position(pad, 100, 50);
  clutter_container_add_actor(CLUTTER_CONTAINER(stage), pad);

  clutter_actor_set_size(pad, 10, 10);
  clutter_actor_set_position(pad, 75, 75);
  clutter_container_add_actor(CLUTTER_CONTAINER(stage), pad);
*/
  MxGraphElementPad *pad;
  MxGraphElement    *elt;
  
  elt = (mx_graph_element_new("TATA", "TETE"));
  clutter_actor_set_size(CLUTTER_ACTOR(elt), 100, 100);
  clutter_actor_set_position(CLUTTER_ACTOR(elt), 50, 50);
  clutter_container_add_actor(CLUTTER_CONTAINER(stage), CLUTTER_ACTOR(elt));

  pad = (mx_graph_element_pad_new("TATA", "TATA", NULL));
  mx_graph_element_add_pad(elt, pad, PAD_POSITION_WEST);

  pad = (mx_graph_element_pad_new("TETE", "TETE", NULL));
  mx_graph_element_add_pad(elt, pad, PAD_POSITION_WEST);

  pad = (mx_graph_element_pad_new("TITI", "TITI", NULL));
  mx_graph_element_add_pad(elt, pad, PAD_POSITION_EAST);

  pad = (mx_graph_element_pad_new("TUTU", "TUTU", NULL));
  mx_graph_element_add_pad(elt, pad, PAD_POSITION_EAST);

  pad = (mx_graph_element_pad_new("TYTY", "TYTY", NULL));
  mx_graph_element_add_pad(elt, pad, PAD_POSITION_EAST);


  clutter_actor_show(stage);

/*
  GList *plugins = gst_default_registry_get_plugin_list();
  while(plugins)
  {
    GstPlugin *plugin = (GstPlugin *)(plugins->data);
    plugins = g_list_next(plugins);
    g_print("plugin : %s\n", plugin->desc.name);
  }

  gint i=0;
  MxStylable *but = MX_STYLABLE(mx_button_new());

  guint nbprops = 0;
  GParamSpec **specs = mx_stylable_list_properties (but, &nbprops);

  for(i=0; i<nbprops; i++)
  {
     g_print("Spec : \n"
             "       nom     : %s\n"
             "       type    : %s (fondamental :%s)\n"
             "       details : %s\n", 
             g_param_spec_get_name(specs[i]),
             g_type_name(specs[i]->value_type),
             g_type_name(G_TYPE_FUNDAMENTAL(specs[i]->value_type)),
             g_param_spec_get_blurb(specs[i]));
  }
*/
  clutter_main();
  return 0;
}
