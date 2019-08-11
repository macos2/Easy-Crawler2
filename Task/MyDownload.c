/*
 * my_download.c
 *
 *  Created on: 2018年3月12日
 *      Author: tom
 */
#include "MyDownload.h"
typedef struct {
	gint count;
	gchar *name;
	GtkTreeView *view;
	GtkListStore *download_store;
	GtkImage *finish, *stop, *downloading, *error, *wait;
	GMutex store_mutex;
	WebKitWebView *web_view;
	GtkMenu *menu;
	GtkDialog *rename_dialog, *add_dialog, *set_dialog;
	GtkTextBuffer *add_uri_buffer;
	GtkEntry *rename_entry, *prefix_entry, *suffix_entry, *name_format;
	GtkFileChooserButton *save_location, *add_from_backup_file;
	gchar *perfix, *suffix, *save_dir;
	guint same_filename_operation, max_count/*最大同时下载的任务数量*/, add_count; //用于生成唯一任务id;
	GtkRadioButton *over_write, *skip_download, *rename_suffix_number;
	GtkCheckButton *skip_same_url, *auto_backup, *special_name_format;
	GString *downloaded_url;
	GAsyncQueue *download_queue;
} MyDownloadPrivate;

typedef struct {
	gchar *url;
	gchar *dir;
	GtkTreeRowReference *ref;
} DownloadQueueData;

enum download_property {
	prop_count = 1, prop_name, prop_total_count, n_property
};

enum column {
	col_name,
	col_progress,
	col_speed,
	col_time_elapsed,
	col_state_pixfuf,
	col_speed_text,
	col_time_elapsed_text,
	col_time_started_text,
	col_url,
	col_time_start,
	col_size_text,
	col_size,
	col_state,
	col_webkitdownload,
	col_path,
	col_s_title
};

static gchar *size_unit[] = { "Byte", "KB", "MB", "GB", "TB" };
static GParamSpec *download_prop[n_property] = { NULL, };

G_DEFINE_TYPE_WITH_CODE(MyDownload, my_download, GTK_TYPE_WINDOW,
		G_ADD_PRIVATE(MyDownload));

void my_download_get_property(GObject *object, guint property_id, GValue *value,
		GParamSpec *pspe);
void my_download_set_property(GObject *object, guint property_id,
		const GValue *value, GParamSpec *pspec);
void my_download_dispose(GObject *object);
void my_download_finalize(GObject *object);

gboolean my_download_view_button_press(GtkWidget *view, GdkEventButton *event,
		MyDownload *MyDownload);
void menu_open_file(GtkMenuItem *menuitem, MyDownload *MyDownload);
void menu_open_save_location(GtkMenuItem *menuitem, MyDownload *MyDownload);
void menu_copy_link(GtkMenuItem *menuitem, MyDownload *MyDownload);
void menu_rename(GtkMenuItem *menuitem, MyDownload *MyDownload);
void menu_delete(GtkMenuItem *menuitem, MyDownload *MyDownload);
void menu_delete_with_file(GtkMenuItem *menuitem, MyDownload *MyDownload);
void menu_continue(GtkMenuItem *menuitem, MyDownload *MyDownload);
void menu_stop(GtkMenuItem *menuitem, MyDownload *MyDownload);
void my_download_add_button_clicked(GtkToolButton *toolbutton,
		MyDownload *MyDownload);
void my_download_set_button_clicked(GtkToolButton *toolbutton,
		MyDownload *MyDownload);
gboolean webkit_download_decide_destination(WebKitDownload *download,
		gchar *suggested_filename, gpointer user_data);
void webkit_download_created_destination(WebKitDownload *download,
		gchar *destination, gpointer user_data);
void webkit_download_finish(WebKitDownload *download, gpointer user_data);
void webkit_download_fail(WebKitDownload *download, GError *error,
		gpointer user_data);
void webkit_download_received_data(WebKitDownload *download,
		guint64 data_length, gpointer user_data);
DownloadQueueData *my_download_queue_data_new(gchar *url, gchar *dir,
		GtkTreeRowReference *ref);
void my_download_queue_data_free(DownloadQueueData *data);
void my_download_queue_pop(MyDownload *self);
void my_download_queue_push(MyDownload *self, gchar *url, gchar *dir,
		GtkTreeRowReference *ref);

static void my_download_class_init(MyDownloadClass *klass) {
	GObjectClass *obj_class = klass;
	GtkWidgetClass *gtk_class = klass;
	obj_class->get_property = my_download_get_property;
	obj_class->set_property = my_download_set_property;
	obj_class->finalize = my_download_finalize;
	obj_class->dispose = my_download_dispose;
	download_prop[prop_count] = g_param_spec_int("count", "count", "count", 0,
	G_MAXINT, 0, G_PARAM_READWRITE);
	download_prop[prop_total_count] = g_param_spec_int("total_count",
			"total_count", "total_count", 0,
			G_MAXINT, 0, G_PARAM_READABLE);
	download_prop[prop_name] = g_param_spec_string("name", "name", "name",
			"DownLoad", G_PARAM_READWRITE);
	g_object_class_install_properties(obj_class, n_property, download_prop);
	gtk_widget_class_set_template_from_resource(gtk_class,
			"/mytask/MyDownload.glade");
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload, view);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			download_store);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload, menu);
	//载入状态图像
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload, finish);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload, error);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			downloading);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload, stop);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload, wait);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			rename_dialog);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			rename_entry);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			prefix_entry);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			suffix_entry);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			add_uri_buffer);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			save_location);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			add_dialog);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			set_dialog);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			over_write);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			skip_download);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			rename_suffix_number);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			name_format);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			special_name_format);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			skip_same_url);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			auto_backup);
	gtk_widget_class_bind_template_child_private(gtk_class, MyDownload,
			add_from_backup_file);

	gtk_widget_class_bind_template_callback(gtk_class,
			my_download_view_button_press);
	gtk_widget_class_bind_template_callback(gtk_class, menu_open_file);
	gtk_widget_class_bind_template_callback(gtk_class, menu_open_save_location);
	gtk_widget_class_bind_template_callback(gtk_class, menu_copy_link);
	gtk_widget_class_bind_template_callback(gtk_class, menu_rename);
	gtk_widget_class_bind_template_callback(gtk_class, menu_delete);
	gtk_widget_class_bind_template_callback(gtk_class, menu_delete_with_file);
	gtk_widget_class_bind_template_callback(gtk_class, menu_continue);
	gtk_widget_class_bind_template_callback(gtk_class, menu_stop);
	gtk_widget_class_bind_template_callback(gtk_class,
			my_download_add_button_clicked);
	gtk_widget_class_bind_template_callback(gtk_class,
			my_download_set_button_clicked);
	g_signal_new("download_start", MY_TYPE_DOWNLOAD, G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(MyDownloadClass, download_start), NULL, NULL, NULL,
			G_TYPE_NONE, 3, WEBKIT_TYPE_DOWNLOAD, GTK_TYPE_LIST_STORE,
			GTK_TYPE_TREE_ROW_REFERENCE, NULL);
	g_signal_new("download_finish", MY_TYPE_DOWNLOAD, G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(MyDownloadClass, download_finish), NULL, NULL, NULL,
			G_TYPE_NONE, 3, WEBKIT_TYPE_DOWNLOAD, GTK_TYPE_LIST_STORE,
			GTK_TYPE_TREE_ROW_REFERENCE, NULL);
	if (uri_regex == NULL)
		uri_regex = g_regex_new("%u", 0, 0, NULL);
	if (ori_regex == NULL)
		ori_regex = g_regex_new("%f", 0, 0, NULL);
	if (title_regex == NULL)
		title_regex = g_regex_new("%w", 0, 0, NULL);
	if (date_regex == NULL)
		date_regex = g_regex_new("%d", 0, 0, NULL);
	if (time_regex == NULL)
		time_regex = g_regex_new("%t", 0, 0, NULL);

}
static void my_download_init(MyDownload *self) {
	gtk_widget_init_template(self);
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	priv->name = g_strdup("Download");
	priv->download_store = gtk_tree_view_get_model(priv->view);
	priv->perfix = g_strdup("");
	priv->suffix = g_strdup("");
	priv->save_dir = g_strdup("");
	priv->same_filename_operation = same_filename_rename_add_suffix;
	priv->downloaded_url = g_string_new("");
	priv->add_count = 1;
	priv->max_count = 5;
	priv->download_queue = g_async_queue_new();
	gtk_toggle_button_set_active(priv->rename_suffix_number, TRUE);
	gtk_toggle_button_set_active(priv->skip_same_url, TRUE);
	gtk_toggle_button_set_active(priv->auto_backup, TRUE);
	gtk_toggle_button_set_active(priv->special_name_format, FALSE);
	g_mutex_init(&priv->store_mutex);
}

