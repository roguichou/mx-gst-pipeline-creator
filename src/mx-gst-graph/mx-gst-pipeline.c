#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-gst-pipeline.h"

#define MX_GST_PIPELINE_GET_PRIVATE(obj)              \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj),                \
                                MX_TYPE_GST_PIPELINE, \
                                MxGstPipelinePrivate))

struct _MxGstPipelinePrivate
{
  gpointer dummy;
};

static void _container_iface_init (ClutterContainerIface *iface);

G_DEFINE_TYPE_WITH_CODE (MxGstPipeline, mx_gst_pipeline, CLUTTER_TYPE_GROUP,
                         G_IMPLEMENT_INTERFACE (CLUTTER_TYPE_CONTAINER,
                                                _container_iface_init));


static void
_real_remove (ClutterContainer *container,
              ClutterActor     *actor)
{
  ClutterContainerIface *iface_class, *old_iface_class;

  iface_class = G_TYPE_INSTANCE_GET_INTERFACE(container, 
                                              CLUTTER_TYPE_CONTAINER,
                                              ClutterContainerIface);
  old_iface_class = g_type_interface_peek_parent(iface_class);

  old_iface_class->remove(container, actor);

  //.TODO.
}

static void
_real_add (ClutterContainer *container,
           ClutterActor     *actor)
{
  ClutterContainerIface *iface_class, *old_iface_class;

  iface_class = G_TYPE_INSTANCE_GET_INTERFACE(container, 
                                              CLUTTER_TYPE_CONTAINER,
                                              ClutterContainerIface);
  old_iface_class = g_type_interface_peek_parent(iface_class);

  old_iface_class->add(container, actor);

  //.TODO.
}

static void
_container_iface_init (ClutterContainerIface *iface)
{
  iface->add = _real_add;
  iface->remove = _real_remove;
}

static void
mx_gst_pipeline_class_init (MxGstPipelineClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGstPipelinePrivate));
  //.TODO.
}

static void
mx_gst_pipeline_init(MxGstPipeline *pipeline)
{
  pipeline->priv = MX_GST_PIPELINE_GET_PRIVATE(pipeline);
  //.TODO.
}

MxGstPipeline *
mx_gst_pipeline_new(void)
{
  return g_object_new (MX_TYPE_GST_PIPELINE, 
                       "clip-to-allocation", TRUE,
                       NULL);
}

