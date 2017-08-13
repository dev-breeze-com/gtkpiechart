#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gtkpiechart.h"

#define MIN_WIDTH   120
#define MIN_HEIGHT  80

typedef struct _AreaProperty AreaProperty;

struct _AreaProperty {
	GdkColor    fore_color;
	gdouble     percentage;
	const char* legend;
};

/* Internal API */
static void gtk_pie_chart_class_init(GtkPieChartClass* klass);
static void gtk_pie_chart_init(GtkPieChart*);

static void gtk_pie_chart_destroy(GtkObject*);
static void gtk_pie_chart_realize(GtkWidget*);
static gboolean gtk_pie_chart_expose(GtkWidget*, GdkEventExpose*);

static void gtk_pie_chart_size_request(GtkWidget*, GtkRequisition*);
static void gtk_pie_chart_size_allocate(GtkWidget*, GtkAllocation*);
static void gtk_pie_chart_make_pixmap(GtkPieChart* pie);
static void gtk_pie_chart_paint(GtkPieChart* pie);

/*
static void gtk_pie_chart_set_property(GObject*, guint prop_id,
	const GValue *value, GParamSpec*);
static void gtk_pie_chart_get_property(GObject*, guint prop_id,
	GValue*, GParamSpec*);
*/

/* Define type */
G_DEFINE_TYPE(GtkPieChart, gtk_pie_chart, GTK_TYPE_WIDGET)

//static GtkWidgetClass* parent_class = NULL;

#if 0
GType
gtk_pie_chart_get_type(void)
{
	static guint pie_chart_type = 0;

	if (!pie_chart_type) {
		GtkTypeInfo pie_chart_info = {
			"GtkPieChart",
			sizeof(GtkPieChart),
			sizeof(GtkPieChartClass),
			(GtkClassInitFunc) gtk_pie_chart_class_init,
			(GtkObjectInitFunc) gtk_pie_chart_init,
			0L, 0L,
			(GtkClassInitFunc) NULL
			//    (GtkArgSetFunc) NULL,
			//  (GtkArgGetFunc) NULL,
		};
		pie_chart_type = gtk_type_unique(gtk_widget_get_type(), &pie_chart_info);
	}
	//fprintf(stderr, "%d %0x\n", pie_chart_type, pie_chart_type );
	return pie_chart_type;
}
#endif

static void
gtk_pie_chart_class_init(GtkPieChartClass* class)
{
	GtkObjectClass* g_class;
	GtkWidgetClass* w_class;

	g_class = (GtkObjectClass*) class;
	w_class = (GtkWidgetClass*) class;
	//parent_class = gtk_type_class(gtk_widget_get_type());

	g_class->destroy = gtk_pie_chart_destroy;
	w_class->realize = gtk_pie_chart_realize;
	w_class->expose_event = gtk_pie_chart_expose;

	w_class->size_request = gtk_pie_chart_size_request;
	w_class->size_allocate = gtk_pie_chart_size_allocate;
}

static void
gtk_pie_chart_init(GtkPieChart* pie)
{
	gtk_widget_set_has_window(GTK_WIDGET(pie), TRUE);
	//GTK_WIDGET_SET_FLAGS(pie, GTK_HAS_DEFAULT);
	//GTK_WIDGET_SET_FLAGS(pie, GTK_BASIC);

	GTK_WIDGET(pie)->requisition.width = MIN_WIDTH;
	GTK_WIDGET(pie)->requisition.height = MIN_HEIGHT;

	pie->offscreen_pixmap = NULL;
	pie->areas = NULL;
}

GtkWidget*
gtk_pie_chart_new(void)
{
	return(g_object_new(GTK_TYPE_PIE_CHART, NULL));
	/*
	guint ptr = gtk_pie_chart_get_type();
	//return GTK_WIDGET(gtk_type_new(
	//fprintf(stderr, "gtk_pie_chart_new %0x\n",(uint)ptr );
	return GTK_WIDGET(gtk_type_new(ptr));
	*/
}

static void
gtk_pie_chart_destroy(GtkObject* object)
{
	GtkPieChart* pie;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_PIE_CHART(object));

	pie = GTK_PIE_CHART(object);

	gtk_pie_chart_clear(pie);

	if (pie->offscreen_pixmap)
	{ gdk_pixmap_unref(pie->offscreen_pixmap); }

	/*
	if (GTK_OBJECT_CLASS(parent_class)->destroy)
	{ (*GTK_OBJECT_CLASS(parent_class)->destroy) (object); }
	*/
}