void my_download_count_modify(MyDownload *self, gint i) {
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	priv->count += i;
	g_object_notify(self, "count");
	while (priv->count < priv->max_count
			&& g_async_queue_length(priv->download_queue) > 0) {
		my_download_queue_pop(self);
	}
}

gint my_download_get_downloading_count(MyDownload *self) {
	gint count = 0;
	g_object_get(self, "count", &count, NULL);
	return count;
}

GList* g_list_next_with_free(GList *list, GDestroyNotify free_func) {
	GList *res = NULL;
	if (list->next == NULL) {
		res = NULL;
		g_list_free_full(list, free_func);
	} else {
		res = list->next;
	}
	return res;
}
;

void my_download_get_property(GObject *object, guint property_id, GValue *value,
		GParamSpec *pspec) {

	MyDownloadPrivate *priv = my_download_get_instance_private(object);
	switch (property_id) {
	case prop_count:
		g_value_set_int(value, priv->count);
		break;
	case prop_total_count:
		g_value_set_int(value,
				gtk_tree_model_iter_n_children(priv->download_store, NULL));
		break;
	case prop_name:
		g_value_set_string(value, priv->name);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}

}
;
void my_download_set_property(GObject *object, guint property_id,
		const GValue *value, GParamSpec *pspec) {
	MyDownloadPrivate *priv = my_download_get_instance_private(object);
	switch (property_id) {
	case prop_count:
		priv->count = g_value_get_int(value);
		break;
	case prop_name:
		g_free(priv->name);
		priv->name = g_strdup(g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}
;

void my_download_dispose(GObject *object) {

}
;
void my_download_finalize(GObject *object) {
	MyDownloadPrivate *priv = my_download_get_instance_private(object);
	g_async_queue_unref(priv->download_queue);
	g_string_free(priv->downloaded_url, TRUE);
	g_free(priv->perfix);
	g_free(priv->suffix);
	g_free(priv->save_dir);
	g_free(priv->name);
}
;

gboolean my_download_view_button_press(GtkWidget *view, GdkEventButton *event,
		MyDownload *MyDownload) {
	MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
	if (event->button == 3) {
		gtk_menu_popup_at_pointer(priv->menu,
		NULL);
		return TRUE;
	}
	return FALSE;
}
;

void menu_open_file(GtkMenuItem *menuitem, MyDownload *MyDownload) {
	MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
	GtkTreeSelection *selec = gtk_tree_view_get_selection(priv->view);
	GList *selec_list = gtk_tree_selection_get_selected_rows(selec,
			&priv->download_store);
	GtkTreePath *path;
	GtkTreeIter iter;
	GValue value = G_VALUE_INIT;
	gchar *temp;
	while (selec_list != NULL) {
		path = selec_list->data;
		if (gtk_tree_model_get_iter(priv->download_store, &iter, path)) {
			gtk_tree_model_get_value(priv->download_store, &iter, col_path,
					&value);
			temp = g_strdup_printf("file://%s", g_value_get_string(&value));
			gtk_show_uri_on_window(MyDownload, temp, GDK_CURRENT_TIME, NULL);
			g_free(temp);
			g_value_unset(&value);
		};
		if (selec_list->next == NULL) {
			g_list_free_full(selec_list, (GDestroyNotify) gtk_tree_path_free);
			selec_list = NULL;
		} else {
			selec_list = selec_list->next;
		}
	}
}
;

void menu_open_save_location(GtkMenuItem *menuitem, MyDownload *MyDownload) {
	MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
	GtkTreeSelection *selec = gtk_tree_view_get_selection(priv->view);
	GList *selec_list = gtk_tree_selection_get_selected_rows(selec,
			&priv->download_store);
	GtkTreePath *path;
	GtkTreeIter iter;
	GValue value = G_VALUE_INIT;
	WebKitDownload *download;
	gchar *temp;
	GFile *file,
	*dir;
	while (selec_list != NULL) {
		path = selec_list->data;
		if (gtk_tree_model_get_iter(priv->download_store, &iter, path)) {
			gtk_tree_model_get_value(priv->download_store, &iter, col_path,
					&value);
			temp = g_value_get_string(&value);
			if (temp != NULL) {
				file = g_file_new_for_path(temp);
				dir = g_file_get_parent(file);
				temp = g_file_get_uri(dir);
				gtk_show_uri_on_window(MyDownload, temp, GDK_CURRENT_TIME,
						NULL);
				g_object_unref(dir);
				g_object_unref(file);
				g_free(temp);
			}
			g_value_unset(&value);
		};
		if (selec_list->next == NULL) {
			g_list_free_full(selec_list, (GDestroyNotify) gtk_tree_path_free);
			selec_list = NULL;
		} else {
			selec_list = selec_list->next;
		}
	}
}
;

void menu_stop(GtkMenuItem *menuitem, MyDownload *MyDownload) {

	MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
	GtkTreeSelection *selec = gtk_tree_view_get_selection(priv->view);
	GList *selec_list = gtk_tree_selection_get_selected_rows(selec,
			&priv->download_store);
	GtkTreePath *path;
	GtkTreeIter iter;
	//GValue value = G_VALUE_INIT;
	WebKitDownload *download;
	while (selec_list != NULL) {
		path = selec_list->data;
		if (gtk_tree_model_get_iter(priv->download_store, &iter, path)) {
			/*gtk_tree_model_get_value(priv->download_store, &iter,
			 col_webkitdownload, &value);
			 download = g_value_get_pointer(&value);*/
			download = NULL;
			gtk_tree_model_get(priv->download_store, &iter, col_webkitdownload,
					&download);
			if (download != NULL)
				webkit_download_cancel(download);
			//g_value_unset(&value);
			gtk_list_store_set(priv->download_store, &iter, col_webkitdownload,
					NULL, -1);
		};
		if (selec_list->next == NULL) {
			g_list_free_full(selec_list, (GDestroyNotify) gtk_tree_path_free);
			selec_list = NULL;
		} else {
			selec_list = selec_list->next;
		}
	}
}
;
void menu_continue(GtkMenuItem *menuitem, MyDownload *MyDownload) {
	guint state = 0;
	MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
	GtkTreeSelection *selec = gtk_tree_view_get_selection(priv->view);
	GList *selec_list = gtk_tree_selection_get_selected_rows(selec,
			&priv->download_store);
	GtkTreePath *path;
	GtkTreeIter iter;
	GValue value = G_VALUE_INIT;
	WebKitDownload *download;
	GtkTreeRowReference *row_ref;
	gchar *s_dir,
	*url;
	GdkPixbuf *pixbuf;
	while (selec_list != NULL) {
		path = selec_list->data;
		if (gtk_tree_model_get_iter(priv->download_store, &iter, path)) {
			gtk_tree_model_get_value(priv->download_store, &iter,
					col_webkitdownload, &value);
			download = g_value_get_pointer(&value);
			if (download == NULL) { //下载完成或下载失败的情况；
				gtk_tree_model_get(priv->download_store, &iter, col_state,
						&state, -1);
				if (state != MY_DOWNLOAD_FINISH) {
					pixbuf = gtk_image_get_pixbuf(priv->wait);
					g_mutex_lock(&priv->store_mutex);
					gtk_list_store_set(priv->download_store, &iter, col_state,
							MY_DOWNLOAD_WAIT, col_state_pixfuf, pixbuf, -1);
					row_ref = gtk_tree_row_reference_new(priv->download_store,
							path);
					g_mutex_unlock(&priv->store_mutex);
					gtk_tree_model_get(priv->download_store, &iter, col_url,
							&url, col_path, &s_dir, -1);
					my_download_queue_push(MyDownload, url, s_dir, row_ref);
					g_free(s_dir);
					g_free(url);
				}
			};
			g_value_unset(&value);
		};
		if (selec_list->next == NULL) {
			g_list_free_full(selec_list, (GDestroyNotify) gtk_tree_path_free);
			selec_list = NULL;
		} else {
			selec_list = selec_list->next;
		}
	}
}
;
void menu_rename(GtkMenuItem *menuitem, MyDownload *MyDownload) {
	MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
	GtkTreeSelection *selec = gtk_tree_view_get_selection(priv->view);
	g_mutex_lock(&priv->store_mutex);
	GList *selec_list = gtk_tree_selection_get_selected_rows(selec,
			&priv->download_store);
	GtkTreePath *path;
	GtkTreeIter iter;
	GValue value = G_VALUE_INIT;
	GFile *file,
	*rename_file;
	gchar *bname, *save_path;
	while (selec_list != NULL) {
		path = selec_list->data;
		gtk_tree_model_get_iter(priv->download_store, &iter, path);
		gtk_tree_model_get_value(priv->download_store, &iter, col_path, &value);
		file = g_file_new_for_path(g_value_get_string(&value));
		bname = g_file_get_basename(file);
		gtk_entry_set_text(priv->rename_entry, bname);
		if (gtk_dialog_run(priv->rename_dialog) == GTK_RESPONSE_OK) {
			rename_file = g_file_set_display_name(file,
					gtk_entry_get_text(priv->rename_entry), NULL, NULL);
			if (rename_file != NULL) {
				g_free(bname);
				bname = g_file_get_basename(rename_file);
				save_path = g_file_get_path(rename_file);
				gtk_list_store_set(priv->download_store, &iter, col_path,
						save_path, col_name, bname, -1);
				g_free(save_path);
				g_object_unref(rename_file);
			}
		}
		g_free(bname);
		g_object_unref(file);
		g_value_unset(&value);
		if (selec_list->next == NULL) {
			g_list_free_full(selec_list, (GDestroyNotify) gtk_tree_path_free);
			selec_list = NULL;
		} else {
			selec_list = selec_list->next;
		}
	}
	g_mutex_unlock(&priv->store_mutex);
}

void menu_delete(GtkMenuItem *menuitem, MyDownload *MyDownload) {
	MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
	GtkTreeSelection *selec = gtk_tree_view_get_selection(priv->view);
	GtkTreePath *tree_path;
	GtkTreeIter iter;
	GValue value = G_VALUE_INIT;
	WebKitDownload *download;
	GList *row_ref = NULL;
	g_mutex_lock(&priv->store_mutex);
	GList *selec_list = gtk_tree_selection_get_selected_rows(selec,
			&priv->download_store);
	while (selec_list != NULL) {
		row_ref = g_list_append(row_ref,
				gtk_tree_row_reference_new(priv->download_store,
						selec_list->data));
		selec_list = g_list_next_with_free(selec_list, gtk_tree_path_free);
	}
	g_mutex_unlock(&priv->store_mutex);
	while (row_ref != NULL) {
		tree_path = gtk_tree_row_reference_get_path(row_ref->data);
		gtk_tree_model_get_iter(priv->download_store, &iter, tree_path);
		gtk_tree_model_get_value(priv->download_store, &iter,
				col_webkitdownload, &value);
		download = g_value_get_pointer(&value);
		if (download != NULL) {
			webkit_download_cancel(download);
			g_object_unref(download);
		}
		gtk_list_store_remove(priv->download_store, &iter);
		g_value_unset(&value);
		row_ref = g_list_next_with_free(row_ref, gtk_tree_row_reference_free);
	}
}
;

void menu_delete_with_file(GtkMenuItem *menuitem, MyDownload *MyDownload)

{
	MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
	GtkTreeSelection *selec = gtk_tree_view_get_selection(priv->view);
	GtkTreePath *tree_path;
	GtkTreeIter iter;
	GValue value = G_VALUE_INIT;
	WebKitDownload *download;
	GList *row_ref = NULL;
	g_mutex_lock(&priv->store_mutex);
	GList *selec_list = gtk_tree_selection_get_selected_rows(selec,
			&priv->download_store);
	while (selec_list != NULL) {
		row_ref = g_list_append(row_ref,
				gtk_tree_row_reference_new(priv->download_store,
						selec_list->data));
		selec_list = g_list_next_with_free(selec_list, gtk_tree_path_free);
	}
	g_mutex_unlock(&priv->store_mutex);
	while (row_ref != NULL) {
		tree_path = gtk_tree_row_reference_get_path(row_ref->data);
		gtk_tree_model_get_iter(priv->download_store, &iter, tree_path);
		gtk_tree_model_get_value(priv->download_store, &iter,
				col_webkitdownload, &value);
		download = g_value_get_pointer(&value);
		if (download != NULL) {
			webkit_download_cancel(download);
			g_object_unref(download);
		}
		g_value_unset(&value);
		gtk_tree_model_get_value(priv->download_store, &iter, col_path, &value);
		g_unlink(g_value_get_string(&value));
		gtk_list_store_remove(priv->download_store, &iter);
		g_value_unset(&value);
		row_ref = g_list_next_with_free(row_ref, gtk_tree_row_reference_free);
	}
}
;

/*{
 MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
 GtkTreeSelection *selec = gtk_tree_view_get_selection(priv->view);
 GList *selec_list = gtk_tree_selection_get_selected_rows(selec,
 &priv->download_store);
 g_mutex_lock(&priv->store_mutex);
 GtkTreePath *tree_path;
 GtkTreeIter iter;
 GValue value = G_VALUE_INIT;
 GValue path_value = G_VALUE_INIT;
 WebKitDownload *download;
 while (selec_list != NULL) {
 tree_path = selec_list->data;
 gtk_tree_model_get_iter(priv->download_store, &iter, tree_path);
 gtk_tree_model_get_value(priv->download_store, &iter,
 col_webkitdownload, &value);
 gtk_tree_model_get_value(priv->download_store, &iter, col_path,
 &path_value);
 download = g_value_get_pointer(&value);
 if (download != NULL) {
 webkit_download_cancel(download);
 g_object_unref(download);
 }
 g_unlink(g_value_get_string(&path_value));
 gtk_tree_model_row_deleted(priv->download_store, tree_path);
 g_value_unset(&value);
 g_value_unset(&path_value);
 selec_list = g_list_next_with_free(selec_list,gtk_tree_path_free);
 g_value_unset(&value);
 }
 g_mutex_unlock(&priv->store_mutex);

 }
 ;*/

void menu_copy_link(GtkMenuItem *menuitem, MyDownload *MyDownload) {
	{
		MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
		GtkTreeSelection *selec = gtk_tree_view_get_selection(priv->view);
		GList *selec_list = gtk_tree_selection_get_selected_rows(selec,
				&priv->download_store);
		GtkTreePath *path;
		GtkTreeIter iter;
		GValue value = G_VALUE_INIT;
		GtkClipboard *clipboard = gtk_clipboard_get_default(
				gdk_display_get_default());
		GString *str = g_string_new("");
		while (selec_list != NULL) {
			path = selec_list->data;
			if (gtk_tree_model_get_iter(priv->download_store, &iter, path)) {
				gtk_tree_model_get_value(priv->download_store, &iter, col_url,
						&value);
				g_string_append(str, g_value_get_string(&value));
				g_value_unset(&value);
			};
			if (selec_list->next == NULL) {
				g_list_free_full(selec_list,
						(GDestroyNotify) gtk_tree_path_free);
				selec_list = NULL;
			} else {
				g_string_append(str, "\n");
				selec_list = selec_list->next;
			}
		}
		gtk_clipboard_set_text(clipboard, str->str, -1);
		g_string_free(str, TRUE);
	};
}
;

gchar * remove_new_line_char(gchar *str) {
	gchar *temp, *temp2;
	temp = g_strrstr(str, "\n");
	if (temp != NULL) {
		temp2 = g_strndup(str, temp - str);
	} else {
		temp2 = g_strdup(str);
	}
	return temp2;
}

void my_download_add_button_clicked(GtkToolButton *toolbutton,
		MyDownload *MyDownload) {
	gchar *text, *temp, *url = NULL, *save_dir = NULL;
	GtkTextIter start, end;
	GFile *file, *backup_file;
	GInputStream *in;
	gulong add_count, pre_count;
	text = g_strdup("");
	MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
	gtk_text_buffer_set_text(priv->add_uri_buffer, text, -1);
	gtk_file_chooser_set_filename(priv->add_from_backup_file, priv->save_dir);
	g_free(text);
	if (gtk_dialog_run(priv->add_dialog) == GTK_RESPONSE_OK) {
		gtk_text_buffer_get_start_iter(priv->add_uri_buffer, &start);
		gtk_text_buffer_get_start_iter(priv->add_uri_buffer, &end);
		while (gtk_text_iter_forward_line(&end) == TRUE) { //获取N-1行URL
			temp = gtk_text_buffer_get_text(priv->add_uri_buffer, &start, &end,
			FALSE);
			text = remove_new_line_char(temp);
			my_download_add(MyDownload, text, NULL);
			gtk_text_iter_forward_line(&start);
			g_free(temp);
			g_free(text);
		};
		gtk_text_buffer_get_end_iter(priv->add_uri_buffer, &end); //获取第N行URL
		temp = gtk_text_buffer_get_text(priv->add_uri_buffer, &start, &end,
		TRUE);
		text = remove_new_line_char(temp);
		my_download_add(MyDownload, text, NULL);
		gtk_text_iter_forward_line(&start);
		g_free(temp);
		g_free(text);

		temp = g_strdup_printf("%s%s%s.bak", priv->save_dir, G_DIR_SEPARATOR_S,
				priv->name);
		backup_file = g_file_new_for_path(temp);
		g_free(temp);
		file = gtk_file_chooser_get_file(priv->add_from_backup_file);
		temp = g_file_get_path(file);
		text = g_file_get_path(backup_file);
		in = g_file_read(file, NULL, NULL);
		if (in != NULL && g_strcmp0(temp, text) != 0) {
			add_count = 0;
			do {
				pre_count = add_count;
				g_free(url);
				g_free(save_dir);
				read_from_file(in, MY_TYPE_STRING, &url, MY_TYPE_STRING,
						&save_dir, MY_TYPE_UINT, &add_count, MY_TYPE_NONE);
				my_download_add(MyDownload, url, save_dir);
			} while (add_count > pre_count);
			g_free(url);
			g_free(save_dir);
		}
		g_free(temp);
		g_free(text);
	}

}
;
void my_download_set_button_clicked(GtkToolButton *toolbutton,
		MyDownload *MyDownload) {
	gchar *save_temp;
	GFile *save_dir, *work_place;
	MyDownloadPrivate *priv = my_download_get_instance_private(MyDownload);
	gtk_entry_set_text(priv->prefix_entry, priv->perfix);
	gtk_entry_set_text(priv->suffix_entry, priv->suffix);
	gtk_file_chooser_set_current_folder(priv->save_location, priv->save_dir);
	switch (priv->same_filename_operation) {
	case same_filename_rename_add_suffix:
		gtk_toggle_button_set_active(priv->rename_suffix_number, TRUE);
		break;
	case same_filename_over_write:
		gtk_toggle_button_set_active(priv->over_write, TRUE);
		break;
	default:
		gtk_toggle_button_set_active(priv->skip_download, TRUE);
		break;
	};
	if (gtk_dialog_run(priv->set_dialog) == GTK_RESPONSE_OK) {
		g_free(priv->perfix);
		g_free(priv->suffix);
		priv->perfix = g_strdup(gtk_entry_get_text(priv->prefix_entry));
		priv->suffix = g_strdup(gtk_entry_get_text(priv->suffix_entry));
		save_dir = gtk_file_chooser_get_file(priv->save_location);
		work_place = g_file_new_for_path("./");
		save_temp = g_file_get_relative_path(work_place, save_dir);
		if (save_temp != NULL) {
			g_free(priv->save_dir);
			priv->save_dir = save_temp;
		}
		g_object_unref(save_dir);
		g_object_unref(work_place);
		priv->same_filename_operation = same_filename_rename_add_suffix;
		if (gtk_toggle_button_get_active(priv->over_write))
			priv->same_filename_operation = same_filename_over_write;
		if (gtk_toggle_button_get_active(priv->skip_download))
			priv->same_filename_operation = same_filename_skip;
		//g_print("perfix:%s\nsuffix:%s\nsave location:%s\n",priv->perfix,priv->suffix,priv->save_dir);
	};
}
;

MyDownload * my_download_new(WebKitWebView *web_view, gchar *save_dir,
		gchar *prefix, gchar *suffix) {
	MyDownload *self;
	gchar *dir, *temp_time;
	GDateTime *time;
	if (WEBKIT_IS_WEB_VIEW(web_view)) {
		self = g_object_new(MY_TYPE_DOWNLOAD, NULL);
		MyDownloadPrivate *priv = my_download_get_instance_private(self);
		priv->web_view = web_view;
		if (save_dir != NULL) {
			priv->save_dir = g_strdup(save_dir);
		} else {
			time = g_date_time_new_now_local();
			temp_time = g_date_time_format(time, "%Y-%m-%d_%H%M%S");
			priv->save_dir = g_strdup_printf(".%c%s", G_DIR_SEPARATOR,
					temp_time);
			g_free(temp_time);
			g_date_time_unref(time);
		}
		if (prefix != NULL) {
			priv->perfix = g_strdup(prefix);
		} else {
			priv->perfix = g_strdup("");
		};
		if (suffix != NULL) {
			priv->suffix = g_strdup(suffix);
		} else {
			priv->suffix = g_strdup("");
		};
	} else {
		g_printerr("my_download_new():web_view is in wrong type");
		self = NULL;
	}
	return self;
}

gchar *webkit_download_name_fmt(const gchar *fmt, const gchar *ori_name,
		const gchar *uri, const gchar *title) {
	gboolean u = FALSE, w = FALSE, f = FALSE, t = FALSE, d = FALSE;
	gchar *name = g_strdup(fmt), *temp,*temp0, *time, *date,**uri_v;
	GDateTime *date_time;
	if (g_strstr_len(fmt, -1, "%f") != NULL)
		f = TRUE;
	if (g_strstr_len(fmt, -1, "%u") != NULL)
		u = TRUE;
	if (g_strstr_len(fmt, -1, "%w") != NULL)
		w = TRUE;
	if (g_strstr_len(fmt, -1, "%t") != NULL)
		t = TRUE;
	if (g_strstr_len(fmt, -1, "%d") != NULL)
		d = TRUE;

	if (f) {
		temp = g_regex_replace(ori_regex, name, -1, 0, ori_name, 0, NULL);
		g_free(name);
		name = temp;
	}
	if (u) {
		uri_v=g_strsplit(uri,"/",-1);
		temp0=g_strjoinv("_",uri_v);
		temp = g_regex_replace(uri_regex, name, -1, 0, temp0, 0, NULL);
		g_free(name);
		name = temp;
		g_free(temp0);
		g_strfreev(uri_v);
	}
	if (w) {
		temp = g_regex_replace(title_regex, name, -1, 0, title, 0, NULL);
		g_free(name);
		name = temp;
	}
	if (t || d) {
		date_time = g_date_time_new_now_local();
		date = g_date_time_format(date_time, "%Y-%m-%d");
		time = g_date_time_format(date_time, "%H:%M:%S");
		g_date_time_unref(date_time);
		temp = g_regex_replace(date_regex, name, -1, 0, date, 0, NULL);
		g_free(name);
		name = g_regex_replace(time_regex, temp, -1, 0, time, 0, NULL);
		g_free(temp);
		g_free(date);
		g_free(time);
	}
	return name;
}

gboolean webkit_download_decide_destination(WebKitDownload *download,
		gchar *suggested_filename, gpointer user_data) {
	guint i = 0, suffix_num = 0;
	;
	GFile *file;
	gchar *name, **name_v, *path;
	GString *str;
	gboolean download_stop = FALSE;
	if (g_strcmp0(suggested_filename, "")
			== 0||g_strcmp0(suggested_filename,NULL)==NULL) {
		//建议名为空，以下载的URL命名下载文件
		WebKitURIRequest *res = webkit_download_get_request(download);
		name = webkit_uri_request_get_uri(res);
		if (name != NULL) {
			name_v = g_strsplit(name, "/", -1);
			name = g_strjoinv("_",name_v);
			g_strfreev(name_v);
		} else {
			name = g_strdup("\"\"");
		}
	} else {
		file = g_file_new_for_uri(suggested_filename);
		name = g_file_get_basename(file);
		g_object_unref(file);
	}

	GtkTreeRowReference *row_ref = g_object_get_data(download, "row_ref");
	MyDownload *my_download = g_object_get_data(download, "my_download");
	MyDownloadPrivate *priv = my_download_get_instance_private(my_download);

//重定义文件名
	gchar *s_dir = NULL;
	gchar *s_title = NULL;
	gchar *name_format = NULL;
	gchar *fmt_name = NULL;
	gchar *uri = webkit_uri_response_get_uri(
			webkit_download_get_response(download));
	s_dir = g_object_get_data(download, "path");
	s_title = g_object_get_data(download, "s_title");
	if (s_title == NULL)
		s_title = g_strdup("");
	if (gtk_toggle_button_get_active(priv->special_name_format)) {
		name_format = g_strdup(gtk_entry_get_text(priv->name_format));
		if (name_format == NULL || g_strcmp0("", name_format)==0) {
			g_free(name_format);
			name_format = g_strdup("%f");
		}
		fmt_name = webkit_download_name_fmt(name_format, name, uri, s_title);
		g_free(name_format);
	} else {
		fmt_name = g_strdup(name);
	}

	gchar *file_name = g_strdup_printf("%s%c%s", s_dir, G_DIR_SEPARATOR,
			fmt_name);

	if (g_access(file_name, 00) == 0) {
		//有同名文件存在，选择处理方法
		switch (priv->same_filename_operation) {
		case same_filename_over_write: //覆盖同名文件
			webkit_download_set_allow_overwrite(download, TRUE);
			break;
		case same_filename_skip: //跳过下载
			download_stop = TRUE;
			webkit_download_cancel(download);
			break;
		default: //更名，名字添加后续数字
			do {
				g_free(file_name);
				file_name = g_strdup_printf("%s%c%s.%.2u", s_dir,
						G_DIR_SEPARATOR, fmt_name, suffix_num);
				suffix_num++;
			} while (g_access(file_name, 00) == 0);
			break;
		};
	};
	g_free(fmt_name);
	g_free(s_title);

//检测下载目录是否存在
	file = g_file_new_for_path(file_name);
	GFile *dir = g_file_get_parent(file);
	g_free(file_name);
	path = g_file_get_path(dir); //new_name临时储存保存路径字符串
	if (g_file_query_exists(dir, NULL) == FALSE)
		g_mkdir_with_parents(path, 0777);
	g_object_unref(dir);
	g_free(path);
//以新的下载文件名重定Webkit下载路径
	name = g_file_get_uri(file);
	webkit_download_set_destination(download, name);
	g_free(name);
//把下载信息添加至列表
	name = g_file_get_basename(file);
	g_mutex_lock(&priv->store_mutex);
	GtkTreePath *tree_path = gtk_tree_row_reference_get_path(row_ref);
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter(priv->download_store, &iter, tree_path)) {
		gtk_list_store_set(priv->download_store, &iter, col_name, name, -1);
	}
	g_free(name);
	if (s_dir != NULL)
		g_free(s_dir);
	g_object_unref(file);
	//gtk_tree_path_free(tree_path);
	g_mutex_unlock(&priv->store_mutex);
	return download_stop;
}

void webkit_download_created_destination(WebKitDownload *download,
		gchar *destination, gpointer user_data) {
	GtkTreeRowReference *row_ref = g_object_get_data(download, "row_ref");
	MyDownload *my_download = g_object_get_data(download, "my_download");
	MyDownloadPrivate *priv = my_download_get_instance_private(my_download);
	g_mutex_lock(&priv->store_mutex);
	GtkTreePath *path = gtk_tree_row_reference_get_path(row_ref);
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	GFile *file = g_file_new_for_uri(destination);
	gchar *save_path = g_file_get_path(file);
	if (gtk_tree_model_get_iter(priv->download_store, &iter, path)) {
		pixbuf = gtk_image_get_pixbuf(priv->downloading);
		gtk_list_store_set(priv->download_store, &iter, col_state,
				MY_DOWNLOAD_DOWNLOADING, col_state_pixfuf, pixbuf, col_path,
				save_path, -1);
	}
	g_free(save_path);
	g_object_unref(file);
	gtk_tree_path_free(path);
	g_mutex_unlock(&priv->store_mutex);
	g_signal_emit_by_name(my_download, "download_start", download,
			priv->download_store, row_ref, NULL);
}

void webkit_download_finish(WebKitDownload *download, gpointer user_data) {
	GtkTreeRowReference *row_ref = g_object_get_data(download, "row_ref");
	MyDownload *my_download = g_object_get_data(download, "my_download");
	MyDownloadPrivate *priv = my_download_get_instance_private(my_download);
	g_mutex_lock(&priv->store_mutex);
	GtkTreePath *path = gtk_tree_row_reference_get_path(row_ref);
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter(priv->download_store, &iter, path)) {
		pixbuf = gtk_image_get_pixbuf(priv->finish);
		gtk_list_store_set(priv->download_store, &iter, col_state,
				MY_DOWNLOAD_FINISH, col_state_pixfuf, pixbuf, col_progress, 100,
				col_speed, 0, col_speed_text, "", col_webkitdownload, NULL, -1);
	}
	g_mutex_unlock(&priv->store_mutex);
	g_signal_emit_by_name(my_download, "download_finish", download,
			priv->download_store, row_ref, NULL);
	gtk_tree_row_reference_free(row_ref);
	gtk_tree_path_free(path);
	my_download_count_modify(my_download, -1);
}

