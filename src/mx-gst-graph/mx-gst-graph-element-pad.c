#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-gst-graph-element-pad.h"

struct _MxGstGraphElementPadPrivate
{
  GstStaticPadTemplate      *pad_template;
  GstPad                    *pad;
  MxGraphElementPadPosition  position;
  gchar                     *details;
};

G_DEFINE_TYPE(MxGstGraphElementPad, 
              mx_gst_graph_element_pad, 
              MX_TYPE_GRAPH_ELEMENT_PAD);

static void
mx_gst_graph_element_pad_class_init (MxGstGraphElementPadClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGstGraphElementPadPrivate));
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
mx_gst_graph_element_pad_create_info_txt (MxGstGraphElementPad *pad)
{
  MxGstGraphElementPadPrivate *priv = pad->priv;

  GString *txt = g_string_new("");
  g_string_append_printf(txt, "Name:  %s\n", 
      gst_pad_get_name(priv->pad));
  GString *caps_str = _print_caps(gst_pad_get_caps(priv->pad), TRUE);
  txt = g_string_append(txt, caps_str->str);  
  g_string_free(caps_str, TRUE);
  priv->details = g_string_free(txt, FALSE);
}

gchar *
mx_gst_pad_get_details(MxGstGraphElementPad *pad)
{
  return pad->priv->details;
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

  mx_gst_graph_element_pad_create_info_txt (eltPad);
  return eltPad;
}

MxGraphElementPadPosition
mx_gst_graph_element_pad_get_position (MxGstGraphElementPad *pad)
{
  MxGstGraphElementPadPrivate *priv = pad->priv;
  return priv->position;
}

