#ifndef _MX_GST_GRAPH_ELEMENT_PAD_H_
#define _MX_GST_GRAPH_ELEMENT_PAD_H_

#include <gst/gst.h>
#include <mx/mx.h>
#include "mx-graph-element.h"

G_BEGIN_DECLS

#define MX_TYPE_GST_GRAPH_ELEMENT_PAD            (mx_gst_graph_element_pad_get_type())

#define MX_GST_GRAPH_ELEMENT_PAD(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),\
  MX_TYPE_GRAPH_ELEMENT_PAD, MxGstGraphElementPad))

#define MX_IS_GRAPH_ELEMENT_PAD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),\
  MX_TYPE_GRAPH_ELEMENT_PAD))

#define MX_GST_GRAPH_ELEMENT_PAD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),\
  MX_TYPE_GRAPH_ELEMENT_PAD, MxGstGraphElementPadClass))

#define MX_IS_GRAPH_ELEMENT_PAD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
  MX_TYPE_GRAPH_ELEMENT_PAD))

#define MX_GST_GRAPH_ELEMENT_PAD_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_GRAPH_ELEMENT_PAD, MxGstGraphElementPad))

typedef struct _MxGstGraphElementPad        MxGstGraphElementPad;
typedef struct _MxGstGraphElementPadClass   MxGstGraphElementPadClass;
typedef struct _MxGstGraphElementPadPrivate MxGstGraphElementPadPrivate;

struct _MxGstGraphElementPad
{
  MxGraphElementPad parent_instance;
  MxGstGraphElementPadPrivate *priv;
};

struct _MxGstGraphElementPadClass
{
  MxGraphElementPadClass parent_class;
};

GType mx_gst_graph_element_pad_get_type (void) G_GNUC_CONST;

MxGstGraphElementPad *
mx_gst_graph_element_pad_new (GstStaticPadTemplate *pad_template,
    GstElement *element);

MxGraphElementPadPosition
mx_gst_graph_element_pad_get_position (MxGstGraphElementPad *pad);

G_END_DECLS

#endif //_MX_GST_GRAPH_ELEMENT_PAD_H_
