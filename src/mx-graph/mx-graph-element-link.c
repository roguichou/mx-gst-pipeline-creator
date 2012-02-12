#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mx-graph-element-pad.h"
#include "mx-graph-element-link.h"
#include <math.h>
#include <string.h>

typedef struct
{
  gfloat x1,x2,y1,y2;
}Bbox;

/**
 * Private structure.
 */
struct _MxGraphElementLinkPrivate
{
  /** bounding box of the actor. */
  Bbox      bounding_box;
  /** coords {(x,y)*}. */
  gfloat   *coords;
  /** nb points in coords (=length_of(coords)/2). */
  gint      nb_pts;
  /** 3 points for the arrow at the end of the link. */
  gfloat    arrow[6]; 
  /** events captured on the stage. */
  gulong    capture_event_signal_id;
  /** True if actor is selected. */
  gboolean  is_selected;
  /** current seleceted point. */
  gint      selected_point;
  /** current selected segment. */
  gint      selected_segment;
  /** true if mouse button is pressed. */
  gboolean  button_press;
  /** half width/height of the selection rectangle (to show the
   * control points. */
  guint     selection_rectangle_half_sz;
};


G_DEFINE_TYPE (MxGraphElementLink, mx_graph_element_link, MX_TYPE_WIDGET)

/** 
 * see #ClutterActor::paint
 */
static void
mx_graph_element_link_paint(ClutterActor *actor)
{
  MxGraphElementLinkPrivate *priv = MX_GRAPH_ELEMENT_LINK(actor)->priv;

  CLUTTER_ACTOR_CLASS (mx_graph_element_link_parent_class)->paint (actor);

  cogl_push_matrix ();

  cogl_path_new();
  cogl_set_source_color4ub (0, 0, 0, 255);
  cogl_path_polyline(priv->coords, priv->nb_pts);
  cogl_path_stroke();

  cogl_path_new();
  cogl_path_polyline(priv->arrow, 3);
  cogl_path_stroke();

  if(priv->is_selected)
  {
    int i=0;
    for(i=0; i<priv->nb_pts; i++)
    {
  
      cogl_path_new();
      cogl_path_rectangle( 
          priv->coords[2*i]   - priv->selection_rectangle_half_sz, 
          priv->coords[2*i+1] - priv->selection_rectangle_half_sz,
          priv->coords[2*i]   + priv->selection_rectangle_half_sz,
          priv->coords[2*i+1] + priv->selection_rectangle_half_sz);
      cogl_path_fill();
    }
  }

  cogl_pop_matrix ();
}

static void
mx_graph_element_link_pick (ClutterActor       *actor,
                            const ClutterColor *color)
{
  MxGraphElementLinkPrivate *priv = MX_GRAPH_ELEMENT_LINK(actor)->priv;

  cogl_push_matrix ();

  cogl_set_source_color4ub (color->red, color->green, 
      color->blue, color->alpha);

  cogl_path_new();
  cogl_path_polyline(priv->coords, priv->nb_pts);
  cogl_path_stroke();

  cogl_path_new();
  cogl_path_polyline(priv->arrow, 3);
  cogl_path_stroke();

  if(priv->is_selected)
  {
    int i=0;
    for(i=0; i<priv->nb_pts; i++)
    {
  
      cogl_path_new();
      cogl_path_rectangle( 
          priv->coords[2*i]   - priv->selection_rectangle_half_sz, 
          priv->coords[2*i+1] - priv->selection_rectangle_half_sz,
          priv->coords[2*i]   + priv->selection_rectangle_half_sz,
          priv->coords[2*i+1] + priv->selection_rectangle_half_sz);
      cogl_path_fill();
    }
  }

  cogl_pop_matrix ();

}

/** 
 * see #GObject::dispose
 */
static void
mx_graph_element_link_dispose(GObject *object)
{
  MxGraphElementLinkPrivate *priv = MX_GRAPH_ELEMENT_LINK(object)->priv;
  if(NULL != priv->coords)
  {
    g_free(priv->coords);
  }
  if(0 != priv->capture_event_signal_id)
  {
    g_signal_handler_disconnect(clutter_stage_get_default(),
        priv->capture_event_signal_id);
    priv->capture_event_signal_id = 0;
  }
  G_OBJECT_CLASS (mx_graph_element_link_parent_class)->dispose (object);
}

static void
mx_graph_element_link_class_init (MxGraphElementLinkClass *klass)
{
  g_type_class_add_private (klass, sizeof (MxGraphElementLinkPrivate));
  ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
  GObjectClass *g_class= G_OBJECT_CLASS(klass);
  actor_class->paint = mx_graph_element_link_paint;
  actor_class->pick = mx_graph_element_link_pick;
  
  g_class->dispose   = mx_graph_element_link_dispose;
}

