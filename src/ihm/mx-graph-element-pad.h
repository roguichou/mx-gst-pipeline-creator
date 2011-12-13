#ifndef _MX_GRAPH_ELEMENT_PAD_H_
#define _MX_GRAPH_ELEMENT_PAD_H_

#include <mx/mx.h>

G_BEGIN_DECLS

#define MX_TYPE_GRAPH_ELEMENT_PAD            (mx_graph_element_pad_get_type())

#define MX_GRAPH_ELEMENT_PAD(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),\
  MX_TYPE_GRAPH_ELEMENT_PAD, MxGraphElementPad))

#define MX_IS_GRAPH_ELEMENT_PAD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),\
  MX_TYPE_GRAPH_ELEMENT_PAD))

#define MX_GRAPH_ELEMENT_PAD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),\
  MX_TYPE_GRAPH_ELEMENT_PAD, MxGraphElementPadClass))

#define MX_IS_GRAPH_ELEMENT_PAD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
  MX_TYPE_GRAPH_ELEMENT_PAD))

#define MX_GRAPH_ELEMENT_PAD_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MX_TYPE_GRAPH_ELEMENT_PAD, MxGraphElementPad))

typedef struct _MxGraphElementPad        MxGraphElementPad;
typedef struct _MxGraphElementPadClass   MxGraphElementPadClass;
typedef struct _MxGraphElementPadPrivate MxGraphElementPadPrivate;

struct _MxGraphElementPad
{
  MxWidget                 parent_instance;
  MxGraphElementPadPrivate *priv;
};

struct _MxGraphElementPadClass
{
  MxWidgetClass parent_class;
};

typedef gboolean (MxGraphElementPadIsCompatible)(MxGraphElementPad *pad);

GType mx_graph_element_pad_get_type (void) G_GNUC_CONST;

MxGraphElementPad *mx_graph_element_pad_new(gchar *name, gchar *short_desc,
  MxGraphElementPadIsCompatible *is_compatible_func);

G_END_DECLS

#endif //_MX_GRAPH_ELEMENT_PAD_H_
