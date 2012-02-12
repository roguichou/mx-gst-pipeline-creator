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

G_DEFINE_TYPE (MxGstPipeline, mx_gst_pipeline, MX_TYPE_GRAPH);


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

