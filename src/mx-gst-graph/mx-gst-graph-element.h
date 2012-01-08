#ifndef _MX_GST_GST_GRAPH_ELEMENT_H_
#define _MX_GST_GST_GRAPH_ELEMENT_H_

#include <gst/gst.h>
#include "mx-graph-element.h"

G_BEGIN_DECLS

#define MX_TYPE_GST_GRAPH_ELEMENT            (mx_gst_graph_element_get_type())

#define MX_GST_GRAPH_ELEMENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), \
  MX_TYPE_GST_GRAPH_ELEMENT, MxGstGraphElement))

#define MX_IS_GST_GRAPH_ELEMENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
  MX_TYPE_GST_GRAPH_ELEMENT))

#define MX_GST_GRAPH_ELEMENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_GST_GRAPH_ELEMENT, MxGstGraphElementClass))

#define MX_IS_GST_GRAPH_ELEMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_GST_GRAPH_ELEMENT))

#define MX_GST_GRAPH_ELEMENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),  \
  MX_TYPE_GST_GRAPH_ELEMENT, MxGstGraphElement))

typedef struct _MxGstGraphElement        MxGstGraphElement;
typedef struct _MxGstGraphElementClass   MxGstGraphElementClass;
typedef struct _MxGstGraphElementPrivate MxGstGraphElementPrivate;

struct _MxGstGraphElement
{
  MxGraphElement            parent_instance;
  MxGstGraphElementPrivate *priv;
};

struct _MxGstGraphElementClass
{
  MxGraphElementClass parent_class;
};

GType mx_gst_graph_element_get_type (void) G_GNUC_CONST;

MxGstGraphElement *
mx_gst_graph_element_new(GstElementFactory *factory);

G_END_DECLS

#endif //_MX_GST_GRAPH_ELEMENT_H_
