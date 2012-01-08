#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-gst-graph-element-pad.h"

struct _MxGstGraphElementPadPrivate
{
  GstStaticPadTemplate      *pad_template;
  GstPad                    *pad;
  MxGraphElementPadPosition  position;
  gboolean                   but_pressed;
  ClutterActor              *dialog;
};

G_DEFINE_TYPE(MxGstGraphElementPad, 
              mx_gst_graph_element_pad, 
              MX_TYPE_GRAPH_ELEMENT_PAD);

static gboolean 
mx_gst_graph_element_pad_leave(ClutterActor *actor,
    ClutterCrossingEvent *event)
{
  MxGstGraphElementPad *pad = MX_GST_GRAPH_ELEMENT_PAD(actor);
  MxGstGraphElementPadPrivate *priv = pad->priv;
  priv->but_pressed = FALSE;
  //for the tooltip
  return CLUTTER_ACTOR_CLASS (mx_gst_graph_element_pad_parent_class)->
    leave_event (actor, event);
}

static gboolean 
mx_gst_graph_element_pad_button_press (ClutterActor       *actor, 
                                   ClutterButtonEvent *event)
{
  MxGstGraphElementPad *pad = MX_GST_GRAPH_ELEMENT_PAD(actor);
  MxGstGraphElementPadPrivate *priv = pad->priv;
  priv->but_pressed = TRUE;

  return CLUTTER_ACTOR_CLASS (mx_gst_graph_element_pad_parent_class)->
    button_press_event (actor, event);
}

static void 
mx_gst_graph_element_pad_close_dialog(gpointer unused,
                                      ClutterActor *dialog)
{
  clutter_actor_hide (dialog);
}

static gboolean 
mx_gst_graph_element_pad_button_release(ClutterActor       *actor, 
                                   ClutterButtonEvent *event)
{
  MxGstGraphElementPad *pad = MX_GST_GRAPH_ELEMENT_PAD(actor);
  MxGstGraphElementPadPrivate *priv = pad->priv;
  if(priv->but_pressed)
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
  priv->but_pressed = FALSE;
  return FALSE;
}

static void
mx_gst_graph_element_pad_class_init (MxGstGraphElementPadClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGstGraphElementPadPrivate));
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  actor_class->button_press_event = mx_gst_graph_element_pad_button_press;
  actor_class->button_release_event = 
    mx_gst_graph_element_pad_button_release;
  actor_class->leave_event = mx_gst_graph_element_pad_leave;
}

static void
mx_gst_graph_element_pad_init (MxGstGraphElementPad *actor)
{
  actor->priv = G_TYPE_INSTANCE_GET_PRIVATE(
    actor, MX_TYPE_GST_GRAPH_ELEMENT_PAD, MxGstGraphElementPadPrivate);
}

static gboolean
_caps_print_field (GQuark field, const GValue * value, gpointer caps_str)
{
  gchar *str = gst_value_serialize (value);
  g_string_append_printf ((GString *)caps_str, "  %15s: %s\n", 
      g_quark_to_string (field), str);
  g_free (str);
  return TRUE;
}

static GString *
_print_caps(GstCaps *caps, gboolean full)
{
  GString *caps_str = g_string_new("Caps: ");
  if (gst_caps_is_any (caps)) 
  {
    g_string_append_printf(caps_str, "ANY\n");
  }
  else if (gst_caps_is_empty (caps)) 
  {
    g_string_append_printf(caps_str, "EMPTY\n");
  }
  else
  {
    g_string_append_printf(caps_str, "\n");
    int i=0;
    if(!full)
    {
      GList *caps_names = NULL;
      for (i = 0; i < gst_caps_get_size (caps); i++) 
      {
        GstStructure *structure = gst_caps_get_structure (caps, i);
        gchar *cap_name = (gchar *)gst_structure_get_name(structure);
        if(NULL == g_list_find_custom(caps_names, cap_name, 
              (GCompareFunc)g_strcmp0))
        {
          caps_names = g_list_prepend(caps_names, cap_name);
        }
      }
      caps_names = g_list_reverse(caps_names);
      while(NULL != caps_names)
      {
        gchar *name = (gchar *)caps_names->data;
        caps_names = g_list_delete_link(caps_names, caps_names);
        g_string_append_printf(caps_str, "%s\n", name);
      }
    }
    else
    {
      for (i = 0; i < gst_caps_get_size (caps); i++) 
      {
        GstStructure *structure = gst_caps_get_structure (caps, i);
        g_string_append_printf(caps_str, "%s\n", 
            gst_structure_get_name (structure));
        gst_structure_foreach (structure, _caps_print_field, 
          (gpointer) caps_str);
      }
    }
  }
  caps_str = g_string_truncate(caps_str, caps_str->len-1);
  return caps_str;
}