gboolean
gtk_pie_chart_add_slice(GtkPieChart* pie, GdkColor* fore, const char* legend, gdouble percentage)
{
	AreaProperty* area;
	GList*      list;
	gdouble     totalp;
	g_return_val_if_fail(pie != NULL, FALSE);
	g_return_val_if_fail(fore != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_PIE_CHART(pie), FALSE);

	if (percentage <= 0.0)
	{ return FALSE; }

	if (percentage > 1.0)
	{ percentage = 1.0; }

	/*
	 *  'percentage' relates to the area of this slice compared to the
	 *  total area. The data we store, however, is the cumulative percentage.
	 *  If we have already reached the total area, we simply refuse to add it.
	 *  If we exceed the total area, we chop it instead to avoid getting bitten
	 *  by rounding errors (suppose the total percentage is 1.0 + 1e-12).
	 */
	totalp = 0;
	list = g_list_last(pie->areas);

	if (list) {
		area = (AreaProperty*) list->data;
		totalp = area->percentage;
	}

	if (totalp >= 1.0)
	{ return FALSE; }

	totalp += percentage;

	if (totalp > 1.0)
	{ totalp = 1.0; }

	area = g_new(AreaProperty, 1);
	area->percentage = totalp;
	area->fore_color = *fore;
	area->legend = (const char*) strdup( legend );
	pie->areas = g_list_append(pie->areas, area);

	gtk_widget_queue_draw(GTK_WIDGET(pie));
	return TRUE;
}

static void
gtk_pie_chart_realize(GtkWidget* widget)
{
	GtkPieChart* pie;
	GdkWindowAttr attrs;
	gint        attrs_mask;

	//fprintf(stderr, "gtk_pie_chart_realize ...\n");
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_PIE_CHART(widget));
	//fprintf(stderr, "gtk_pie_chart_realize 2 ...\n");

	pie = GTK_PIE_CHART(widget);
	gtk_widget_set_realized(widget, TRUE);

	//GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attrs.window_type = GDK_WINDOW_CHILD;
	attrs.x = widget->allocation.x;
	attrs.y = widget->allocation.y;
	attrs.width = widget->allocation.width;
	attrs.height = widget->allocation.height;
	attrs.wclass = GDK_INPUT_OUTPUT;
	attrs.visual = gtk_widget_get_visual(widget);
	attrs.colormap = gtk_widget_get_colormap(widget);
	attrs.event_mask = gtk_widget_get_events(widget);
	attrs.event_mask |= GDK_EXPOSURE_MASK;
	attrs_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new(
		gtk_widget_get_parent_window(widget), &attrs, attrs_mask
	);

	gdk_window_set_user_data(widget->window, pie);
	//gtk_widget_set_window(widget, widget->window);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);

	gtk_pie_chart_make_pixmap(pie);
}

static void
gtk_pie_chart_size_allocate(GtkWidget* widget, GtkAllocation* allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_PIE_CHART(widget));
	g_return_if_fail(allocation != NULL);

	gtk_widget_set_allocation(widget, allocation);

	if (gtk_widget_get_realized(widget)) {
		gdk_window_move_resize(widget->window,
			allocation->x, allocation->y,
			allocation->width, allocation->height);
		gtk_pie_chart_make_pixmap(GTK_PIE_CHART(widget));
	}
}

static void
gtk_pie_chart_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	requisition->width  = MIN_WIDTH;
	requisition->height = MIN_HEIGHT;
}

static gboolean
gtk_pie_chart_expose(GtkWidget* widget, GdkEventExpose* event)
{
	GtkPieChart* pie;
	//fprintf(stderr, "gtk_pie_chart_expose ...\n");
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_PIE_CHART(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);
	//fprintf(stderr, "gtk_pie_chart_expose 2 ...\n");

	if (GTK_WIDGET_DRAWABLE(widget)) {
		pie = GTK_PIE_CHART(widget);
		gtk_pie_chart_make_pixmap(pie);

	//fprintf(stderr, "gtk_pie_chart_expose 3 ...\n");

		gdk_draw_pixmap(widget->window,
			widget->style->bg_gc[GTK_STATE_NORMAL],
			pie->offscreen_pixmap,
			0, 0, 0, 0,
			widget->allocation.width,
			widget->allocation.height);

	}
	return TRUE;
	//return FALSE;
}

