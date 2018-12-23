/*
 * my_operater.c
 *
 *  Created on: 2018年3月18日
 *      Author: tom
 */

#include "MyOperater.h"

typedef struct {
	GList *list;
	WebKitWebView *web_view;
	GtkLayout *parent_layout;
	GtkMenu *setting_menu;
	GtkHeaderBar *headbar;
	GtkBox *box;
	gdouble x, y;
	gboolean pressed;
} MyOperaterPrivate;

G_DEFINE_TYPE_WITH_CODE(MyOperater, my_operater, GTK_TYPE_BOX,
		G_ADD_PRIVATE(MyOperater));
void my_operater_setting_clicked(GtkButton *button, MyOperater *self);
gboolean
my_operater_motion_notify(GtkWidget *widget, GdkEvent *event, MyOperater *self);
gboolean
my_operater_headbar_press(GtkWidget *widget, GdkEvent *event, MyOperater *self);

void
my_operater_add_query(GtkMenuItem *menuitem, MyOperater *self);
void
my_operater_add_filter(GtkMenuItem *menuitem, MyOperater *self);
void
my_operater_add_javascript(GtkMenuItem *menuitem, MyOperater *self);
void
my_operater_add_download(GtkMenuItem *menuitem, MyOperater *self);
void my_operater_add_start(GtkMenuItem *menuitem, MyOperater *self);

void my_operater_add_web_loader(GtkMenuItem *menuitem, MyOperater *self);
void my_operater_re_name(GtkMenuItem *menuitem, MyOperater *self);
void
my_operater_del_operater(GtkMenuItem *menuitem, MyOperater *self);

static void my_operater_class_init(MyOperaterClass *klass) {
	gtk_widget_class_set_template_from_resource(klass,
			"/myoperater/MyOperater.glade");
	gtk_widget_class_bind_template_child_private(klass, MyOperater,
			setting_menu);
	gtk_widget_class_bind_template_child_private(klass, MyOperater, headbar);
	gtk_widget_class_bind_template_child_private(klass, MyOperater, box);
	gtk_widget_class_bind_template_callback(klass, my_operater_setting_clicked);
	gtk_widget_class_bind_template_callback(klass, my_operater_motion_notify);
	gtk_widget_class_bind_template_callback(klass, my_operater_headbar_press);

	gtk_widget_class_bind_template_callback(klass, my_operater_add_query);
	gtk_widget_class_bind_template_callback(klass, my_operater_add_filter);
	gtk_widget_class_bind_template_callback(klass, my_operater_add_javascript);
	gtk_widget_class_bind_template_callback(klass, my_operater_add_download);
	gtk_widget_class_bind_template_callback(klass, my_operater_add_web_loader);
	gtk_widget_class_bind_template_callback(klass, my_operater_add_start);
	gtk_widget_class_bind_template_callback(klass,my_operater_re_name);
	gtk_widget_class_bind_template_callback(klass, my_operater_del_operater);

}

static void my_operater_init(MyOperater *self) {
	gtk_widget_init_template(self);
	MyOperaterPrivate *priv = my_operater_get_instance_private(self);
	gtk_widget_add_events(priv->headbar,
			GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_MASK);
	priv->pressed = FALSE;
	priv->x = 0;
	priv->y = 0;
}

void my_operater_setting_clicked(GtkButton *button, MyOperater *self) {
	MyOperaterPrivate *priv = my_operater_get_instance_private(self);
	gtk_menu_popup_at_pointer(priv->setting_menu, NULL);
}
;

gboolean my_operater_motion_notify(GtkWidget *widget, GdkEvent *event,
		MyOperater *self) {
	MyOperaterPrivate *priv = my_operater_get_instance_private(self);
	if (priv->pressed) {
		GtkAllocation alloc;
		gint x, y;
		gtk_widget_get_allocation(self, &alloc);
		if (alloc.x >= 0 || event->button.x >= 0)
			x = event->button.x - priv->x;
		if (alloc.y >= 0 || event->button.y >= 0)
			y = event->button.y - priv->y;
		gtk_layout_move(priv->parent_layout, self, x + alloc.x, y + alloc.y);
		gtk_widget_queue_draw(priv->parent_layout);
		return TRUE;
	}
	return FALSE;
}
;
gboolean my_operater_headbar_press(GtkWidget *widget, GdkEvent *event,
		MyOperater *self) {
	MyOperaterPrivate *priv = my_operater_get_instance_private(self);
	if (event->button.button == GDK_BUTTON_PRIMARY
			&& event->button.type == GDK_BUTTON_PRESS) {
		priv->pressed = TRUE;
		priv->x = event->button.x;
		priv->y = event->button.y;
	} else {
		priv->pressed = FALSE;
	}
	return FALSE;
}
;