void webkit_download_fail(WebKitDownload *download, GError *error,
		gpointer user_data) {
	GtkTreeRowReference *row_ref = g_object_get_data(download, "row_ref");
	MyDownload *my_download = g_object_get_data(download, "my_download");
	MyDownloadPrivate *priv = my_download_get_instance_private(my_download);
	g_mutex_lock(&priv->store_mutex);
	GtkTreePath *path = gtk_tree_row_reference_get_path(row_ref);
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter(priv->download_store, &iter, path)) {
		if (error->code == WEBKIT_DOWNLOAD_ERROR_CANCELLED_BY_USER) {
			pixbuf = gtk_image_get_pixbuf(priv->stop);
		} else {
			pixbuf = gtk_image_get_pixbuf(priv->error);
		}
		gtk_list_store_set(priv->download_store, &iter, col_state,
				MY_DOWNLOAD_ERROR, col_state_pixfuf, pixbuf, col_webkitdownload,
				NULL, -1);
		g_signal_handlers_disconnect_by_func(download, webkit_download_finish,
				NULL);
	}
	gtk_tree_path_free(path);
	g_mutex_unlock(&priv->store_mutex);
	g_object_unref(download);
	gtk_tree_row_reference_free(row_ref);
	my_download_count_modify(my_download, -1);
}
void webkit_download_received_data(WebKitDownload *download,
		guint64 data_length, gpointer user_data) {
	GtkTreeRowReference *row_ref = g_object_get_data(download, "row_ref");
	MyDownload *my_download = g_object_get_data(download, "my_download");
	MyDownloadPrivate *priv = my_download_get_instance_private(my_download);
	g_mutex_lock(&priv->store_mutex);
	GtkTreePath *path = gtk_tree_row_reference_get_path(row_ref);
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;

	gdouble elapsed_time = webkit_download_get_elapsed_time(download);
	GDateTime *elapsed_time_temp = g_date_time_new_from_unix_utc(
			(gint64) elapsed_time);
	gchar *elapsed_time_text = g_date_time_format(elapsed_time_temp,
			"%H:%M:%S");
	g_date_time_unref(elapsed_time_temp);

	gdouble progress = webkit_download_get_estimated_progress(download);
	guint s_unit = 0;
	guint64 size = webkit_download_get_received_data_length(download)
			/ progress;
	while (size > 1024) {
		size = size / 1024;
		s_unit++;
	}
	gchar *size_text = g_strdup_printf("%lu %s", (gulong) size,
			size_unit[s_unit]);
	guint speed = webkit_download_get_received_data_length(download)
			/ elapsed_time;
	s_unit = 0;
	while (speed >= 1024) {
		speed = speed / 1024;
		s_unit++;
	}
	gchar *speed_text = g_strdup_printf("%u %s/s", speed, size_unit[s_unit]);

	if (gtk_tree_model_get_iter(priv->download_store, &iter, path)) {
		pixbuf = gtk_image_get_pixbuf(priv->downloading);
		gtk_list_store_set(priv->download_store, &iter, col_state,
				MY_DOWNLOAD_DOWNLOADING, col_state_pixfuf, pixbuf, col_progress,
				(gint) (progress * 100.), col_time_elapsed,
				(guint) elapsed_time, col_time_elapsed_text, elapsed_time_text,
				col_speed, (guint) (data_length / elapsed_time), col_speed_text,
				speed_text, col_size, (gulong) size, col_size_text, size_text,
				-1);
	}
	gtk_tree_path_free(path);
	g_mutex_unlock(&priv->store_mutex);
	g_free(size_text);
	g_free(speed_text);
	g_free(elapsed_time_text);
}