static void
mx_gst_graph_element_pad_create_info_dialog(MxGstGraphElementPad *pad)
{
  MxGstGraphElementPadPrivate *priv = pad->priv;
  priv->dialog = mx_dialog_new();
  MxAction *action_quit = mx_action_new_full("close", "Close",
      G_CALLBACK(mx_gst_graph_element_pad_close_dialog), priv->dialog);
  mx_dialog_add_action(MX_DIALOG(priv->dialog), action_quit);

  ClutterActor *scroll_view = mx_scroll_view_new();
  ClutterActor *kinetic = mx_kinetic_scroll_view_new ();
  mx_bin_set_child (MX_BIN (priv->dialog), scroll_view);
  clutter_actor_set_size (scroll_view, 275, 200);

  ClutterActor *view = mx_viewport_new ();
  mx_viewport_set_sync_adjustments (MX_VIEWPORT (view), FALSE);
  clutter_container_add_actor (CLUTTER_CONTAINER (kinetic), view);
  clutter_container_add_actor (CLUTTER_CONTAINER (scroll_view), kinetic);

  GString *txt = g_string_new("");
  g_string_append_printf(txt, "Name:  %s\n", 
      gst_pad_get_name(priv->pad));
  GString *caps_str = _print_caps(gst_pad_get_caps(priv->pad), TRUE);
  txt = g_string_append(txt, caps_str->str);  
  ClutterActor *label = mx_label_new_with_text (txt->str);
  clutter_container_add_actor (CLUTTER_CONTAINER (view), label);
  g_string_free(caps_str, TRUE);
  g_string_free(txt, TRUE);

  MxAdjustment *vadjust;
  mx_scrollable_get_adjustments (MX_SCROLLABLE (view), NULL, &vadjust);
  mx_adjustment_set_values (vadjust,
      0, 0, clutter_actor_get_height(label), 60, 120, 200);

  clutter_container_add_actor(
      CLUTTER_CONTAINER(clutter_stage_get_default()),
      priv->dialog); 
}

static gboolean
mx_gst_pad_is_compatible(MxGstGraphElementPad *src, 
    MxGstGraphElementPad *dest)
{
  GstCaps *src_caps  = gst_pad_get_caps(src->priv->pad);
  GstCaps *dest_caps = gst_pad_get_caps(dest->priv->pad);
  
  if( (GST_PAD_SRC != gst_pad_get_direction(src->priv->pad)) ||
      (GST_PAD_SINK != gst_pad_get_direction(dest->priv->pad)) )
  {
    return FALSE;
  }

  if(gst_caps_is_any(src_caps) || gst_caps_is_any(dest_caps))
  {
    return TRUE;
  }

  if(gst_caps_is_empty(src_caps) || gst_caps_is_empty(dest_caps))
  {
    return FALSE;
  }

  return !gst_caps_is_empty(gst_caps_intersect(src_caps, dest_caps));
}

MxGstGraphElementPad *
mx_gst_graph_element_pad_new (GstStaticPadTemplate *pad_template, 
    GstElement *element)
{
  GstPad *pad = NULL;
  if(GST_PAD_ALWAYS == pad_template->presence)
  {
    pad = gst_element_get_pad(element, 
        (gchar *)pad_template->name_template);
  }
  else
  {
    pad = gst_element_request_pad(element,
        gst_static_pad_template_get(pad_template), NULL, NULL);
  }

  GString *caps_str = _print_caps(gst_pad_get_caps(pad), FALSE);
  gchar *name = g_strdup_printf("Name:  %s", gst_pad_get_name(pad));

  MxGstGraphElementPad *eltPad = MX_GST_GRAPH_ELEMENT_PAD(
      g_object_new(MX_TYPE_GST_GRAPH_ELEMENT_PAD, 
        "name",  name,
        "blurb", caps_str->str,
        "is-compatible-func", mx_gst_pad_is_compatible,
        NULL));

  g_string_free(caps_str, TRUE);

  MxGstGraphElementPadPrivate *priv = eltPad->priv;
  priv->pad_template = pad_template;
  priv->pad = pad;

  priv->position = 
    (GST_PAD_SRC  == pad_template->direction) ? PAD_POSITION_EAST :
    (GST_PAD_SINK == pad_template->direction) ? PAD_POSITION_WEST:
    PAD_POSITION_SOUTH;

  mx_gst_graph_element_pad_create_info_dialog (eltPad);
  return eltPad;
}

MxGraphElementPadPosition
mx_gst_graph_element_pad_get_position (MxGstGraphElementPad *pad)
{
  MxGstGraphElementPadPrivate *priv = pad->priv;
  return priv->position;
}

