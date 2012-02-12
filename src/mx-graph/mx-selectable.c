#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-selectable.h"

static void
mx_selectable_default_init (MxSelectableInterface *iface);

GType
mx_selectable_get_type (void)
{
  static GType our_type = 0;

  if (G_UNLIKELY (our_type == 0))
    {
      const GTypeInfo selectable_info = {
        sizeof (MxSelectableInterface),
        (GBaseInitFunc)mx_selectable_default_init,
        NULL, /* base_finalize */
      };

      our_type = g_type_register_static (G_TYPE_INTERFACE,
          g_intern_static_string ("MxSelectable"),
          &selectable_info, 0);

      g_type_interface_add_prerequisite (our_type, CLUTTER_TYPE_ACTOR);
    }

  return our_type;
}

static void
mx_selectable_default_init (MxSelectableInterface *iface)
{
  static gboolean is_initialized = FALSE;

  if (G_UNLIKELY (!is_initialized))
  {
    is_initialized = TRUE;
    GParamSpec *pspec;
    pspec = g_param_spec_boolean ("selected",
                                  "Selected",
                                  "Selected",
                                  FALSE,
                                  G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_interface_install_property (iface, pspec);
  }
}

void
mx_selectable_set_selected (MxSelectable *selectable,
                            gboolean      selected)
{
  MxSelectableInterface *iface = MX_SELECTABLE_GET_IFACE(selectable);
  gboolean curr;
  g_object_get (G_OBJECT (selectable), "selected", &curr, NULL);
  if (curr != selected)
  {
    if (NULL != iface->set_selected)
    {
      return iface->set_selected (selectable, selected);
    }
    g_object_notify (G_OBJECT(selectable), "selected");
  }
}

void
mx_selectable_draw_around_actor (MxSelectable     *selectable, 
                                 ClutterRectangle *rect)
{
  g_return_if_fail (CLUTTER_IS_ACTOR (selectable));
  gboolean selected;
  guint width = clutter_rectangle_get_border_width(rect);
  g_object_get (G_OBJECT (selectable), 
                "selected", &selected, 
                NULL);
  if (selected)
  {
    cogl_path_new ();
    cogl_set_source_color4ub (0, 0, 0, 255);
    ClutterActorBox  box;
    clutter_actor_get_allocation_box (CLUTTER_ACTOR (rect), &box);
    cogl_path_rectangle (box.x1-1,       box.y1-1,
                         box.x1+width+1, box.y1+width+1);
    cogl_path_rectangle (box.x2-width-1, box.y1-1,
                         box.x2+1,       box.y1+width+1);
    cogl_path_rectangle (box.x1-1,       box.y2-width-1,
                         box.x1+width+1, box.y2+1);
    cogl_path_rectangle (box.x2-width-1, box.y2-width-1,
                         box.x2+1,       box.y2+1);
    cogl_path_fill();
  }
}