/**
 * mx_graph_element_link_set_point:
 *
 * sets the position of the point from its index, coords must be
 * absolute (relative to the stage).
 *
 * @actor -in- a #MxGraphElementLink
 * @idx   -in- idx of the point (idx<priv->nb_pts)
 * @x     -in- new xcoord (relative to the stage)
 * @y     -in- new ycoord (relative to the stage)
 */
static void
mx_graph_element_link_set_point (MxGraphElementLink *actor, 
                                 guint               idx,
                                 gfloat              x,
                                 gfloat              y)
{
  MxGraphElementLinkPrivate *priv = actor->priv;
  g_return_if_fail(idx<priv->nb_pts);

  gfloat xpos, ypos;
  clutter_actor_get_transformed_position(CLUTTER_ACTOR(actor), &xpos, &ypos);

  gint idx_pt = 2*idx;
  priv->coords[idx_pt  ] = x-xpos;
  priv->coords[idx_pt+1] = y-ypos;

  if((gint)idx > priv->nb_pts-3) // last point or the one just before
  {
    // We have to compute the arrow coords.
    // The arrow is a path containing 3 points.
    // The middle one is the last point of our polyline
    // and the others are computed so that the arrow follows the
    // orientation of the last segment.
    gfloat abx = priv->coords[2*(priv->nb_pts-1)] - 
      priv->coords[2*(priv->nb_pts-2)];
    gfloat aby = priv->coords[2*(priv->nb_pts-1)+1] - 
      priv->coords[2*(priv->nb_pts-2)+1];
    gfloat dab = sqrt(abx*abx+aby*aby);
    abx /= dab;
    aby /= dab;

    gfloat nabx = -aby;
    gfloat naby = abx;

    gfloat acx = priv->coords[2*(priv->nb_pts-2)] + abx*(dab-10.);
    gfloat acy = priv->coords[2*(priv->nb_pts-2)+1] + aby*(dab-10.);
    priv->arrow[0] = acx+nabx*5.;
    priv->arrow[1] = acy+naby*5.;

    priv->arrow[2] = priv->coords[2*(priv->nb_pts-1)];
    priv->arrow[3] = priv->coords[2*(priv->nb_pts-1)+1];
    
    priv->arrow[4] = acx-nabx*5.;
    priv->arrow[5] = acy-naby*5.;
  }

  //compute BB of the actor
  priv->bounding_box.x1 = priv->coords[0];
  priv->bounding_box.y1 = priv->coords[1];
  priv->bounding_box.x2 = priv->coords[0];
  priv->bounding_box.y2 = priv->coords[1];

  int i=0;
  for (i=1; i<priv->nb_pts; i++)
  {
    priv->bounding_box.x1 = MIN(priv->bounding_box.x1, priv->coords[2*i]);
    priv->bounding_box.y1 = MIN(priv->bounding_box.y1, priv->coords[2*i+1]);
    
    priv->bounding_box.x2 = MAX(priv->bounding_box.x2, priv->coords[2*i]);
    priv->bounding_box.y2 = MAX(priv->bounding_box.y2, priv->coords[2*i+1]);
  }
  for (i=0; i<3; i++)
  {
    priv->bounding_box.x1 = MIN(priv->bounding_box.x1, priv->arrow[2*i]);
    priv->bounding_box.y1 = MIN(priv->bounding_box.y1, priv->arrow[2*i+1]);
    
    priv->bounding_box.x2 = MAX(priv->bounding_box.x2, priv->arrow[2*i]);
    priv->bounding_box.y2 = MAX(priv->bounding_box.y2, priv->arrow[2*i+1]);
  }

  //due to the drawing if in selection.
  priv->bounding_box.x1 -= priv->selection_rectangle_half_sz;
  priv->bounding_box.y1 -= priv->selection_rectangle_half_sz;
    
  priv->bounding_box.x2 += priv->selection_rectangle_half_sz;
  priv->bounding_box.y2 += priv->selection_rectangle_half_sz;

  //Move the actor so that the upper-left corner of bounding box is 0,0.
  //And compute the paths coords to draw at the right position.
  for(i=0; i<priv->nb_pts; i++)
  {
    priv->coords[2*i  ] -= priv->bounding_box.x1;
    priv->coords[2*i+1] -= priv->bounding_box.y1;
  }
  for(i=0; i<3; i++)
  {
    priv->arrow[2*i  ] -= priv->bounding_box.x1;
    priv->arrow[2*i+1] -= priv->bounding_box.y1;
  }

  gfloat oldx, oldy;
  clutter_actor_get_position(CLUTTER_ACTOR(actor), &oldx, &oldy);
  clutter_actor_set_position(CLUTTER_ACTOR(actor), 
      oldx + priv->bounding_box.x1, oldy + priv->bounding_box.y1);

  priv->bounding_box.x2 -= priv->bounding_box.x1;
  priv->bounding_box.y2 -= priv->bounding_box.y1;
  priv->bounding_box.x1 = 0;
  priv->bounding_box.y1 = 0;
  
  //set the new size of the actor ("pack")
  clutter_actor_set_size(CLUTTER_ACTOR(actor), 
      priv->bounding_box.x2, priv->bounding_box.y2);
}

