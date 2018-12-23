/*
 * MyLoader.c
 *
 *  Created on: 2018年5月6日
 *      Author: tom
 */

#include "MyLoader.h"

typedef struct {
	GtkImage *input_img, *output_img, *state_img;
	GtkLabel *label;
	MyLoaderSetting setting;
	GtkEventBox *input, *output;
	GtkNotebook *notebook;
	GtkLabel *status;
	GString *loaded_url;
	guint max_loader, source, runing_count;
	GAsyncQueue *msg_queue;
	WebKitSettings *view_setting;
	GtkSwitch *javascript, *auto_load_image, *media_stream, *smooth_scrolling,*plugins;
} MyLoaderPrivate;

G_DEFINE_TYPE_WITH_CODE(MyLoader, my_loader, MY_TYPE_TASK,
		G_ADD_PRIVATE(MyLoader));

void my_loader_finalize(MyLoader *self) {
	GList *list, *l;
	MyLoaderPrivate *priv = my_loader_get_instance_private(self);
	g_object_unref(priv->view_setting);
	list = gtk_container_get_children(priv->notebook);
	l = list;
	while (l != NULL) {
		if (WEBKIT_IS_WEB_VIEW(l->data))
			g_object_unref(l->data);
		l = l->next;
	}
	g_list_free(list);
	if (priv->loaded_url != NULL) {
		g_string_free(priv->loaded_url, TRUE);
		g_async_queue_unref(priv->msg_queue);
	}
	priv->loaded_url = NULL;
}
;

void my_loader_run_init(MyLoader *self);
void my_loader_run(MyLoader *loader, TaskMsg *msg);
void my_loader_save(MyLoader *self, GOutputStream *out);
void my_loader_load(MyLoader *self, GInputStream *in);
void my_loader_stop_run(MyLoader *self);

void
my_loader_clicked(GtkToolButton *toolbutton, MyLoader *self);

static void my_loader_class_init(MyLoaderClass *klass) {
	GObjectClass *obj_class = klass;
	MyTaskClass *task_class = klass;
	obj_class->finalize = my_loader_finalize;
	task_class->run_init = my_loader_run_init;
	task_class->run = my_loader_run;
	task_class->save = my_loader_save;
	task_class->load = my_loader_load;
	task_class->stop_run = my_loader_stop_run;
	gtk_widget_class_set_template_from_resource(klass,
			"/mytask/MyLoader.glade");
	gtk_widget_class_bind_template_child_private(klass, MyLoader, input_img);
	gtk_widget_class_bind_template_child_private(klass, MyLoader, output_img);
	gtk_widget_class_bind_template_child_private(klass, MyLoader, state_img);
	gtk_widget_class_bind_template_child_private(klass, MyLoader, label);
	gtk_widget_class_bind_template_child_private(klass, MyLoader, input);
	gtk_widget_class_bind_template_child_private(klass, MyLoader, output);
	gtk_widget_class_bind_template_child_private(klass, MyLoader, notebook);
	gtk_widget_class_bind_template_child_private(klass, MyLoader, status);
	gtk_widget_class_bind_template_child_private(klass, MyLoader,
			javascript);
	gtk_widget_class_bind_template_child_private(klass, MyLoader,
			auto_load_image);
	gtk_widget_class_bind_template_child_private(klass, MyLoader,
			media_stream);
	gtk_widget_class_bind_template_child_private(klass, MyLoader,
			smooth_scrolling);
	gtk_widget_class_bind_template_child_private(klass, MyLoader,
			plugins);
	gtk_widget_class_bind_template_callback(klass, my_loader_clicked);
}

