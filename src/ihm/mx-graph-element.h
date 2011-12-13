#ifndef _MX_GRAPH_ELEMENT_H_
#define _MX_GRAPH_ELEMENT_H_

#include <mx/mx.h>
#include "mx-graph-element-pad.h"

G_BEGIN_DECLS

#define MX_TYPE_GRAPH_ELEMENT            (mx_graph_element_get_type())

#define MX_GRAPH_ELEMENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), \
  MX_TYPE_GRAPH_ELEMENT, MxGraphElement))

#define MX_IS_GRAPH_ELEMENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
  MX_TYPE_GRAPH_ELEMENT)

#define MX_GRAPH_ELEMENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_GRAPH_ELEMENT, MxGraphElementClass))

#define MX_IS_GRAPH_ELEMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_GRAPH_ELEMENT))

#define MX_GRAPH_ELEMENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),  \
  MX_TYPE_GRAPH_ELEMENT, MxGraphElement))

typedef enum
{
  PAD_POSITION_NORTH,
  PAD_POSITION_SOUTH,
  PAD_POSITION_EAST,
  PAD_POSITION_WEST
}MxGraphElementPadPosition;

typedef struct _MxGraphElement        MxGraphElement;
typedef struct _MxGraphElementClass   MxGraphElementClass;
typedef struct _MxGraphElementPrivate MxGraphElementPrivate;

struct _MxGraphElement
{
  MxWidget               parent_instance;
  MxGraphElementPrivate *priv;
};

struct _MxGraphElementClass
{
  MxWidgetClass parent_class;
};

GType mx_graph_element_get_type (void) G_GNUC_CONST;

void 
mx_graph_element_add_pad(MxGraphElement *elt, MxGraphElementPad *pad, 
    MxGraphElementPadPosition position);

MxGraphElement *mx_graph_element_new(gchar *name, gchar *short_desc);

G_END_DECLS

#endif //_MX_GRAPH_ELEMENT_H_
