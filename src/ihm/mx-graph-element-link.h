#ifndef _MX_GRAPH_ELEMENT_LINK_H_
#define _MX_GRAPH_ELEMENT_LINK_H_

#include <mx/mx.h>
#include "mx-graph-element-pad.h"

G_BEGIN_DECLS

#define MX_TYPE_GRAPH_ELEMENT_LINK                               \
  (mx_graph_element_link_get_type())

#define MX_GRAPH_ELEMENT_LINK(obj)                               \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),                             \
                              MX_TYPE_GRAPH_ELEMENT_LINK,        \
                              MxGraphElementLink))

#define MX_IS_GRAPH_ELEMENT_LINK(obj)                            \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),                             \
                              MX_TYPE_GRAPH_ELEMENT_LINK)

#define MX_GRAPH_ELEMENT_LINK_CLASS(klass)                       \
   (G_TYPE_CHECK_CLASS_CAST ((klass),                            \
                             MX_TYPE_GRAPH_ELEMENT_LINK,         \
                             MxGraphElementLinkClass))

#define MX_IS_GRAPH_ELEMENT_LINK_CLASS(klass)                    \
   (G_TYPE_CHECK_CLASS_TYPE ((klass),                            \
                             MX_TYPE_GRAPH_ELEMENT_LINK))

#define MX_GRAPH_ELEMENT_LINK_GET_CLASS(obj)                     \
   (G_TYPE_INSTANCE_GET_CLASS ((obj),                            \
                               MX_TYPE_GRAPH_ELEMENT_LINK,       \
                               MxGraphElementLink))

typedef struct _MxGraphElementLink        MxGraphElementLink;
typedef struct _MxGraphElementLinkClass   MxGraphElementLinkClass;
typedef struct _MxGraphElementLinkPrivate MxGraphElementLinkPrivate;

struct _MxGraphElementLink
{
  MxWidget               parent_instance;
  MxGraphElementLinkPrivate *priv;
};

struct _MxGraphElementLinkClass
{
  MxWidgetClass parent_class;
};

GType mx_graph_element_link_get_type (void) G_GNUC_CONST;

MxGraphElementLink *mx_graph_element_link_new(gfloat xorig, gfloat yorig);

void
mx_graph_element_link_set_dest_point (MxGraphElementLink *actor, 
    gfloat xdest, gfloat ydest);

void
mx_graph_element_link_set_orig_point (MxGraphElementLink *actor, 
    gfloat xori, gfloat yori);

G_END_DECLS

#endif //_MX_GRAPH_ELEMENT_LINK_H_