DownloadQueueData *my_download_queue_data_new(gchar *url, gchar *dir,
		GtkTreeRowReference *ref) {
	DownloadQueueData *data = g_malloc0(sizeof(DownloadQueueData));
	data->dir = g_strdup(dir);
	data->ref = ref;
	data->url = g_strdup(url);
	return data;
}
;
void my_download_queue_data_free(DownloadQueueData *data) {
	g_free(data->dir);
	//if(data->ref!=NULL)gtk_tree_row_reference_free(data->ref);
	g_free(data->url);

}
;

void my_download_queue_push(MyDownload *self, gchar *url, gchar *dir,
		GtkTreeRowReference *ref) {
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	DownloadQueueData *data = my_download_queue_data_new(url, dir, ref);
	g_async_queue_push(priv->download_queue, data);
	while (priv->count < priv->max_count
			&& g_async_queue_length(priv->download_queue) > 0) {
		my_download_queue_pop(self);
	}
}
;

void my_download_queue_pop(MyDownload *self) {
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	DownloadQueueData *data = g_async_queue_pop(priv->download_queue);
	WebKitDownload *download = webkit_web_view_download_uri(priv->web_view,
			data->url);
	GtkTreeIter iter;
	GtkTreePath *path;

	g_mutex_lock(&priv->store_mutex);
	path = gtk_tree_row_reference_get_path(data->ref);
	gtk_tree_model_get_iter(priv->download_store, &iter, path);
	gtk_tree_path_free(path);
	gtk_list_store_set(priv->download_store, &iter, col_webkitdownload,
			download, -1);
	g_mutex_unlock(&priv->store_mutex);

	g_object_set_data(download, "row_ref", data->ref);
	g_object_set_data(download, "my_download", self);
	if (data->dir != NULL)
		g_object_set_data(download, "path", g_strdup(data->dir));
	else
		g_object_set_data(download, "path", g_strdup(priv->save_dir));
	//上一级任务特定保存目录
	g_signal_connect(download, "decide-destination",
			webkit_download_decide_destination, NULL);
	g_signal_connect(download, "created-destination",
			webkit_download_created_destination, NULL);
	g_signal_connect(download, "received-data", webkit_download_received_data,
			NULL);
	g_signal_connect(download, "failed", webkit_download_fail, NULL);
	g_signal_connect(download, "finished", webkit_download_finish, NULL);
	priv->count++;
	g_object_notify(self, "count");
	my_download_queue_data_free(data);
}
;