static void
gtk_pie_chart_make_pixmap(GtkPieChart* pie)
{
	GtkWidget*  widget = GTK_WIDGET(pie);
	g_return_if_fail(pie != NULL);
	g_return_if_fail(GTK_IS_PIE_CHART(pie));

	//fprintf(stderr, "gtk_pie_chart_make_pixmap ...\n");

	//if (GTK_WIDGET_REALIZED(pie))
	if (gtk_widget_get_realized(widget)) {

		if (pie->offscreen_pixmap) {
			gdk_pixmap_unref(pie->offscreen_pixmap);
		}

		pie->offscreen_pixmap = gdk_pixmap_new(
			widget->window,
			widget->allocation.width,
			widget->allocation.height,
			(-1)
		);

//fprintf(stderr, "gtk_pie_chart_make_pixmap 2 %0lu ...\n", (ulong)pie->offscreen_pixmap);
fflush(stderr);

		gtk_pie_chart_paint(pie);
	}
}

void
gtk_pie_chart_clear(GtkPieChart* pie)
{
	g_return_if_fail(pie != NULL);
	g_return_if_fail(GTK_IS_PIE_CHART(pie));

	if (pie->areas) {
		AreaProperty* area;
		GList* list = pie->areas;

		for (; list; list = list->next) {
			area = (AreaProperty*) list->data;
			free((char*)area->legend);
			g_free(list->data);
		}
		g_list_free(pie->areas);
	}
	pie->areas = NULL;
}

/*
 *  Time for some easy to prove facts about ellipses:
 *
 *  An ellipse described by the equation (x/a)^2 + (y/b)^2 = 1,
 *  has width 2*a and height 2*b. Its area A is pi*a*b and the point P
 *  with coordinates (a*cos(2*pi*p), b*sin(2*pi*p)) where 0 <= p < 1,
 *  lies on the ellipse and carves out a sector P -> (0, 0) -> (a, 0)
 *  with an area of p*A. The angle alfa between P and the X-axis
 *  satisfies (width/height)*tan(alfa) = tan(2*pi*p). Referring to the
 *  man page of XDrawArc, the angle this function expects is not alfa,
 *  but an skewed angle 'corrected' by means of the above formula.
 *
 *  To make a long story short: if we pass XDrawArc the angle 64*360*p,
 *  the visual effect will be such that our desired area is shown on screen.
 *  We still need the angle alfa for drawing the dividing lines ourselves.
 *
 *  If only XDrawArc could draw a contour of the entire sector instead of
 *  just the arc... Another possibility is to do the ellipse drawing and
 *  filling ourselves. This would allow some extra visual effects.
 */

