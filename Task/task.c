/*
 * task.c
 *
 *  Created on: 2018年10月7日
 *      Author: tom
 */

#include "task.h"

enum {
	task_prop_name = 1, task_prop_log, task_prop_log_filename,
};

GtkTargetEntry task_entry[] = { { .flags = GTK_TARGET_SAME_APP, .info = 0,
		.target = "Same APP" }, { .flags = GTK_TARGET_OTHER_WIDGET, .info = 1,
		.target = "Text" }, { .flags = GTK_TARGET_OTHER_APP, .info = 2,
		.target = "Other APP" }, }; //拖放操作元件能接受的目标;

typedef struct {
	GIOChannel *log_io;
	gchar *name, *log_file;
	gboolean log;
	void *data;
	GQueue *queue;
	GHashTable *link_table;
	GtkMenu *menu;
} MyTaskPrivate;

G_DEFINE_TYPE_WITH_CODE(MyTask, my_task, GTK_TYPE_BOX, G_ADD_PRIVATE(MyTask));

void my_task_set_property(GObject *object, guint property_id,
		const GValue *value, GParamSpec *pspec) {
	MyTaskPrivate *priv = my_task_get_instance_private(object);
	switch (property_id) {
	case task_prop_log:
		priv->log = g_value_get_boolean(value);
		break;
	case task_prop_log_filename:
		if (priv->log_file != NULL)
			g_free(priv->log_file);
		priv->log_file = g_strdup(g_value_get_string(value));
		break;
	case task_prop_name:
		if (priv->name != NULL)
			g_free(priv->name);
		priv->name = g_strdup(g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}
;
void my_task_get_property(GObject *object, guint property_id, GValue *value,
		GParamSpec *pspec) {
	MyTaskPrivate *priv = my_task_get_instance_private(object);
	switch (property_id) {
	case task_prop_log:
		g_value_set_boolean(value, priv->log);
		break;
	case task_prop_log_filename:
		g_value_set_string(value, priv->log_file);
		break;
	case task_prop_name:
		g_value_set_string(value, priv->name);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}
;

void my_task_dispose(GObject *object) {
	MyTaskPrivate *priv = my_task_get_instance_private(object);
	if (priv->log_io != NULL) {
		g_io_channel_close(priv->log_io);
		g_io_channel_unref(priv->log_io);
	}
	if (priv->link_table != NULL)
		g_hash_table_destroy(priv->link_table);
	if (priv->queue != NULL)
		g_queue_free(priv->queue);

	priv->log_io = NULL;
	priv->link_table = NULL;
	priv->queue = NULL;
	task_list = g_list_remove(task_list, object);
	G_OBJECT_CLASS(my_task_parent_class)->dispose(object);
}
;
void my_task_finalize(GObject *object) {
	MyTaskPrivate *priv = my_task_get_instance_private(object);
	g_free(priv->data);
	g_free(priv->log_file);
	g_free(priv->name);
	G_OBJECT_CLASS(my_task_parent_class)->finalize(object);
}
;


void move_up_activate_cb(GtkMenuItem *menuitem, MyTask *self) {
	gint i = 0;
	GtkBox *box = gtk_widget_get_parent(self);
	gtk_container_child_get(box, self, "position", &i, NULL);
	if (i > 0)
		i--;
	gtk_container_child_set(box, self, "position", i, NULL);
}
;
void move_down_activate_cb(GtkMenuItem *menuitem, MyTask *self) {
	gint i = 0;
	GtkBox *box = gtk_widget_get_parent(self);
	gtk_container_child_get(box, self, "position", &i, NULL);
	i++;
	gtk_container_child_set(box, self, "position", i, NULL);

}
;
void re_name_activate_cb(GtkMenuItem *menuitem, MyTask *self) {
	gchar *name = NULL;
	g_object_get(self, MY_TASK_PROP_NAME, &name, NULL);
	GtkDialog *dialog = gtk_dialog_new_with_buttons("RENAME", main_window,
			GTK_DIALOG_MODAL, "Rename", GTK_RESPONSE_OK, "Cancle",
			GTK_RESPONSE_CANCEL, NULL);
	GtkEntry *entry = gtk_entry_new();
	gtk_entry_set_text(entry, name);
	gtk_container_add(gtk_dialog_get_content_area(dialog), entry);
	gtk_widget_show_all(dialog);
	if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
		g_object_set(self, MY_TASK_PROP_NAME, gtk_entry_get_text(entry), NULL);
	}
	g_free(name);
	gtk_widget_destroy(entry);
	gtk_widget_destroy(dialog);
}
;

void rm_link_activate_cb(GtkMenuItem *menuitem, MyTask *self) {
	GList *list;
	GtkWidget *widget;
	if (link_table == NULL)
		return;
	widget = g_object_get_data(self, MY_TASK_RIGHT_CLICKED_WIDGET);
	if (widget == NULL)
		return;
	list = g_hash_table_lookup(link_table, widget);
	if (list == NULL)
		return;
	g_hash_table_remove(link_table, widget);
	g_list_free(list);
}
;

void del_task_activate_cb(GtkMenuItem *menuitem, MyTask *self) {
	gtk_widget_destroy(self);

}
;

static void my_task_class_init(MyTaskClass *klass) {
	GObjectClass *obj_class = klass;
	klass->run = NULL;
	klass->run_finish = NULL;
	klass->run_init = NULL;
	klass->stop_run=NULL;
	obj_class->get_property = my_task_get_property;
	obj_class->set_property = my_task_set_property;
	obj_class->dispose = my_task_dispose;
	obj_class->finalize = my_task_finalize;
	gtk_widget_class_set_template_from_resource(klass, "/mytask/MyTask.glade");
	gtk_widget_class_bind_template_child_private(klass, MyTask, menu);
	gtk_widget_class_bind_template_callback(klass, move_up_activate_cb);
	gtk_widget_class_bind_template_callback(klass, move_down_activate_cb);
	gtk_widget_class_bind_template_callback(klass, re_name_activate_cb);
	gtk_widget_class_bind_template_callback(klass, rm_link_activate_cb);
	gtk_widget_class_bind_template_callback(klass, del_task_activate_cb);
	g_object_class_install_property(klass, task_prop_name,
			g_param_spec_string(MY_TASK_PROP_NAME, "name", "Task Name", "task",
					G_PARAM_READWRITE));
	g_object_class_install_property(klass, task_prop_log_filename,
			g_param_spec_string(MY_TASK_PROP_LOG_FILE, "log_filename",
					"Task Log_filename", "task_log.txt", G_PARAM_READWRITE));
	g_object_class_install_property(klass, task_prop_log,
			g_param_spec_boolean(MY_TASK_PROP_LOG, "log", "log or not", FALSE,
					G_PARAM_READWRITE));
}

static void my_task_init(MyTask *self) {
	gtk_widget_init_template(self);
	MyTaskPrivate *priv = my_task_get_instance_private(self);
	priv->link_table = NULL;
	priv->data = NULL;
	priv->log = TRUE;
	priv->log_file = g_strdup("task_log.html");
	priv->log_io = NULL;
	priv->name = g_strdup("task");
	priv->queue = NULL;
	task_list = g_list_append(task_list, self);
}

void my_task_run_init(MyTask *self) {
	MyTaskClass *klass = MY_TASK_GET_CLASS(self);
	MyTaskPrivate *priv = my_task_get_instance_private(self);
	if (priv->log && priv->log_file != NULL) {
		if (priv->log_io == NULL)
			priv->log_io = g_io_channel_new_file(priv->log_file, "a+", NULL);
		if (priv->log_io != NULL)
			my_task_log(self,
					"<html><head><style>\
					p.err{color:red;font-size:15px}\
					p.time{color:blue;font-size:15px}\
					div{width: 1000px;\
					border-style: outset;\
					border-top-style: outset;\
					border-right-style: outset;\
					border-bottom-style: outset;\
    				border-left-style: outset;\
					border-width: 3px;\
					border-radius: 10px;\
					background-color: beige;\
					nowrap:'true';}\
					</style>\
					<meta http-equiv='content-type' content='text/html;charset=utf-8'>\
					</head><body>");
	}
	if (klass->run_init != NULL)
		klass->run_init(self);
}
;
void my_task_run(MyTask *self, TaskMsg *msg) {
	runing_count++;
	gchar **strv;
	guint i = 0;
	GValue from = G_VALUE_INIT, to = G_VALUE_INIT;
	MyTaskClass *klass = MY_TASK_GET_CLASS(self);
	MyTaskPrivate *priv = my_task_get_instance_private(self);
	if (priv->log && priv->log_io == NULL) {
		my_task_run_init(self);
	}
	if (priv->log_io != NULL) {
		GDateTime *time = g_date_time_new_now_local();
		gchar *str_time = g_date_time_format(time, "%Y-%m-%d %T");
		if (msg == NULL) {
			my_task_log(self,
					"<p class='time'>%s</p><div class='err'><p class='err'>Receive Wrong Format Msg</p></div></br>",
					str_time);
		} else {
			strv = g_strsplit(msg->msg, "\n", -1);
			g_value_init(&from, G_TYPE_STRING);
			g_value_init(&to, G_TYPE_STRING);
			g_object_get_property(msg->from_task, MY_TASK_PROP_NAME, &from);
			g_object_get_property(msg->to_task, MY_TASK_PROP_NAME, &to);
			my_task_log(self,
					"<p class='time'>%s</p>\
					<div><table border='2px'>\
					<tr><td>Proccess Msg</td><td>%s->%s</td></tr>\
					<tr><td>URL</td><td>%s</td></tr>\
					<tr><td>Context</td><td>",
					str_time, g_value_get_string(&from),
					g_value_get_string(&to),
					webkit_web_view_get_uri(msg->view));
			while (strv != NULL && strv[i] != NULL) {
				my_task_log(self, "%s</br>", strv[i]);
				i++;
			}
			my_task_log(self, "</td></tr>\
					</table></div></br>");
			g_value_unset(&from);
			g_value_unset(&to);
			g_strfreev(strv);
		}
		g_free(str_time);
		g_date_time_unref(time);
	}
	if (klass->run != NULL) {
		klass->run(self, msg);
	}
	runing_count--;
}
;

void my_task_run_finish(MyTask *self) {
	MyTaskClass *klass = MY_TASK_GET_CLASS(self);
	MyTaskPrivate *priv = my_task_get_instance_private(self);
	if (priv->log_io != NULL) {
		my_task_log(self, "</body></html>");
		g_io_channel_close(priv->log_io);
		g_io_channel_unref(priv->log_io);
		priv->log_io = NULL;
	}
	if (klass->run_finish != NULL)
		klass->run_finish(self);
}
;
void my_task_save(MyTask *self, GOutputStream *out) {
	if (MY_TASK_GET_CLASS(self)->save != NULL)
		MY_TASK_GET_CLASS(self)->save(self, out);
}
;

void my_task_load(MyTask *self, GInputStream *in) {
	if (MY_TASK_GET_CLASS(self)->load != NULL)
		MY_TASK_GET_CLASS(self)->load(self, in);
}
;
void my_task_stop_run(MyTask *self){
	if (MY_TASK_GET_CLASS(self)->stop_run != NULL)
		MY_TASK_GET_CLASS(self)->stop_run(self);
};

gboolean my_task_run_source(TaskMsg *msg) {
	GMutex *mutex = NULL;
	if (WEBKIT_IS_WEB_VIEW(msg->view)) { //任务务可能对webview进行操作，对webview上锁；
		mutex = g_object_get_data(msg->view, "mutex");
		if (g_mutex_trylock(mutex)) {
			my_task_run(msg->to_task, msg);
		} else {
			return G_SOURCE_CONTINUE;
		};
	} else { //无webview进行操作，直接运行；
		my_task_run(msg->to_task, msg);
	}
	return G_SOURCE_REMOVE;
}

void my_task_send_msg(MyTask *self, MyTask *target, void *view, gchar* context) {
	if (MAIN_START) {
		MyTask *target_task = target;
		if (!MY_IS_TASK(target_task)) { //由于link_table只记录的是MyTask的子控件间的连接，需通过子控件找回母控件MyTask；
			target_task = gtk_widget_get_parent(target_task);
		}
		TaskMsg *msg = task_msg_new(view, context, self, target_task);
		g_idle_add(my_task_run_source, msg);
	}
}
;

gboolean my_task_log(MyTask *self, gchar *format, ...) {
	MyTaskClass *klass = MY_TASK_GET_CLASS(self);
	MyTaskPrivate *priv = my_task_get_instance_private(self);
	if (priv->log) {
		if (priv->log_io == NULL) {
			my_task_run_init(self);
		}
		static gchar buf[512 * 1024];
		gsize n;
		va_list p;
		va_start(p, NULL);
		n = vsnprintf(buf, sizeof(buf), format, p);
		g_io_channel_write_chars(priv->log_io, buf, -1, NULL, NULL);
		g_io_channel_flush(priv->log_io, NULL);
		va_end(p);
		return TRUE;
	}
	return FALSE;
}
;

void my_task_rename_id(MyTask *task, gchar *name, uint id) {
	gchar *temp = g_strdup_printf("%s%u", name, id);
	g_object_set(task, MY_TASK_PROP_NAME, temp, NULL);
	g_free(temp);
}
;

MyTask *my_task_get_next_task(MyTask *task) {
	GtkWidget *container;
	GList *list, *l;
	MyTask *next_task = NULL;
	container = gtk_widget_get_parent(task);
	list = gtk_container_get_children(container);
	if (list == NULL)
		return NULL;
	l = g_list_find(list, task);
	if (l->next != NULL) {
		next_task = l->next->data;
	}
	g_list_free(list);
	return next_task;
}
;

void my_task_drag_dest_set(GtkWidget *widget) {
	gtk_drag_dest_set(widget, GTK_DEST_DEFAULT_ALL, task_entry, 3,
			GDK_ACTION_MOVE);
	g_signal_handlers_disconnect_by_func(widget, my_task_drag_data_received,
			NULL);
	g_signal_connect(widget, "drag-data-received", my_task_drag_data_received,
			NULL);
	gtk_widget_add_events(widget, GDK_ALL_EVENTS_MASK);
}
;
void my_task_drag_source_set(GtkWidget *widget) {
	gtk_drag_source_set(widget, GDK_BUTTON1_MASK, task_entry, 3,
			GDK_ACTION_MOVE);
	g_signal_handlers_disconnect_by_func(widget, my_task_drag_data_get, NULL);
	g_signal_connect(widget, "drag-data-get", my_task_drag_data_get, NULL);
	gtk_widget_add_events(widget, GDK_ALL_EVENTS_MASK);
}
;
void my_task_drag_data_get(GtkWidget *widget, GdkDragContext *context,
		GtkSelectionData *selection_data, guint info, guint time_) {
	switch (info) {
	case 0:
		gtk_selection_data_set(selection_data,
				gdk_atom_intern("Same APP", TRUE), 0, &widget,
				sizeof(gpointer));
		break;
	default:
		break;
	}
}
;
void my_task_drag_data_received(GtkWidget *widget, GdkDragContext *context,
		gint x, gint y, GtkSelectionData *selection_data, guint info,
		guint time_) {
	GtkWidget **source;
	GList *list = NULL;
	if (link_table == NULL) {
		link_table = g_hash_table_new(g_direct_hash, g_direct_equal);
	}
	switch (info) {
	case 0:
		source = gtk_selection_data_get_data(selection_data);
		list = g_hash_table_lookup(link_table, *source);
		list = g_list_append(list, widget);
		g_hash_table_insert(link_table, *source, list);
		break;
	default:
		break;
	}

}
;

gboolean task_press_cb(GtkWidget *widget, GdkEvent *event, MyTask *self) {
	MyTaskPrivate *priv = my_task_get_instance_private(self);
	GtkMenu *menu;
	if (event->button.button == 3) {
		//g_print("right clicked\n");
		g_signal_emit_by_name(self, "right_clicked", event);
		g_object_set_data(self, MY_TASK_RIGHT_CLICKED_WIDGET, widget);
		gtk_menu_popup_at_pointer(priv->menu, NULL);
		return TRUE;
	}
	return FALSE;
}
;

void my_task_set_right_clicked(GtkWidget *widget, MyTask *task) {
	g_signal_connect(widget, "button-press-event", task_press_cb, task);
}

GIOChannel *my_task_get_log_io(MyTask *self) {
	MyTaskPrivate *priv = my_task_get_instance_private(self);
	return priv->log_io;
}
;