void my_download_add(MyDownload *self, gchar *url, gchar *dir) {
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeRowReference *row_ref;
	GdkPixbuf *buf;
	gchar *size_text, *speed_text, *time_elapsed_text, *time_started_text,
			*temp;
	int s_unit;
	gulong s_temp;
	Download_State state;
	GOutputStream *out;
	if (g_strcmp0(url, "\n") == 0 || g_strcmp0(url, "") == 0)
		return; //url地址为空

	if (gtk_toggle_button_get_active(priv->skip_same_url)) { //是否要跳过重复下载的项目
		if (g_strstr_len(priv->downloaded_url, -1, url) != NULL) { //已有下载任务放弃
			return;
		} else { //加入已下载任务
			g_string_append(priv->downloaded_url, url);
		};
	}
	if (gtk_toggle_button_get_active(priv->auto_backup)) {
		g_mkdir(priv->save_dir, 0777);
		temp = g_strdup_printf("%s%s%s.bak", priv->save_dir, G_DIR_SEPARATOR_S,
				priv->name);
		GFile *bufile = g_file_new_for_path(temp);
		if (g_file_query_exists(bufile, NULL)) {
			out = g_file_append_to(bufile, G_FILE_CREATE_REPLACE_DESTINATION,
					NULL, NULL);
		} else {
			out = g_file_create(bufile, G_FILE_CREATE_REPLACE_DESTINATION, NULL,
					NULL);
		}
		if (out != NULL) {
			write_to_file(out, MY_TYPE_STRING, url, MY_TYPE_STRING, dir,
					MY_TYPE_UINT, &priv->add_count, MY_TYPE_NONE);
			priv->add_count++;
		}
		g_output_stream_close(out, NULL, NULL);
		g_object_unref(out);
		g_object_unref(bufile);
		g_free(temp);
	}
	state.name = strdup("");
	state.state = MY_DOWNLOAD_WAIT;
	state.url = strdup(url);
	buf = gtk_image_get_pixbuf(priv->wait);
	GDateTime *datetime = g_date_time_new_now_local();
	speed_text = g_strdup("0 byte/s");
	time_elapsed_text = g_strdup("0s");
	time_started_text = g_date_time_format(datetime, "%Y-%m-%d %H:%M:%S");
	g_mutex_lock(&priv->store_mutex);

	gtk_list_store_append(priv->download_store, &iter);
	if (gtk_list_store_iter_is_valid(priv->download_store, &iter)) {
		gtk_list_store_set(priv->download_store, &iter, col_name, state.name,
				col_progress, 0, col_size, 0, col_size_text, "0 byte",
				col_speed, 0, col_speed_text, speed_text, col_state_pixfuf, buf,
				col_time_elapsed, 0, col_time_elapsed_text, time_elapsed_text,
				col_time_start, g_date_time_to_unix(datetime),
				col_time_started_text, time_started_text, col_url, url,
				col_state, MY_DOWNLOAD_WAIT, col_path, dir, -1);
	} else {
		g_printerr("Iter is invalid!!\n");
	}
	//g_free(size_text);
	g_free(speed_text);
	g_free(time_elapsed_text);
	g_free(time_started_text);
	g_date_time_unref(datetime);
	path = gtk_tree_model_get_path(priv->download_store, &iter);
	row_ref = gtk_tree_row_reference_new(priv->download_store, path);
	gtk_tree_path_free(path);
	g_mutex_unlock(&priv->store_mutex);
	my_download_queue_push(self, url, dir, row_ref);
}
;

