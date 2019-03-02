/*
 * MyMainUi.c
 *
 *  Created on: 2018年5月4日
 *      Author: tom
 */

#include "MyMainUi.h"

enum {
	MAIN_UI_PROP_STATUS = 1
};

typedef struct {
	GtkLayout *layout;
	gdouble mouse_x, mouse_y;
	GtkWidget *select_dest, *select_src;
	GtkMenu *menu;
	GtkStatusbar *status_bar;
	GtkButton *run;
	MyDownload *download;
} MyMainUiPrivate;

G_DEFINE_TYPE_WITH_CODE(MyMainUi, my_main_ui, GTK_TYPE_WINDOW,
		G_ADD_PRIVATE(MyMainUi));

void my_main_ui_set_property(MyMainUi *object, guint property_id,
		const GValue *value, GParamSpec *pspec) {

	MyMainUiPrivate *priv = my_main_ui_get_instance_private(object);
	switch (property_id) {
	case MAIN_UI_PROP_STATUS:
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}
;
void my_main_ui_get_property(MyMainUi *object, guint property_id, GValue *value,
		GParamSpec *pspec) {

	switch (property_id) {
	case MAIN_UI_PROP_STATUS:
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}
;

void my_main_ui_add_operater(GtkToolButton *button, MyMainUi *self);
void my_main_ui_run(GtkToolButton *button, MyMainUi *self);
void my_main_ui_stop(GtkToolButton *button, MyMainUi *self);
void my_main_ui_menu_new(GtkMenuItem *item, MyMainUi *self);
void my_main_ui_menu_open(GtkMenuItem *item, MyMainUi *self);
void my_main_ui_menu_save(GtkMenuItem *item, MyMainUi *self);
void my_main_ui_menu_download(GtkMenuItem *item, MyMainUi *self);
void web_context_download_started(WebKitWebContext *context,
		WebKitDownload *download, MyMainUi *self);
gboolean
my_main_ui_layout_draw_cb(GtkWidget *widget, cairo_t *cr, MyMainUi *self);

gboolean layout_motion_notify_event(GtkWidget *widget, GdkEventMotion *event,
		MyMainUi *self) {
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	priv->mouse_x = event->x;
	priv->mouse_y = event->y;
	gtk_widget_queue_draw(widget);
	return FALSE;
}
;

gboolean my_main_ui_status_bar_notify(MyMainUi *self) {
	gboolean res = G_SOURCE_CONTINUE;
	gchar status[50];
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	GList *l = task_list;
	if (runing_count == 0 && msg_count == 0) {
		complete_check--;
	} else {
		complete_check = 8;
	}
	if (runing_count == 0 && msg_count == 0 && complete_check == 0) {
		MAIN_START = FALSE;
		g_sprintf(status, "Ready...");
		gtk_widget_set_sensitive(priv->run, TRUE);
		res = G_SOURCE_REMOVE;
		while (l != NULL) {
			my_task_run_finish(l->data);
			l = l->next;
		}
		WebKitWebContext *ctx = webkit_web_context_get_default();
		webkit_web_context_clear_cache(ctx);
	} else {
		g_sprintf(status, "%s\t| %u Runing %u Message ",
				MAIN_START ? "START" : "STOP ", runing_count, msg_count);
	}
	gtk_statusbar_push(priv->status_bar, 0, status);
	gtk_widget_queue_draw(priv->layout);

	return res;
}

gboolean layout_button_press_event(GtkWidget *widget, GdkEventButton *event,
		MyMainUi *self) {
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	if (priv->select_dest == NULL || priv->select_src == NULL)
		return FALSE;
	if (event->type == GDK_BUTTON_RELEASE && event->button == 3) {
		gtk_menu_popup_at_pointer(priv->menu, NULL);
	}
	return TRUE;
}
;
void menu_delete_link(GtkMenu *menu, MyMainUi *self) {
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	GList *value = g_hash_table_lookup(link_table, priv->select_src);
	value = g_list_remove(value, priv->select_dest);
	g_hash_table_insert(link_table, priv->select_src, value);
}
;

static void my_main_ui_class_init(MyMainUiClass *klass) {
	GObjectClass *obj_class = klass;
	obj_class->set_property = my_main_ui_set_property;
	obj_class->get_property = my_main_ui_get_property;
	gtk_widget_class_set_template_from_resource(klass,
			"/mymainui/MyMainUi.glade");
	gtk_widget_class_bind_template_child_private(klass, MyMainUi, layout);
	gtk_widget_class_bind_template_child_private(klass, MyMainUi, menu);
	gtk_widget_class_bind_template_child_private(klass, MyMainUi, status_bar);
	gtk_widget_class_bind_template_callback(klass, my_main_ui_add_operater);
	gtk_widget_class_bind_template_callback(klass, my_main_ui_run);
	gtk_widget_class_bind_template_callback(klass, my_main_ui_stop);
	gtk_widget_class_bind_template_callback(klass, my_main_ui_menu_open);
	gtk_widget_class_bind_template_callback(klass, my_main_ui_menu_save);
	gtk_widget_class_bind_template_callback(klass, my_main_ui_menu_download);
	gtk_widget_class_bind_template_callback(klass, my_main_ui_menu_new);
	gtk_widget_class_bind_template_callback(klass, my_main_ui_layout_draw_cb);
	gtk_widget_class_bind_template_callback(klass, layout_motion_notify_event);
	gtk_widget_class_bind_template_callback(klass, layout_button_press_event);
	gtk_widget_class_bind_template_callback(klass, menu_delete_link);
	g_object_class_install_property(klass, MAIN_UI_PROP_STATUS,
			g_param_spec_string("status", "status", "status", "",
					G_PARAM_WRITABLE));
	g_signal_new("save", MY_TYPE_MAINUI, G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(MyMainUiClass, save), NULL, NULL, NULL, G_TYPE_NONE,
			0, NULL);
	g_signal_new("load", MY_TYPE_MAINUI, G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(MyMainUiClass, load), NULL, NULL, NULL, G_TYPE_NONE,
			0, NULL);

}

static void my_main_ui_init(MyMainUi *self) {
	gtk_widget_init_template(self);
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	g_signal_connect(self, "delete-event", gtk_main_quit, NULL);
	gtk_widget_add_events(priv->layout, GDK_ALL_EVENTS_MASK);
	g_signal_connect(priv->layout, "draw", my_main_ui_layout_draw_cb, self);
	g_signal_connect(priv->layout, "motion-notify-event",
			layout_motion_notify_event, self);
	g_signal_connect(priv->layout, "button-release-event",
			layout_button_press_event, self);

	priv->download=my_download_new(default_view,NULL,NULL,NULL);
	g_signal_connect(priv->download,"delete-event",gtk_widget_hide,NULL);
	WebKitWebContext *ctx =webkit_web_context_get_default();
	g_signal_connect(ctx,"download-started",web_context_download_started,self);
}

void my_main_ui_operater_destroy_notify(MyOperater *op, MyMainUi *self) {
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	operater_list = g_list_remove(operater_list, op);
}

void my_main_ui_operater_setting(MyMainUi *self, MyOperater *operater) {
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	operater_list = g_list_append(operater_list, operater);
	gtk_layout_put(priv->layout, operater, 0, 0);
	my_operater_set_layout(operater, priv->layout);
	g_signal_connect(operater, "destroy", my_main_ui_operater_destroy_notify,
			self);
}

void my_main_ui_add_operater(GtkToolButton *button, MyMainUi *self) {
	MyOperater *operater = my_operater_new("Operater");
	my_main_ui_operater_setting(self, operater);
}
;

void my_main_ui_run(GtkToolButton *button, MyMainUi *self) {
	GList *l;
	TaskMsg *msg = NULL;
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	MAIN_START = TRUE;
	l = task_list;
	while (l != NULL) {
		my_task_run_init(l->data);
		l = l->next;
	}
	l = start_list;
	while (l != NULL) {
		msg = task_msg_new(NULL, NULL, l->data, NULL);
		my_task_run(l->data, msg);
		l = l->next;
	}
	priv->run = button;
	g_timeout_add(250, my_main_ui_status_bar_notify, self);
	gtk_widget_set_sensitive(button, FALSE);
}
;
void my_main_ui_stop(GtkToolButton *button, MyMainUi *self) {
	GList *l = task_list;
	MAIN_START = FALSE;
	while (l != NULL) {
		my_task_stop_run(l->data);
		l = l->next;
	}
}
;

void my_main_ui_menu_new(GtkMenuItem *item, MyMainUi *self) {
	GtkDialog *dialog = gtk_message_dialog_new(self, GTK_DIALOG_MODAL,
			GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO, "Clear All The Operaters ?",
			NULL);
	gint res = gtk_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	MyOperater *op;
	if (res == GTK_RESPONSE_YES) {
		while (operater_list != NULL) {
			op = operater_list->data;
			operater_list = g_list_remove(operater_list, op);
			gtk_widget_destroy(op);
		}
		g_list_free(start_list);
	}
}
;

void my_main_ui_menu_open(GtkMenuItem *item, MyMainUi *self) {
	GFile *file;
	GInputStream *in;
	gint respon = 0, i = 0, type_count = 0;
	GType type;
	GArray *array;
	MyTask *task;
	MyOperater *op;
	GList *l_task, *l_operater, *l_type, *l_child, *list, *list2;
	GHashTable *l_table;
	GtkAllocation alloc;
	gchar *task_name;
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	GtkDialog *dialog = gtk_file_chooser_dialog_new("Open Session From File",
			self, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OPEN, GTK_RESPONSE_OK,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	respon = gtk_dialog_run(dialog);
	file = gtk_file_chooser_get_file(dialog);
	gtk_widget_destroy(dialog);
	if (respon != GTK_RESPONSE_OK) {
		g_object_unref(file);
		return;
	}
	in = g_file_read(file, NULL, NULL);
	if (in == NULL) {
		g_object_unref(file);
		return;
	}
	load_table = g_hash_table_new(g_direct_hash, g_direct_equal);
//load type
	for (i = 0; i < my_type_arr->len; i++) {

		g_input_stream_read(in, &type, sizeof(GType), NULL, NULL);
		g_hash_table_insert(load_table, GSIZE_TO_POINTER(type),
				GSIZE_TO_POINTER(g_array_index(my_type_arr,GType,i)));
	}
//load task
	read_from_file(in, MY_TYPE_GLIST, &l_task, MY_TYPE_NONE);
	list = l_task;
	while (list != NULL) {
		task_name = read_string(in);
		g_input_stream_read(in, &type, sizeof(GType), NULL, NULL);
		g_print("Task Type:%x\n", type);
		type = g_hash_table_lookup(load_table, GSIZE_TO_POINTER(type));
		task = g_object_new(type, NULL);
		g_object_set(task, MY_TASK_PROP_NAME, task_name, NULL);
		my_task_load(task, in);
		g_hash_table_insert(load_table, list->data, task);
		list = list->next;
		g_free(task_name);
	}
	g_list_free(l_task);
//load operater
	read_from_file(in, MY_TYPE_GLIST, &l_operater, MY_TYPE_NONE);
	list = l_operater;
	while (list != NULL) {
		read_from_file(in, MY_TYPE_STRING, &task_name, MY_TYPE_INT, &alloc.x,
				MY_TYPE_INT, &alloc.y, MY_TYPE_GLIST, &l_child, MY_TYPE_NONE);
		op = my_operater_new(task_name);
		my_main_ui_operater_setting(self, op);
		gtk_layout_move(priv->layout, op, alloc.x, alloc.y);
		list2 = l_child;
		while (list2 != NULL) {
			task = g_hash_table_lookup(load_table, list2->data);
			if (task != NULL && MY_IS_TASK(task))
				gtk_container_add(op, task);
			list2 = list2->next;
		}
		g_list_free(l_child);
		l_child = NULL;
		list = list->next;
		g_free(task_name);
	}
	g_list_free(l_operater);
//load link table
	read_from_file(in, MY_TYPE_LINK_TABLE, &l_table, MY_TYPE_NONE);
	g_hash_table_unref(l_table);
	g_signal_emit_by_name(self, "load", NULL);
	g_object_unref(file);
	g_hash_table_unref(load_table);
}
;
void my_main_ui_menu_save(GtkMenuItem *item, MyMainUi *self) {
	gint i = 0;
	GType t;
	GOutputStream *out = NULL;
	GFile *file = NULL;
	GtkAllocation alloc;
	GList *list, *child_list;
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	GtkDialog *dialog, *message;
	gchar *task_name;
	while (TRUE) {
		dialog = gtk_file_chooser_dialog_new("Save Session To File", self,
				GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE, GTK_RESPONSE_OK,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
		i = gtk_dialog_run(dialog);
		file = gtk_file_chooser_get_file(dialog);
		gtk_widget_destroy(dialog);
		if (i == GTK_RESPONSE_OK) {
			if (g_file_query_exists(file, NULL)) {
				message = gtk_message_dialog_new(self, GTK_DIALOG_MODAL,
						GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO,
						"Replace The Exist File?", NULL);
				i = gtk_dialog_run(message);
				gtk_widget_destroy(message);
				if (i == GTK_RESPONSE_YES) {
					unlink(g_file_get_path(file));
					break;
				} else {
					g_object_unref(file);
				}
			} else {
				break;
			};
		} else {
			g_object_unref(file);
			return;
		}
	}
	g_signal_emit_by_name(self, "save", NULL);
	out = g_file_create(file, G_FILE_CREATE_REPLACE_DESTINATION, NULL, NULL);
	if (out == NULL) {
		g_object_unref(file);
		return;
	}
	//保存Task的Gtype信息；
	i = 0;
	for (i = 0; i < my_type_arr->len; i++) {
		t = g_array_index(my_type_arr, GType, i);
		g_output_stream_write(out, &t, sizeof(GType), NULL, NULL);
	}
	//保存所有task信息并并保存；
	task_list = g_list_first(task_list);
	write_to_file(out, MY_TYPE_GLIST, task_list, MY_TYPE_NONE);
	list = task_list;
	while (list != NULL) {
		g_object_get(list->data, MY_TASK_PROP_NAME, &task_name, NULL);
		write_string(out, task_name);
		g_free(task_name);
		my_task_save(list->data, out);
		list = list->next;
	}
	//保存operater信息；
	operater_list = g_list_first(operater_list);
	write_to_file(out, MY_TYPE_GLIST, operater_list, MY_TYPE_NONE);
	list = operater_list;
	while (list != NULL) {
		gtk_widget_get_allocation(list->data, &alloc);
		child_list = gtk_container_get_children(list->data);
		write_to_file(out, MY_TYPE_STRING, my_operater_get_title(list->data),
				MY_TYPE_INT, &alloc.x, MY_TYPE_INT, &alloc.y, MY_TYPE_GLIST,
				child_list, MY_TYPE_NONE);
		g_list_free(child_list);
		list = list->next;
	}
	//保存连接表信息
	if (link_table != NULL) {
		write_to_file(out, MY_TYPE_LINK_TABLE, link_table, MY_TYPE_NONE);
	}
	//关闭输出流；
	g_output_stream_close(out, NULL, NULL);
	g_object_unref(out);
	g_object_unref(file);
}
;

void my_main_ui_menu_download(GtkMenuItem *item, MyMainUi *self){
MyMainUiPrivate *priv=my_main_ui_get_instance_private(self);
gtk_widget_show(priv->download);
gtk_widget_grab_focus(priv->download);
};

void web_context_download_started(WebKitWebContext *context,
		WebKitDownload *download, MyMainUi *self){
	MyMainUiPrivate *priv=my_main_ui_get_instance_private(self);
	if(g_object_get_data(download,"my_download")==NULL){
		my_download_add_webkitdownload(priv->download,download);
	}
};


gboolean my_main_ui_layout_draw_cb(GtkWidget *widget, cairo_t *cr,
		MyMainUi *self) {
	GtkWidget *src, *dest;
	GList *key = NULL, *value = NULL, *k = NULL, *v = NULL;
	GtkAllocation src_alloc, dest_alloc;
	MyMainUiPrivate *priv = my_main_ui_get_instance_private(self);
	priv->select_dest = NULL;
	priv->select_src = NULL;
	if (link_table == NULL)
		return FALSE;
	key = g_hash_table_get_keys(link_table);
	if (key == NULL)
		return FALSE;
	k = key;
	do { //清除无效的源
		if (!GTK_IS_WIDGET(k->data)) {
			g_hash_table_remove(link_table, k->data);
		}
		k = k->next;
	} while (k != NULL);
	g_list_free(key);
	key = g_hash_table_get_keys(link_table);
	if (key == NULL)
		return FALSE;
	k = key;
	do {
		src = k->data;
		value = g_hash_table_lookup(link_table, src);
		if (value != NULL) {
			v = value;
			do { //清除无效的目标
				if (!GTK_IS_WIDGET(v->data)) {
					v = g_list_remove(value, v->data);
					value = v;
					g_hash_table_insert(link_table, src, value);
				} else {
					v = v->next;
				}
			} while (v != NULL);
			v = value;
			while (v != NULL) { //绘制连线
				dest = v->data;
				gtk_widget_get_allocation(src, &src_alloc);
				gtk_widget_get_allocation(dest, &dest_alloc);
				cairo_save(cr);
				cairo_move_to(cr, src_alloc.x + src_alloc.width,
						src_alloc.y + src_alloc.height / 2.0);
				cairo_line_to(cr, src_alloc.x + src_alloc.width + 10.,
						src_alloc.y + src_alloc.height / 2.0);
				cairo_line_to(cr, dest_alloc.x - 10.,
						dest_alloc.y + dest_alloc.height / 2.0);
				cairo_line_to(cr, dest_alloc.x,
						dest_alloc.y + dest_alloc.height / 2.0);
				cairo_set_line_width(cr, 10);
				cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
				cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
				if (cairo_in_stroke(cr, priv->mouse_x, priv->mouse_y)) { //该连接被鼠标选中
					cairo_set_source_rgba(cr, 1.0, 0, 0, 0.5);
					priv->select_dest = dest;
					priv->select_src = src;
				} else {
					cairo_set_source_rgba(cr, 0, 1., 0, 0.5);
				}
				cairo_stroke(cr);
				cairo_restore(cr);
				v = v->next;
			};
		} else {
			g_hash_table_remove(link_table, src);
		}
		k = k->next;
	} while (k != NULL);
	g_list_free(key);
	return FALSE;
}
;

MyMainUi *my_main_ui_new() {
	return g_object_new(MY_TYPE_MAINUI, NULL);
}
;

