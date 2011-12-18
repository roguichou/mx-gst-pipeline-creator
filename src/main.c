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
/*
  GList *plugins = gst_default_registry_get_plugin_list();
  while(plugins)
  {
    GstPlugin *plugin = (GstPlugin *)(plugins->data);
    plugins = g_list_next(plugins);
    g_print("plugin : %s\n", plugin->desc.name);
  }
*/
 
  ClutterActor *stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 640, 480);

  MxGraphElementPad *pad;
  MxGraphElement    *elt;
  
  elt = (mx_graph_element_new("TATA", "TETE"));
  clutter_actor_set_size(CLUTTER_ACTOR(elt), 100, 100);
  clutter_actor_set_position(CLUTTER_ACTOR(elt), 50, 50);
  clutter_container_add_actor(CLUTTER_CONTAINER(stage), CLUTTER_ACTOR(elt));
  mx_draggable_set_axis (MX_DRAGGABLE (elt), MX_DRAG_AXIS_NONE);
  mx_draggable_enable (MX_DRAGGABLE (elt));

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


  elt = (mx_graph_element_new("SISI", "NONO"));
  clutter_actor_set_size(CLUTTER_ACTOR(elt), 100, 100);
  clutter_actor_set_position(CLUTTER_ACTOR(elt), 250, 50);
  clutter_container_add_actor(CLUTTER_CONTAINER(stage), CLUTTER_ACTOR(elt));
  mx_draggable_set_axis (MX_DRAGGABLE (elt), MX_DRAG_AXIS_NONE);
  mx_draggable_enable (MX_DRAGGABLE (elt));

  pad = (mx_graph_element_pad_new("NONO", "le petit robot", NULL));
  mx_graph_element_add_pad(elt, pad, PAD_POSITION_WEST);
  clutter_actor_show(stage);

  clutter_main();
  return 0;
}