void my_download_add_webkitdownload(MyDownload *self, WebKitDownload *download) {
	GtkTreeIter iter;
	Download_State state;
	GDateTime *datetime = g_date_time_new_now_local();
	gchar *time_started_text = g_date_time_format(datetime,
			"%Y-%m-%d %H:%M:%S");
	gchar *url = webkit_uri_request_get_uri(
			webkit_download_get_request(download));
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	gtk_list_store_append(priv->download_store, &iter);
	GtkTreePath *path = gtk_tree_model_get_path(priv->download_store, &iter);
	GtkTreeRowReference *row_ref = gtk_tree_row_reference_new(
			priv->download_store, path);
	gchar *title = webkit_web_view_get_title(
			webkit_download_get_web_view(download));
	g_object_set_data(download, "my_download", self);
	g_object_set_data(download, "row_ref", row_ref);
	if (title != NULL)
		g_object_set_data(download, "s_title", g_strdup(title)); //上一级任务特定前续
	gtk_list_store_set(priv->download_store, &iter, col_name, "", col_progress,
			0, col_size, 0, col_size_text, "0 byte", col_speed, 0,
			col_speed_text, "0 byte/s", col_state_pixfuf,
			gtk_image_get_pixbuf(priv->downloading), col_time_elapsed, 0,
			col_time_elapsed_text, "0 s", col_time_start,
			g_date_time_to_unix(datetime), col_time_started_text,
			time_started_text, col_url, url, col_state, MY_DOWNLOAD_WAIT,
			col_s_title, title, -1);
	g_signal_connect(download, "decide-destination",
			webkit_download_decide_destination, NULL);
	g_signal_connect(download, "created-destination",
			webkit_download_created_destination, NULL);
	g_signal_connect(download, "received-data", webkit_download_received_data,
			NULL);
	g_signal_connect(download, "failed", webkit_download_fail, NULL);
	g_signal_connect(download, "finished", webkit_download_finish, NULL);
	g_free(time_started_text);
	g_date_time_unref(datetime);
}
;

