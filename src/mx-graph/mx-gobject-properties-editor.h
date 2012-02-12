#ifndef __MX_GOBJECT_PROPERTIES_EDITOR_H__
#define __MX_GOBJECT_PROPERTIES_EDITOR_H__

#include <mx/mx.h>

G_BEGIN_DECLS

#define MX_TYPE_GOBJECT_PROPERTIES_EDITOR             \
  (mx_gobject_properties_editor_get_type ())
#define MX_GOBJECT_PROPERTIES_EDITOR(obj)             \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_GOBJECT_PROPERTIES_EDITOR, \
                               MxGObjectPropertiesEditor))
#define MX_IS_GOBJECT_PROPERTIES_EDITOR(obj)          \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_GOBJECT_PROPERTIES_EDITOR))
#define MX_GOBJECT_PROPERTIES_EDITOR_CLASS(klass)     \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_GOBJECT_PROPERTIES_EDITOR, \
                            MxGObjectPropertiesEditorClass))
#define MX_IS_GOBJECT_PROPERTIES_EDITOR_CLASS(klass)  \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_GOBJECT_PROPERTIES_EDITOR))
#define MX_GOBJECT_PROPERTIES_EDITOR_GET_CLASS(obj)   \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_GOBJECT_PROPERTIES_EDITOR, \
                              MxGObjectPropertiesEditorClass))

typedef struct _MxGObjectPropertiesEditor        MxGObjectPropertiesEditor;
typedef struct _MxGObjectPropertiesEditorPrivate \
          MxGObjectPropertiesEditorPrivate;
typedef struct _MxGObjectPropertiesEditorClass   \
          MxGObjectPropertiesEditorClass;

/**
 * MxGObjectPropertiesEditor:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxGObjectPropertiesEditor
{
  /*< private >*/
  MxTable parent_instance;

  MxGObjectPropertiesEditorPrivate *priv;
};

struct _MxGObjectPropertiesEditorClass
{
  MxTableClass parent_class;
};

GType
mx_gobject_properties_editor_get_type (void) G_GNUC_CONST;

ClutterActor *
mx_gobject_properties_editor_new      (GObject *object);

G_END_DECLS

#endif /* __MX_GOBJECT_PROPERTIES_EDITOR_H__ */