static void
gtk_pie_chart_paint(GtkPieChart* pie)
{
	GtkWidget* widget = 0L;
	AreaProperty* area = 0L;
	GList* list = 0L;
	GdkGC* gc = 0L;
	gint ang1, ang2, ang3;
	gint legend_offset;
	gint x = 0;
	gint y = 0;
	gint w = 0;
	gint h = 0;
	gint xc, yc;
	gint offs;

	g_return_if_fail(pie != NULL);
	g_return_if_fail(GTK_IS_PIE_CHART(pie));

	if (!pie->offscreen_pixmap) { return; }

	widget = GTK_WIDGET(pie);

	w = widget->allocation.width - 1;
	h = widget->allocation.height - 1;

	legend_offset = w * 0.20;
	legend_offset = legend_offset < 100 ? 100 : legend_offset;
	legend_offset = legend_offset > 150 ? 150 : legend_offset;

	gc = gdk_gc_new(widget->window);
	gdk_gc_copy(gc, widget->style->fg_gc[GTK_STATE_NORMAL]);

	/* Clear pixmap */
	gdk_draw_rectangle(
		pie->offscreen_pixmap, gc, TRUE, 0, 0, w + 1, h + 1
	);

	gdk_draw_rectangle(
		pie->offscreen_pixmap,
		widget->style->fg_gc[GTK_STATE_NORMAL],
		TRUE, 0, 0, w + 1, h + 1
	);

	gdk_draw_rectangle(
		pie->offscreen_pixmap,
		widget->style->fg_gc[GTK_STATE_INSENSITIVE],
		TRUE, 0, 0, w + 1, h + 1
	);

	gdk_draw_rectangle(
		pie->offscreen_pixmap,
		widget->style->bg_gc[GTK_STATE_NORMAL],
		TRUE, 0, 0, w + 1, h + 1
	);

	if (!pie->areas) { return; }

	w -= legend_offset;
	/* Calculate the visible elevation */
	offs = 0;

	if (w > h) {
		offs = sqrt(w * w - h * h) / 8;

		if (offs < 8)
		{ offs = 2; }

		if (offs > h)
		{ offs = h - 2; }
	}

	h -= offs;
	/* Draw the bottom ellipse and its contour */
	gdk_draw_arc(pie->offscreen_pixmap,
		 widget->style->fg_gc[GTK_STATE_INSENSITIVE], TRUE,
		 0, offs, w, h, 0, 360 * 64);
	gdk_draw_arc(pie->offscreen_pixmap,
		 widget->style->fg_gc[GTK_STATE_NORMAL], FALSE,
		 0, offs, w, h, 0, 360 * 64);
	gdk_draw_rectangle(pie->offscreen_pixmap,
	   widget->style->fg_gc[GTK_STATE_INSENSITIVE], TRUE,
	   0, h / 2, w, offs);
	/* Draw the slices */
	ang2 = 0;

	for (list = pie->areas; list; list = list->next) {
		area = (AreaProperty*) list->data;
		ang1 = ang2;
		ang2 = (gint) - (64 * 360 * area->percentage);
		ang3 = ang2 - ang1;

		if (ang1 < -180 * 64)
		{ ang1 += 360 * 64; }

		gdk_gc_set_foreground(gc, &area->fore_color);
		gdk_draw_arc(pie->offscreen_pixmap, gc, TRUE,
		             0, 0, w, h, ang1 - 180 * 64, ang3);
	}

	/* Draw the contour of the top ellipse and connect it to the bottom one */
	xc = w / 2;
	yc = h / 2;
	gdk_gc_copy(gc, widget->style->fg_gc[GTK_STATE_NORMAL]);
	gdk_draw_arc(pie->offscreen_pixmap, gc, FALSE, 0, 0, w, h, 0, 360 * 64);
	gdk_draw_line(pie->offscreen_pixmap, gc, 0, yc, 0, yc + offs);
	gdk_draw_line(pie->offscreen_pixmap, gc, w, yc, w, yc + offs);

	/*
	 *  Draw the dividing lines and the little vertical lines.
	 *  Note: don't change the rounding of x and y. This is the only
	 *  combination that matches best what XDrawArc does.
	 */
	for (list = pie->areas; list; list = list->next) {
		area = (AreaProperty*) list->data;
		x = w * cos(M_PI_2 * area->percentage) / 2 - 0.5;
		x = w * cos(M_PI_2 * area->percentage) / 2 - 0.5;
		//y = h * sin(PI2 * area->percentage) / 2 - 0.5;
		//y = h * sin(PI2 * area->percentage) / 2 - 0.5;
		gdk_draw_line(pie->offscreen_pixmap, gc, xc, yc, xc - x, yc - y);

		if (y < 0)
			gdk_draw_line(pie->offscreen_pixmap, gc,
			              xc - x, yc - y + 1,
			              xc - x, yc - y + offs + 1);
	}

	y = 5;
	x = w + 10;
	GdkFont* font =
	    gdk_font_load( "-*-arial-bold-r-normal--*-90-*-*-*-*-iso8859-1" );
	gdk_gc_copy(gc, widget->style->bg_gc[GTK_STATE_NORMAL]);

	for (list = pie->areas; list; list = list->next) {
		area = (AreaProperty*) list->data;
		gdk_gc_set_foreground(gc, &area->fore_color);
		gdk_draw_rectangle(pie->offscreen_pixmap, gc, TRUE,
		                   x, y, 20, 20 );
		gdk_gc_set_foreground(gc, &area->fore_color);
		gdk_draw_rectangle(pie->offscreen_pixmap, gc, TRUE,
		                   x, y, 20, 20 );
		gdk_draw_text(pie->offscreen_pixmap, font,
		              widget->style->fg_gc[GTK_STATE_NORMAL],
		              x + 25, y + 20, area->legend, strlen(area->legend) );
		y += 25;
	}
	gdk_gc_destroy(gc);
}