/**
 * mx_graph_element_link_is_point_on_polyline:
 *
 * Checks if a point is on the drawn polyline or is one of the
 * control points of the polyline.
 *
 * @actor   -in-  a #MxGraphElementLink
 * @x       -in-  x coord (relative to the upper left corner of the actor)
 * @y       -in-  y coord (relative to the upper left corner of the actor)
 * @idx_pt  -out- index of the point if the coords correspond to a
 *                control point of the polyline or -1 if not found
 * @idx_seg -out- index of the segment on which the point (x,y) is
 *                located or -1 if not found
 *
 * @return TRUE if either idx_pt or idx_seg != -1
 */
gboolean
mx_graph_element_link_is_point_on_polyline(MxGraphElementLink *actor,
                                           gfloat              x,
                                           gfloat              y,
                                           gint               *idx_pt, 
                                           gint               *idx_seg)
{
  MxGraphElementLinkPrivate *priv = actor->priv;
  if(idx_seg) *idx_seg = -1;
  if(idx_pt)  *idx_pt = -1;
  //step 1 :: check if it's a point (tolerancy: 5px).
  int i=0;
  for (i=0; i<priv->nb_pts; i++)
  {
    gfloat xpt = priv->coords[2*i];
    gfloat ypt = priv->coords[2*i+1];
    if(fabs(xpt-x)<5 && fabs(ypt-y)<5)
    {
      if(idx_pt) *idx_pt = i;
      return TRUE;
    }
  }
  
  //step 2 :: check if it's inside a segment. 
  //   the idea is to use the scalar product and vect product.
  //   (tolerancy of 2 pixels)
  for (i=0; i<priv->nb_pts-1; i++)
  {
    gfloat abx = priv->coords[2*(i+1)] - priv->coords[2*i];
    gfloat aby = priv->coords[2*(i+1)+1] - priv->coords[2*i+1];
    gfloat dab = sqrt(abx*abx+aby*aby);
    abx /= dab;
    aby /= dab;

    gfloat nabx = -aby;
    gfloat naby = abx;
    
    gfloat apx = x-priv->coords[2*i];
    gfloat apy = y-priv->coords[2*i+1];

    //proj AP on AB
    gfloat proj = (apx*abx+apy*aby);

    gfloat nproj = (apx*nabx+apy*naby);

    //dist P to AB = nAB.AP
    if( proj >= 0. && proj <= dab && fabs(nproj)<3)
    {
      if(idx_seg) *idx_seg = i;
      return TRUE;
    }
  }
  return FALSE;
}

static gboolean
mx_graph_element_link_on_stage_capture(ClutterActor       *stage,
                                       ClutterEvent       *event,
                                       MxGraphElementLink *actor)
{
  MxGraphElementLinkPrivate *priv = actor->priv;

  if(priv->is_selected && CLUTTER_KEY_RELEASE == event->type &&
      (CLUTTER_KEY_Delete == ((ClutterKeyEvent *)event)->keyval || 
       CLUTTER_KEY_KP_Delete == ((ClutterKeyEvent *)event)->keyval))
  {
    //we were asked to delete ourselves, so remove from parent.
    clutter_container_remove_actor(
        CLUTTER_CONTAINER(clutter_actor_get_parent(CLUTTER_ACTOR(actor))),
        CLUTTER_ACTOR(actor));
    return FALSE;
  }

  gfloat x,y, xparent, yparent;
  clutter_event_get_coords(event, &x, &y);
  clutter_actor_get_transformed_position  ( CLUTTER_ACTOR(actor),
      &xparent, &yparent);
  x -= xparent;
  y -= yparent;


  if(CLUTTER_BUTTON_PRESS == event->type)
  {
    int idx_pt = -1;
    int idx_seg = -1;
    if( x >= priv->bounding_box.x1 && x <= priv->bounding_box.x2 &&
        y >= priv->bounding_box.y1 && y <= priv->bounding_box.y2 && 
        mx_graph_element_link_is_point_on_polyline(
          actor, x, y, &idx_pt, &idx_seg))
    {
      //select
      priv->is_selected      = TRUE;
      priv->selected_point   = idx_pt;
      priv->selected_segment = idx_seg;
      priv->button_press     = TRUE;
    }
    else if(priv->is_selected)
    {
      //unselect
      priv->is_selected      = FALSE;
      priv->selected_point   = -1;
      priv->selected_segment = -1;
      priv->button_press     = FALSE;
    }
    //redraw to show the control points
    clutter_actor_queue_redraw(CLUTTER_ACTOR(actor));
    return FALSE;
  }
  else if(!priv->is_selected)
  {
    return FALSE;
  }
  
  //priv->is_selected = TRUE!!!

  if(CLUTTER_BUTTON_RELEASE == event->type)
  {
      priv->selected_point   = -1;
      priv->selected_segment = -1;
      priv->button_press     = FALSE;
  }

  if(CLUTTER_MOTION == event->type && priv->button_press)
  {
    if(-1 != priv->selected_point && 0 != priv->selected_point
      && priv->nb_pts-1 != priv->selected_point)
    {
      //we are moving a point of the line else than the first and
      //the last one.
      mx_graph_element_link_set_point (actor, priv->selected_point, 
          x+xparent, y+yparent);
    }
    else if(-1 != priv->selected_segment)
    {
      //someoe is trying to add a point insidea segment
      gfloat *coords = g_malloc(2*(priv->nb_pts+1)*sizeof(gfloat));
      int idx_nv_pt = 2*(priv->selected_segment+1);

      g_memmove(coords, priv->coords, idx_nv_pt*sizeof(gfloat));
      priv->selected_point = priv->selected_segment+1;

      g_memmove(&coords[idx_nv_pt+2], &priv->coords[idx_nv_pt], 
          (2*priv->nb_pts-idx_nv_pt)*sizeof(gfloat));
      priv->nb_pts++;
      g_free(priv->coords);
      priv->coords = coords;
      
      mx_graph_element_link_set_point (actor, priv->selected_point,
          x+xparent, y+yparent);
      priv->selected_segment = -1;
    }
  }

  return FALSE;
}