void my_download_set(MyDownload *self, gchar *save_dir, gchar *prefix,
		gchar *suffix) {
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	GFile *dir = g_file_new_for_path(save_dir);
	if (g_file_query_exists(dir, NULL)) {
		priv->save_dir = g_file_get_path(dir);
	}
	g_object_unref(dir);
	if (prefix != NULL) {
		g_free(priv->perfix);
		priv->perfix = g_strdup(prefix);
	}
	if (suffix != NULL) {
		g_free(priv->suffix);
		priv->suffix = g_strdup(suffix);
	}
}
;

Download_State *my_download_get_download_state(MyDownload *self,
		WebKitDownload *download) {
	Download_State *state = NULL;
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	GtkTreeRowReference *row_ref = g_object_get_data(download, "row_ref");
	GtkTreePath *tree_path = gtk_tree_row_reference_get_path(row_ref);
	GtkTreeIter iter;
	GValue value = G_VALUE_INIT;
	if (gtk_tree_model_get_iter(priv->download_store, &iter, tree_path)) {
		state = g_malloc(sizeof(Download_State));
		gtk_tree_model_get_value(priv->download_store, &iter, col_name, &value);
		state->name = g_strdup(g_value_get_string(&value));
		g_value_unset(&value);

		gtk_tree_model_get_value(priv->download_store, &iter, col_url, &value);
		state->url = g_strdup(g_value_get_string(&value));
		g_value_unset(&value);

		gtk_tree_model_get_value(priv->download_store, &iter, col_progress,
				&value);
		state->progress = g_value_get_uint(&value);
		g_value_unset(&value);

		gtk_tree_model_get_value(priv->download_store, &iter, col_time_elapsed,
				&value);
		state->time_elapsed = g_value_get_uint(&value);
		g_value_unset(&value);

		gtk_tree_model_get_value(priv->download_store, &iter, col_state,
				&value);
		state->state = g_value_get_uchar(&value);
		g_value_unset(&value);

		gtk_tree_model_get_value(priv->download_store, &iter, col_time_start,
				&value);
		state->start_time = g_date_time_new_from_unix_local(
				g_value_get_int64(&value));
		g_value_unset(&value);

		gtk_tree_model_get_value(priv->download_store, &iter, col_size, &value);
		state->size = g_value_get_ulong(&value);
		g_value_unset(&value);
	};
	gtk_tree_path_free(tree_path);

	return state;
}
;

