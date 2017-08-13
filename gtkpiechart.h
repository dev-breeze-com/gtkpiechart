#ifndef __GTK_PIE_CHART_H__
#define __GTK_PIE_CHART_H__

#include <gdk/gdk.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/* Standart GObject macros */
#define GTK_TYPE_PIE_CHART (gtk_pie_chart_get_type())
#define GTK_PIE_CHART(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_PIE_CHART, GtkPieChart))
#define GTK_PIE_CHART_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_PIE_CHART, GtkPieChart))
#define GTK_IS_PIE_CHART(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_PIE_CHART))
#define GTK_IS_PIE_CHART_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_PIE_CHART))
#define GTK_PIE_CHART_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_PIE_CHART, GtkPieChart))

#if 0
#define GTK_PIE_CHART(obj) \
	GTK_CHECK_CAST(obj, gtk_pie_chart_get_type (), GtkPieChart)

#define GTK_PIE_CHART_CLASS(klass) \
	GTK_CHECK_CLASS_CAST(klass, gtk_pie_chart_get_type (), GtkPieChartClass)

#define GTK_IS_PIE_CHART(obj) \
	GTK_CHECK_TYPE(obj, gtk_pie_chart_get_type ())
#endif

typedef struct _GtkPieChart       GtkPieChart;
typedef struct _GtkPieChartClass  GtkPieChartClass;

struct _GtkPieChart {
	GtkWidget parent;
	GtkWidget widget;
	GdkPixmap* offscreen_pixmap;
	GList*     areas;
};

struct _GtkPieChartClass {
	GtkWidgetClass parent_class;
};

GType		gtk_pie_chart_get_type(void);
GtkWidget*	gtk_pie_chart_new(void);
void		gtk_pie_chart_clear(GtkPieChart* pie);
gboolean	gtk_pie_chart_add_slice(GtkPieChart* pie, GdkColor* fore, const char* legend, gdouble percentage);

G_END_DECLS

#endif /* __GTK_PIE_CHART_H__ */