void my_operater_add_query(GtkMenuItem *menuitem, MyOperater *self) {
	MyQuery *query = my_query_new(NULL, "a", "href", NULL);
	gtk_box_pack_start(self, query, FALSE, FALSE, 0);
	gtk_widget_show_all(query);
}
;
void my_operater_add_filter(GtkMenuItem *menuitem, MyOperater *self) {
	MyFilter *filter = my_filter_new("[\\w]+://[^\\s^\\n^,^\\\"]+",
			"a=b\nc=123\n3.141516\nhttp://www.abc.com\nThis is a \"Word\"!!!");
	gtk_box_pack_start(self, filter, FALSE, FALSE, 0);
	gtk_widget_show_all(filter);
}
;
void my_operater_add_javascript(GtkMenuItem *menuitem, MyOperater *self) {
	MyJsCmd *js = my_js_cmd_new("JavaScript", "");
	gtk_box_pack_start(self, js, FALSE, FALSE, 0);
	gtk_widget_show_all(js);
}
;
void my_operater_add_download(GtkMenuItem *menuitem, MyOperater *self) {
	MyDl *dl = my_dl_new();
	gtk_box_pack_start(self, dl, FALSE, FALSE, 0);
	gtk_widget_show_all(dl);

}
;

void my_operater_add_start(GtkMenuItem *menuitem, MyOperater *self) {
	MyStart *start = my_start_new();
	gtk_box_pack_start(self, start, FALSE, FALSE, 0);
	gtk_widget_show_all(start);
}
;

void my_operater_add_web_loader(GtkMenuItem *menuitem, MyOperater *self) {
	MyLoader *loader = my_loader_new();
	gtk_box_pack_start(self, loader, FALSE, FALSE, 0);
	gtk_widget_show_all(loader);
}
;

void my_operater_re_name(GtkMenuItem *menuitem, MyOperater *self){
	MyOperaterPrivate *priv=my_operater_get_instance_private(self);
	GtkDialog *dialog = gtk_dialog_new_with_buttons("RENAME", main_window,
			GTK_DIALOG_MODAL, "Rename", GTK_RESPONSE_OK, "Cancle",
			GTK_RESPONSE_CANCEL, NULL);
	GtkEntry *entry = gtk_entry_new();
	gtk_entry_set_text(entry,gtk_header_bar_get_title(priv->headbar));
	gtk_container_add(gtk_dialog_get_content_area(dialog), entry);
	gtk_widget_show_all(dialog);
	if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
		gtk_header_bar_set_title(priv->headbar,gtk_entry_get_text(entry));
	}
	gtk_widget_destroy(entry);
	gtk_widget_destroy(dialog);
};

void my_operater_del_operater(GtkMenuItem *menuitem, MyOperater *self) {
	gtk_widget_destroy(self);
}
;

MyOperater *my_operater_new(gchar *title) {
	MyOperater *op = g_object_new(MY_TYPE_OPERATER, NULL);
	MyOperaterPrivate *priv = my_operater_get_instance_private(op);
	if (title != NULL)
		gtk_header_bar_set_title(priv->headbar, title);
	return op;
}
;

void my_operater_set_layout(MyOperater *operater, GtkLayout *layout) {
	MyOperaterPrivate *priv = my_operater_get_instance_private(operater);
	priv->parent_layout = layout;
}
;

void my_operater_set_title(MyOperater *operater,const gchar *str){
	MyOperaterPrivate *priv = my_operater_get_instance_private(operater);
	if(str!=NULL)gtk_header_bar_set_title(priv->headbar,str);
};
const gchar *my_operater_get_title(MyOperater *operater){
	MyOperaterPrivate *priv = my_operater_get_instance_private(operater);
	return gtk_header_bar_get_title(priv->headbar);

};
