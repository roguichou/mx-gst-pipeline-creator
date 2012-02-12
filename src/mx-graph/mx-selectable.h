#ifndef __MX_SELECTABLE_H__
#define __MX_SELECTABLE_H__

#include <glib-object.h>
#include <mx/mx.h>

G_BEGIN_DECLS

#define MX_TYPE_SELECTABLE             (mx_selectable_get_type ())
#define MX_SELECTABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
      MX_TYPE_SELECTABLE, MxSelectable))
#define MX_IS_SELECTABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
      MX_TYPE_SELECTABLE))
#define MX_SELECTABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj),\
      MX_TYPE_SELECTABLE, MxSelectableInterface))

typedef struct _MxSelectable               MxSelectable;
typedef struct _MxSelectableInterface      MxSelectableInterface;

struct _MxSelectableInterface
{
  /*< private >*/
  GTypeInterface g_iface;

  /*< public >*/
  void (*set_selected) (MxSelectable *selectable,
                        gboolean      selected);
};

GType mx_selectable_get_type (void) G_GNUC_CONST;

void mx_selectable_set_selected (MxSelectable *selectable,
                                 gboolean      selected);

void mx_selectable_draw_around_actor (MxSelectable     *selectable, 
                                      ClutterRectangle *rect);
G_END_DECLS

#endif //__MX_SELECTABLE_H__