WebKitWebView *my_download_get_web_view(MyDownload *self) {
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	return priv->web_view;
}
;

void my_download_stop_all(MyDownload *self) {
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(priv->view);
	gtk_tree_selection_select_all(selection);
	menu_stop(NULL, self);
	gtk_tree_selection_unselect_all(selection);
}
;
void my_download_start_all(MyDownload *self) {
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	GtkTreeSelection *selection = gtk_tree_view_get_selection(priv->view);
	gtk_tree_selection_select_all(selection);
	menu_continue(NULL, self);
	gtk_tree_selection_unselect_all(selection);
}
;

MyDownloadSetting *my_download_get_setting(MyDownload *self) {
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	MyDownloadSetting *set = g_malloc(sizeof(MyDownloadSetting));
	set->global_prefix = g_strdup(priv->perfix);
	set->global_suffix = g_strdup(priv->suffix);
	set->same_op = priv->same_filename_operation;
	set->save_local = g_strdup(priv->save_dir);
	set->u_special_name_format = gtk_toggle_button_get_active(
			priv->special_name_format);
	set->auto_backup = gtk_toggle_button_get_active(priv->auto_backup);
	set->skip_same_url = gtk_toggle_button_get_active(priv->skip_same_url);
	set->name_format = g_strdup(gtk_entry_get_text(priv->name_format));
	return set;
}
;

void my_download_set_setting(MyDownload *self, MyDownloadSetting *set) {
	MyDownloadPrivate *priv = my_download_get_instance_private(self);
	g_free(priv->perfix);
	g_free(priv->suffix);
	g_free(priv->save_dir);
	priv->perfix = g_strdup(set->global_prefix);
	priv->suffix = g_strdup(set->global_suffix);
	priv->save_dir = g_strdup(set->save_local);
	priv->same_filename_operation = set->same_op;
	gtk_entry_set_text(priv->name_format, set->name_format);
	gtk_toggle_button_set_active(priv->special_name_format,
			set->u_special_name_format);
	gtk_toggle_button_set_active(priv->auto_backup, set->auto_backup);
	gtk_toggle_button_set_active(priv->skip_same_url, set->skip_same_url);
}
;

void my_download_setting_free(MyDownloadSetting *set) {
	g_free(set->global_prefix);
	g_free(set->global_suffix);
	g_free(set->save_local);
	g_free(set->name_format);
	g_free(set);
}
;
