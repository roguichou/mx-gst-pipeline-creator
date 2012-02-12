#ifndef _MX_GST_PIPELINE_H_
#define _MX_GST_PIPELINE_H_

#include <mx/mx.h>

G_BEGIN_DECLS

#define MX_TYPE_GST_PIPELINE            (mx_gst_pipeline_get_type())

#define MX_GST_PIPELINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), \
  MX_TYPE_GST_PIPELINE, MxGstPipeline))

#define MX_IS_GST_PIPELINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
  MX_TYPE_GST_PIPELINE))

#define MX_GST_PIPELINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MX_TYPE_GST_PIPELINE, MxGstPipelineClass))

#define MX_IS_GST_PIPELINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MX_TYPE_GST_PIPELINE))

#define MX_GST_PIPELINE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),  \
  MX_TYPE_GST_PIPELINE, MxGstPipeline))

typedef struct _MxGstPipeline        MxGstPipeline;
typedef struct _MxGstPipelineClass   MxGstPipelineClass;
typedef struct _MxGstPipelinePrivate MxGstPipelinePrivate;

struct _MxGstPipeline
{
  ClutterGroup          parent_instance;
  MxGstPipelinePrivate *priv;
};

struct _MxGstPipelineClass
{
  ClutterGroupClass parent_class;
};

GType mx_gst_pipeline_get_type (void) G_GNUC_CONST;

MxGstPipeline *
mx_gst_pipeline_new(void);

G_END_DECLS

#endif //_MX_GST_PIPELINE_H_
