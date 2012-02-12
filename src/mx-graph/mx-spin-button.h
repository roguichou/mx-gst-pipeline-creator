#ifndef __MX_SPIN_BUTTON_H__
#define __MX_SPIN_BUTTON_H__

#include <mx/mx.h>

G_BEGIN_DECLS

#define MX_TYPE_SPIN_BUTTON                (mx_spin_button_get_type ())
#define MX_SPIN_BUTTON(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), MX_TYPE_SPIN_BUTTON, MxSpinButton))
#define MX_IS_SPIN_BUTTON(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MX_TYPE_SPIN_BUTTON))
#define MX_SPIN_BUTTON_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MX_TYPE_SPIN_BUTTON, MxSpinButtonClass))
#define MX_IS_SPIN_BUTTON_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MX_TYPE_SPIN_BUTTON))
#define MX_SPIN_BUTTON_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MX_TYPE_SPIN_BUTTON, MxSpinButtonClass))

typedef struct _MxSpinButton              MxSpinButton;
typedef struct _MxSpinButtonPrivate       MxSpinButtonPrivate;
typedef struct _MxSpinButtonClass         MxSpinButtonClass;

/**
 * MxSpinButton:
 *
 * The contents of this structure is private and should only be accessed using
 * the provided API.
 */
struct _MxSpinButton
{
  /*< private >*/
  MxWidget parent_instance;

  MxSpinButtonPrivate *priv;
};

struct _MxSpinButtonClass
{
  MxWidgetClass parent_class;

  /* signals, not vfuncs */
  void (* value_changed) (MxSpinButton *self,
                          GValue *value);
};

GType
mx_spin_button_get_type  (void) G_GNUC_CONST;

ClutterActor *
mx_spin_button_new       (GType type);

void
mx_spin_button_set_value (MxSpinButton *spin_button, 
                          GValue        value);

void
mx_spin_button_set_range (MxSpinButton *spin_button, 
                          GValue       *minval,
                          GValue       *maxval);

G_END_DECLS

#endif /* __MX_SPIN_BUTTON_H__ */