static void my_loader_init(MyLoader *self) {
	static uint id = 0;
	gtk_widget_init_template(self);
	MyLoaderPrivate *priv = my_loader_get_instance_private(self);
	gtk_image_set_from_resource(priv->input_img, "/mytask/go-next.svg");
	gtk_image_set_from_resource(priv->output_img, "/mytask/network-transmit-receive-symbolic.svg");
	gtk_image_set_from_resource(priv->state_img,
			"/mytask/view-web-browser-dom-tree.svg");
	gtk_label_set_label(priv->label, "Loader");
	priv->setting.load_backstage = FALSE;
	priv->setting.timeout = 10.;
	priv->setting.skip_same_url = TRUE;
	priv->setting.max_count = 10;
	priv->loaded_url = NULL;
	priv->msg_queue = g_async_queue_new();
	priv->max_loader = 10;
	priv->source = 0;
	priv->runing_count = 0;
	g_object_get(self, MY_TASK_PROP_LOG, &priv->setting.log,
	MY_TASK_PROP_LOG_FILE, &priv->setting.log_file, NULL);
	my_task_rename_id(self, "Loader", id++);
	my_task_drag_dest_set(priv->input);
	my_task_drag_source_set(priv->output);
	my_task_set_right_clicked(priv->output, self);

	priv->view_setting = webkit_settings_new();
	//绑定Switch控件与webkit_settings属性
	g_object_bind_property(priv->view_setting,"enable-javascript",priv->javascript,"active",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(priv->view_setting,"enable-smooth-scrolling",priv->smooth_scrolling,"active",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(priv->view_setting,"enable-media-stream",priv->media_stream,"active",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(priv->view_setting,"auto-load-images",priv->auto_load_image,"active",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(priv->view_setting,"enable-plugins",priv->plugins,"active",G_BINDING_BIDIRECTIONAL);
	//优化webkit的载入速度
	webkit_settings_set_auto_load_images(priv->view_setting, FALSE);
	webkit_settings_set_enable_media_stream(priv->view_setting, FALSE);
	webkit_settings_set_load_icons_ignoring_image_load_setting(
			priv->view_setting, FALSE);
	webkit_settings_set_enable_smooth_scrolling(priv->view_setting, FALSE);
	webkit_settings_set_enable_plugins(priv->view_setting, FALSE);
}

gboolean my_loader_web_view_load_timeout(WebKitWebView *view) {
	if(!WEBKIT_IS_WEB_VIEW(view))return G_SOURCE_REMOVE;
	if(webkit_web_view_is_loading(view))webkit_web_view_stop_loading(view);
	return G_SOURCE_CONTINUE;
}
;
void my_loader_web_view_destroy(WebKitWebView *view,MyLoader *self){
	MyLoaderPrivate *priv = my_loader_get_instance_private(self);
	priv->runing_count--;
	runing_count--;
	g_object_ref(view);
	gtk_container_remove(priv->notebook,view);
}

typedef struct {
	gchar *url;
	MyLoader *loader;
	guint source_id;
} MyLoaderRunData;

void my_loader_view_load_changed(WebKitWebView *web_view,
		WebKitLoadEvent load_event, MyLoaderRunData *data) {
	MyTask *next_task;
	GList *list = NULL;
	MyLoaderPrivate *priv = my_loader_get_instance_private(data->loader);
	switch (load_event) {
	case WEBKIT_LOAD_STARTED:
		gtk_notebook_set_tab_label_text(priv->notebook, web_view,
				"Load Start ");
		break;
	case WEBKIT_LOAD_REDIRECTED:
		gtk_notebook_set_tab_label_text(priv->notebook, web_view,
				"Redirected ");
		break;
	case WEBKIT_LOAD_COMMITTED:
		gtk_notebook_set_tab_label_text(priv->notebook, web_view,
				"Loading... ");
		break;
	case WEBKIT_LOAD_FINISHED:
		gtk_notebook_set_tab_label_text(priv->notebook, web_view,
				"Load Finish");
		//g_source_remove(data->source_id);
		priv = my_loader_get_instance_private(data->loader);
		list = g_hash_table_lookup(link_table, priv->output);
		while (list != NULL) {
			my_task_send_msg(data->loader, list->data, web_view, NULL);
			list = list->next;
		}
		next_task = my_task_get_next_task(data->loader);
		if (next_task != NULL)
			my_task_send_msg(data->loader, next_task, web_view, NULL);
		g_object_unref(web_view);
		g_free(data->url);
		g_free(data);
		break;
	default:
		break;
	}
}
;

gboolean my_loader_thread(MyLoader *loader) {
	WebKitWebView *view;
	WebKitWebContext *ctx;
	gchar *str;
	MyLoaderRunData *data;
	MyLoaderPrivate *priv = my_loader_get_instance_private(loader);
	if (priv->runing_count >= priv->setting.max_count)
		return G_SOURCE_CONTINUE; //已到达工作上上限
	gchar *url = g_async_queue_try_pop(priv->msg_queue);
	if (url == NULL) { //没有工作作要处理处理
		if (priv->runing_count > 0) {
			str = g_strdup_printf("Loaded:%4u Queue:%4d", priv->runing_count,
					0);
			gtk_label_set_text(priv->label, str);
			g_free(str);
			return G_SOURCE_CONTINUE;
		} else {
			priv->source = 0;
			gtk_label_set_text(priv->label, "Loader");
			return G_SOURCE_REMOVE;
		}
	}
	//载入URL
		view = webkit_web_view_new();
		gtk_notebook_append_page(priv->notebook, view, NULL);
		gtk_notebook_set_tab_label_text(priv->notebook, view,
				"Load Start ");
		webkit_web_view_set_settings(view, priv->view_setting);
		gtk_widget_show_all(view);
		data = g_malloc(sizeof(MyLoaderRunData));
		data->loader = loader;
		g_signal_connect(view, "load_changed", my_loader_view_load_changed,
				data);
		g_signal_connect(view, "destroy", my_loader_web_view_destroy,
				loader);
	webkit_web_view_load_uri(view, url);
	data->url = url;
	data->source_id = g_timeout_add_seconds((guint) priv->setting.timeout,
			my_loader_web_view_load_timeout, view);
	priv->runing_count++;
	str = g_strdup_printf("Load:%4u Queue:%4d", priv->runing_count,
			g_async_queue_length(priv->msg_queue));
	gtk_label_set_text(priv->label, str);
	g_free(str);
	return G_SOURCE_CONTINUE;
}

void my_loader_run_init(MyLoader *self) {
	MyLoaderPrivate *priv = my_loader_get_instance_private(self);
	if (priv->loaded_url != NULL) {
		g_string_free(priv->loaded_url, TRUE);
	}
	priv->loaded_url = NULL;
}
;

void my_loader_run(MyLoader *loader, TaskMsg *msg) {
	gint i = 0;
	SoupURI *s_uri, *b_uri;
	MyLoaderPrivate *priv = my_loader_get_instance_private(loader);
	if (msg->msg == NULL){
		task_msg_free(msg);
		return;
	}
	gchar **strv = g_strsplit(msg->msg, "\n", -1);
	if (priv->loaded_url == NULL)
		priv->loaded_url = g_string_new("");
	my_task_log(loader, "<div><table><tr><th>Web Loader</th></tr>");
	b_uri = soup_uri_new(webkit_web_view_get_uri(msg->view));
	while (strv[i] != NULL) {
		if(g_strcmp0(strv[i],"")==0||g_strcmp0(strv[i],"\n")==0){
			i++;
			continue;
		}
		if (priv->setting.skip_same_url) {
			if (g_strstr_len(priv->loaded_url->str, -1, strv[i]) == NULL) {
				g_string_append(priv->loaded_url, strv[i]); //记录已处理过的URL
				my_task_log(loader, "<tr><td>%s</td></tr>", strv[i]);
			} else { //跳过已处理重复的URL
				my_task_log(loader, "<tr><td><b>!SKIP</b> %s</td></tr>",
						strv[i]);
				i++;
				continue;
			}
		}
		s_uri = soup_uri_new_with_base(b_uri, strv[i]);
		g_async_queue_push(priv->msg_queue, soup_uri_to_string(s_uri, FALSE));
		runing_count++;
		i++;
		soup_uri_free(s_uri);
	}
	my_task_log(loader, "</table></div>");
	if (priv->source == 0) {
		priv->source = g_idle_add(my_loader_thread, loader);
	}
	soup_uri_free(b_uri);
	g_strfreev(strv);
	task_msg_free(msg);
}
void my_loader_save(MyLoader *self, GOutputStream *out) {
	GType type = my_loader_get_type();
	gboolean javascript,media_stream,plugins,auto_load_image,smooth_scrolling;
	MyLoaderPrivate *priv = my_loader_get_instance_private(self);
	javascript=gtk_switch_get_active(priv->javascript);
	media_stream=gtk_switch_get_active(priv->media_stream);
	plugins=gtk_switch_get_active(priv->plugins);
	auto_load_image=gtk_switch_get_active(priv->auto_load_image);
	smooth_scrolling=gtk_switch_get_active(priv->smooth_scrolling);
	write_to_file(out, MY_TYPE_GTYPE, &type, MY_TYPE_BOOLEAN,
			&priv->setting.load_backstage, MY_TYPE_BOOLEAN, &priv->setting.log,
			MY_TYPE_STRING, priv->setting.log_file, MY_TYPE_UINT,
			&priv->setting.max_count, MY_TYPE_BOOLEAN,
			&priv->setting.skip_same_url, MY_TYPE_DOUBLE,
			&priv->setting.timeout, MY_TYPE_POINTER, &priv->input,
			MY_TYPE_POINTER, &priv->output, \
			MY_TYPE_BOOLEAN ,&javascript,\
			MY_TYPE_BOOLEAN,&media_stream,\
			MY_TYPE_BOOLEAN,&plugins,\
			MY_TYPE_BOOLEAN,&auto_load_image,\
			MY_TYPE_BOOLEAN,&smooth_scrolling,\
			MY_TYPE_NONE);
}
;
void my_loader_load(MyLoader *self, GInputStream *in) {
	gpointer input, output;
	gboolean javascript,media_stream,plugins,auto_load_image,smooth_scrolling;
	MyLoaderPrivate *priv = my_loader_get_instance_private(self);
	read_from_file(in, MY_TYPE_BOOLEAN, &priv->setting.load_backstage,
			MY_TYPE_BOOLEAN, &priv->setting.log, MY_TYPE_STRING,
			&priv->setting.log_file, MY_TYPE_UINT, &priv->setting.max_count,
			MY_TYPE_BOOLEAN, &priv->setting.skip_same_url, MY_TYPE_DOUBLE,
			&priv->setting.timeout, MY_TYPE_POINTER, &input, MY_TYPE_POINTER,
			&output,\
			MY_TYPE_BOOLEAN ,&javascript,\
			MY_TYPE_BOOLEAN,&media_stream,\
			MY_TYPE_BOOLEAN,&plugins,\
			MY_TYPE_BOOLEAN,&auto_load_image,\
			MY_TYPE_BOOLEAN,&smooth_scrolling,\
			MY_TYPE_NONE
	);
	gtk_switch_set_active(priv->auto_load_image,auto_load_image);
	gtk_switch_set_active(priv->javascript,javascript);
	gtk_switch_set_active(priv->media_stream,media_stream);
	gtk_switch_set_active(priv->plugins,plugins);
	gtk_switch_set_active(priv->smooth_scrolling,smooth_scrolling);
	g_hash_table_insert(load_table, input, priv->input);
	g_hash_table_insert(load_table, output, priv->output);
}
;

void my_loader_stop_run(MyLoader *self) {
	GList *list, *l;
	WebKitWebView *view;
	gchar *url;
	MyLoaderPrivate *priv = my_loader_get_instance_private(self);
	while (g_async_queue_length(priv->msg_queue) > 0) {
		url = g_async_queue_try_pop(priv->msg_queue);
		g_free(url);
		runing_count--;
	}
	list = gtk_container_get_children(priv->notebook);
	l = list;
	while (l != NULL) {
		if (WEBKIT_IS_WEB_VIEW(l->data)) {
			gtk_widget_destroy(l->data);
		}
		l = l->next;
	}
	g_list_free(list);
}
;

void my_loader_clicked(GtkToolButton *toolbutton, MyLoader *self) {
	MyLoaderPrivate *priv = my_loader_get_instance_private(self);
	MyLoaderSetDialog *dialog = my_loader_set_dialog_new(&priv->setting,
			priv->notebook);
	gtk_widget_show_all(priv->notebook);
	if(priv->setting.load_backstage)gtk_widget_hide(priv->notebook);
	if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
		my_loader_set_dialog_get_setting(dialog, &priv->setting);
		g_object_set(self, MY_TASK_PROP_LOG, priv->setting.log,
		MY_TASK_PROP_LOG_FILE, priv->setting.log_file, NULL);
	}
	my_loader_set_dialog_remove_notebook(dialog,priv->notebook);
	gtk_widget_destroy(dialog);
}
;

MyLoader *my_loader_new() {
	return g_object_new(MY_TYPE_LOADER, NULL);
}
;