static void
mx_graph_element_link_init (MxGraphElementLink *actor)
{
  MxGraphElementLinkPrivate *priv = actor->priv = 
    G_TYPE_INSTANCE_GET_PRIVATE(actor, MX_TYPE_GRAPH_ELEMENT_LINK,
        MxGraphElementLinkPrivate);
  priv->capture_event_signal_id = g_signal_connect_after (
      clutter_stage_get_default(),
      "captured-event", G_CALLBACK (mx_graph_element_link_on_stage_capture),
      actor);
  priv->is_selected = FALSE;
  priv->selection_rectangle_half_sz = 2; 
}

/**
 * mx_graph_element_link_new:
 *
 * Creates a new link with an origin
 *
 * @xorig -in-: x coord of first point(relative to the link parent).
 * @yorig -in-: y coord of first point(relative to the link parent).
 *
 * @return a new #MxGraphElementLink
 */
MxGraphElementLink *
mx_graph_element_link_new(ClutterContainer *cont,
                          gfloat            xorig,
                          gfloat            yorig)
{
  MxGraphElementLink *res = MX_GRAPH_ELEMENT_LINK(
      g_object_new(MX_TYPE_GRAPH_ELEMENT_LINK, NULL));

  clutter_container_add_actor(cont, CLUTTER_ACTOR(res));

  MxGraphElementLinkPrivate *priv = res->priv;
  
  //Initialization : a degenerated line with the 2 same points.
  priv->coords = g_malloc(4*sizeof(gfloat));
  priv->coords[0] = priv->coords[2] = 0;
  priv->coords[1] = priv->coords[3] = 0;
  priv->nb_pts = 2;
  
  priv->bounding_box.x1 = 0; priv->bounding_box.y1 = 0;
  priv->bounding_box.x2 = 0; priv->bounding_box.y2 = 0;

  clutter_actor_set_position(CLUTTER_ACTOR(res),xorig, yorig);
  clutter_actor_set_size(CLUTTER_ACTOR(res),1,1);
  
  return res;
}

/**
 * mx_graph_element_link_set_dest_point:
 *
 * @actor -in- a #MxGraphElementLink
 *
 * @xdest -in- absolute new x coord of the link's last point 
 * @ydest -in- absolute new y coord of the link's last point 
 */
void
mx_graph_element_link_set_dest_point (MxGraphElementLink *actor, 
                                      gfloat              xdest,
                                      gfloat              ydest)
{
  MxGraphElementLinkPrivate *priv = actor->priv;

  mx_graph_element_link_set_point (actor, priv->nb_pts-1, xdest, ydest);
 
}

/**
 * mx_graph_element_link_set_orig_point:
 *
 * @actor -in- a #MxGraphElementLink
 *
 * @xdest -in- abolute new x coord of the link's first point 
 * @ydest -in- abolute new y coord of the link's first point 
 */
void
mx_graph_element_link_set_orig_point (MxGraphElementLink *actor, 
                                      gfloat              xori,
                                      gfloat              yori)
{
  mx_graph_element_link_set_point (actor, 0, xori, yori);
}

