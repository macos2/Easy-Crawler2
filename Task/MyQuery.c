/*
 * my_query.c
 *
 *  Created on: 2018年3月19日
 *      Author: tom
 */

#include "MyQuery.h"

#include "../MyJsHelper.h"

typedef struct {
	WebKitWebView *web_view;
	GtkImage *state_img, *input_img;
	GtkLabel *task_label;
	GtkBox *output_box;
	MyQuerySetting *set;
	GHashTable *link_table;
	GtkEventBox *input;
} MyQueryPrivate;

typedef struct {
	MyQuery *query;
	TaskMsg *msg;
} RunJsData;

void my_query_get_property(GObject *object, guint property_id, GValue *value,
		GParamSpec *pspec);
void my_query_set_property(GObject *object, guint property_id,
		const GValue *value, GParamSpec *pspec);
void my_query_check_js_callback(WebKitWebView *webview, GAsyncResult *res,
		TaskMsg *msg);
void my_query_processs_operation(MyQuery *self, TaskMsg *msg);
gboolean my_query_auto_scroll(TaskMsg *msg);
void my_query_check_js_callback(WebKitWebView *webview, GAsyncResult *res,
		TaskMsg *msg);
void my_query_run(MyQuery *self, TaskMsg *msg);
void my_query_run_js_callback(WebKitWebView *webview, GAsyncResult *res,
		RunJsData *data);
void my_query_save(MyQuery *self, GOutputStream *out);
void my_query_load(MyQuery *self, GInputStream *in);

void my_query_clicked(GtkButton *button, MyQuery *query);
void my_query_output_refresh(MyQuery *self);

G_DEFINE_TYPE_WITH_CODE(MyQuery, my_query, MY_TYPE_TASK,
		G_ADD_PRIVATE(MyQuery));

static void my_query_class_init(MyQueryClass *klass) {
	GObjectClass *obj_class = klass;
	MyTaskClass *task_class = klass;
	gtk_widget_class_set_template_from_resource(klass, "/mytask/MyQuery.glade");
	gtk_widget_class_bind_template_child_private(klass, MyQuery, state_img);
	gtk_widget_class_bind_template_child_private(klass, MyQuery, input_img);
	gtk_widget_class_bind_template_child_private(klass, MyQuery, task_label);
	gtk_widget_class_bind_template_child_private(klass, MyQuery, output_box);
	gtk_widget_class_bind_template_child_private(klass, MyQuery, input);
	gtk_widget_class_bind_template_callback(klass, my_query_clicked);
	obj_class->get_property = my_query_get_property;
	obj_class->set_property = my_query_set_property;
	task_class->run = my_query_run;
	task_class->save = my_query_save;
	task_class->load = my_query_load;
}

static void my_query_init(MyQuery *self) {
	static uint id = 0;
	gtk_widget_init_template(self);
	MyQueryPrivate *priv = my_query_get_instance_private(self);
	priv->web_view = NULL;
	priv->link_table = g_hash_table_new(g_direct_hash, g_direct_equal);
	priv->set = my_query_set_new();
	priv->set->auto_scroll = TRUE;
	priv->set->wait_for_load_finish = TRUE;
	priv->set->inspect_interval = 250;
	priv->set->inspect_times = 3;
	g_object_get(self, MY_TASK_PROP_LOG, &priv->set->log, MY_TASK_PROP_LOG_FILE,
			&priv->set->log_file, NULL);
	gtk_box_set_spacing(priv->output_box, 1);
	gtk_image_set_from_resource(priv->state_img, "/mytask/edit-find.svg");
	gtk_image_set_from_resource(priv->input_img, "/mytask/network-transmit-receive-symbolic.svg");
	gtk_widget_show_all(self);
	my_task_rename_id(self, "Query", id++);
	my_task_drag_dest_set(priv->input);
}

void my_query_get_property(GObject *object, guint property_id, GValue *value,
		GParamSpec *pspec) {
	MyQueryPrivate *priv = my_query_get_instance_private(object);
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}
;
void my_query_set_property(GObject *object, guint property_id,
		const GValue *value, GParamSpec *pspec) {
	MyQueryPrivate *priv = my_query_get_instance_private(object);
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}
;

void my_query_run(MyQuery *self, TaskMsg *msg) {
	guint source_id = 0;
	MyQueryPrivate *priv = my_query_get_instance_private(self);
	source_id = g_timeout_add(priv->set->inspect_interval, my_query_auto_scroll,
			msg);
	msg->preserve = GUINT_TO_POINTER(source_id);
	runing_count++;
}
;

gboolean my_query_auto_scroll(TaskMsg *msg) {
	WebKitWebView *view = msg->view;
	MyQuery *query = msg->to_task;
	MyQueryPrivate *priv = my_query_get_instance_private(query);
	GString *str = g_string_new("");
	if (priv->set->auto_scroll)
		webkit_web_view_run_javascript(view,
				"window.scrollTo(document.body.scrollWidth,document.body.scrollHeight);",
				NULL,
				NULL, NULL);

	if (g_object_get_data(view, "checking") == NULL) {
		g_string_printf(str, "document.querySelectorAll('%s').length",
				priv->set->selector);
		webkit_web_view_run_javascript(view, str->str, NULL,
				my_query_check_js_callback, msg);
		g_object_set_data(view, "checking", GINT_TO_POINTER(1));
	}

	g_string_free(str, TRUE);
	return G_SOURCE_CONTINUE;
}
;

void my_query_check_js_callback(WebKitWebView *webview, GAsyncResult *res,
		TaskMsg *msg) {
	gdouble *cur, *pre;
	gint *reply;
	MyQuery *query = msg->to_task;
	;
	MyQueryPrivate *priv = my_query_get_instance_private(query);
	cur = g_malloc0(sizeof(gdouble));
	WebKitJavascriptResult *js_result = webkit_web_view_run_javascript_finish(
			webview, res, NULL);
	if (js_result == NULL)
		return;
	JSValueRef value = webkit_javascript_result_get_value(js_result);
	JSGlobalContextRef ctx = webkit_javascript_result_get_global_context(
			js_result);
	if (JSValueIsNumber(ctx, value)) {
		*cur = JSValueToNumber(ctx, value, NULL);
		pre = g_object_steal_data(webview, "selector_length");
		if (pre == NULL) {
			g_object_set_data(webview, "selector_length", cur);
		} else {
			reply = g_object_steal_data(webview, "selector_length_reply");
			if (reply == NULL) {
				reply = g_malloc(sizeof(gint));
				*reply = 0;
				g_object_set_data(webview, "selector_length_reply", reply);
			};
			if (*cur == *pre) {
				if (*reply >= priv->set->inspect_times) {
					g_source_remove(GPOINTER_TO_UINT(msg->preserve));
					my_query_processs_operation(query, msg);
					g_free(reply);
				} else {
					*reply = *reply + 1;
					g_object_set_data(webview, "selector_length_reply", reply);
				}
				g_free(cur);
				g_free(pre);
			} else {
				*reply = 0;
				g_object_set_data(webview, "selector_length_reply", reply);
				g_object_set_data(webview, "selector_length", cur);
				g_free(pre);
			};
		}
	}
	g_object_steal_data(webview, "checking");
	webkit_javascript_result_unref(js_result);
}
;

void my_query_processs_operation(MyQuery *self, TaskMsg *msg) {
	MyQueryPrivate *priv = my_query_get_instance_private(self);
	GList *output_prop = priv->set->output_prop;
	GString *str = g_string_new("");
	RunJsData *data;
	gchar *p;
	data = g_malloc(sizeof(RunJsData));
	data->query = self;
	data->msg = msg;
	g_string_printf(str,
			"var array=new Array();window.document.querySelectorAll('%s').forEach(function(x){",
			priv->set->selector);
	while (output_prop != NULL) {
		/*p=g_utf8_strchr(gtk_button_get_label(output_prop->data),-1,'-');//检测属性名是否含有符号
		p==NULL?g_string_append_printf(str, "array.push(x.%s);",gtk_button_get_label(output_prop->data)):*/
		g_string_append_printf(str, "array.push(x.getAttribute('%s'));",gtk_button_get_label(output_prop->data));

		output_prop = output_prop->next;
	}
	g_string_append_printf(str, "});array;");
	webkit_web_view_run_javascript(msg->view, str->str, NULL,
			my_query_run_js_callback, data);
	g_string_free(str, TRUE);
}

void my_query_run_js_callback(WebKitWebView *webview, GAsyncResult *res,
		RunJsData *data) {
	GError *error = NULL;
	gchar *temp;
	guint n = 0, i = 0;
	JSValueRef value, iter, next, done, str;
	JSObjectRef value_obj, iter_obj, next_obj;
	JSStringRef str_str;
	JSContextRef ctx;
	GArray *arr;
	GList *list = NULL, *output_prop;
	MyTask *next_task;
	MyQueryPrivate *priv = my_query_get_instance_private(data->query);
	WebKitJavascriptResult *js_result = webkit_web_view_run_javascript_finish(
			webview, res, &error);
	if (js_result == NULL) {
		g_warning("####JavaSript Run Error:%s\n", error->message);
		g_error_free(error);
		task_msg_free(data->msg);
		g_free(data);
		runing_count--;
		return;
	}
	//value=webkit_javascript_result_get_js_value(js_result);
	value = webkit_javascript_result_get_value(js_result);
	ctx = webkit_javascript_result_get_global_context(js_result);
	if (JSValueIsArray(ctx, value)) {
		n = g_list_length(priv->set->output_prop);
		arr = g_array_new(FALSE, FALSE, sizeof(gpointer));
		my_task_log(data->query, "<h3>Selector:'%s'</h3>", priv->set->selector);
		my_task_log(data->query, "<div><table border='2px'><tr>");
		output_prop = priv->set->output_prop;
		while (output_prop != NULL) {
			temp = g_string_new("");
			g_array_append_val(arr, temp);
			my_task_log(data->query, "<th>%s</th>",
					gtk_button_get_label(output_prop->data));
			output_prop = output_prop->next;
		}
		my_task_log(data->query, "</tr><tr>");
		value_obj = JSValueToObject(ctx, value, NULL);
		iter = my_js_obj_method(ctx, value_obj, "values");
		iter_obj = JSValueToObject(ctx, iter, NULL);
		next = my_js_obj_method(ctx, iter_obj, "next");
		next_obj = JSValueToObject(ctx, next, NULL);
		done = my_js_obj_get_prop(ctx, next_obj, "done");
		i = 0;
		while (JSValueToBoolean(ctx, done) == false) {
			str = my_js_obj_get_prop(ctx, next_obj, "value");
			temp = my_js_value_to_gchar(ctx, str);
			my_task_log(data->query, "<td>%s</td>", temp);
			g_string_append(g_array_index(arr, GString*, i), temp);
			g_string_append(g_array_index(arr, GString*, i), "\n");
			g_free(temp);

			next = my_js_obj_method(ctx, iter_obj, "next");
			next_obj = JSValueToObject(ctx, next, NULL);
			done = my_js_obj_get_prop(ctx, next_obj, "done");
			i++;
			if (i >= n) {
				i = 0;
				my_task_log(data->query, "</tr><tr>");
			}
		}
		my_task_log(data->query, "</tr></table></div></br>");
		output_prop = priv->set->output_prop;
		next_task = my_task_get_next_task(data->query);
		if (next_task != NULL) {
			for (i = 0; i < n; i++) {
				my_task_send_msg(data->query, next_task, webview,
				g_array_index(arr,GString*,i)->str);
			}
		}
		i = 0;
		while (output_prop != NULL) {
			list = g_hash_table_lookup(link_table, output_prop->data);
			while (list != NULL) {
				my_task_send_msg(data->query, list->data, webview,
				g_array_index(arr,GString*,i)->str);
				list = list->next;
			}
			g_string_free(g_array_index(arr, GString*, i), TRUE);
			i++;
			output_prop = output_prop->next;
		}

	}
	task_msg_free(data->msg);
	g_free(data);
	runing_count--;
	webkit_javascript_result_unref(js_result);

}
;

void my_query_save(MyQuery *self, GOutputStream *out) {
	GType type = my_query_get_type();
	MyQueryPrivate *priv = my_query_get_instance_private(self);
	guint out_prop_len = g_list_length(priv->set->output_prop);
	GList *l = priv->set->output_prop;
	write_to_file(out,
	MY_TYPE_GTYPE, &type,
	MY_TYPE_BOOLEAN, &priv->set->auto_scroll,
	MY_TYPE_DOUBLE, &priv->set->delay,
	MY_TYPE_UINT, &priv->set->inspect_interval,
	MY_TYPE_UINT, &priv->set->inspect_times,
	MY_TYPE_BOOLEAN, &priv->set->log,
	MY_TYPE_STRING, priv->set->log_file,
	MY_TYPE_STRING, priv->set->selector,
	MY_TYPE_POINTER, &priv->input,
	MY_TYPE_UINT, &out_prop_len,
	MY_TYPE_NONE);
	while (l != NULL) {
		write_to_file(out,
		MY_TYPE_POINTER, &l->data,
		MY_TYPE_STRING, gtk_button_get_label(l->data),
		MY_TYPE_NONE);
		l = l->next;
	}
}
;
void my_query_load(MyQuery *self, GInputStream *in) {
	g_print("Load query\n");
	gpointer input;
	MyQueryPrivate *priv = my_query_get_instance_private(self);
	priv->set->del_out_prop = g_list_copy(priv->set->output_prop);
	my_query_output_refresh(self);
	guint out_prop_len,i;
	gpointer *p;
	gchar *str;
	GList *l=NULL;
	read_from_file(in,
	MY_TYPE_BOOLEAN, &priv->set->auto_scroll,
	MY_TYPE_DOUBLE, &priv->set->delay,
	MY_TYPE_UINT, &priv->set->inspect_interval,
	MY_TYPE_UINT, &priv->set->inspect_times,
	MY_TYPE_BOOLEAN, &priv->set->log,
	MY_TYPE_STRING, &priv->set->log_file,
	MY_TYPE_STRING, &priv->set->selector,
	MY_TYPE_POINTER, &input,
	MY_TYPE_UINT, &out_prop_len,
	MY_TYPE_NONE);
	p=g_malloc(out_prop_len*sizeof(gpointer));
	g_hash_table_insert(load_table,input,priv->input);
	for(i=0;i<out_prop_len;i++){
		str=NULL;
		read_from_file(in,\
		MY_TYPE_POINTER, &p[i],\
		MY_TYPE_STRING, &str,\
		MY_TYPE_NONE);
		priv->set->add_out_prop=g_list_append(priv->set->add_out_prop,str);
	}
	my_query_output_refresh(self);
	l=priv->set->output_prop;
	for(i=0;i<out_prop_len;i++){
		g_hash_table_insert(load_table,p[i],l->data);
		l=l->next;
	}
	g_free(p);
	gtk_label_set_text(priv->task_label,priv->set->selector);
}
;

void my_query_clicked(GtkButton *button, MyQuery *query) {
	MyQueryPrivate *priv = my_query_get_instance_private(query);
	MyQuerySetDialog *dialog = g_object_new(MY_TYPE_QUERY_SET_DIALOG, NULL);
	gtk_window_set_transient_for(dialog, gtk_widget_get_toplevel(query));
	my_query_set_dialog_set_set(dialog, priv->set);
	if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
		my_query_set_dialog_get_set(dialog, priv->set);
		gtk_label_set_text(priv->task_label, priv->set->selector);
		my_query_output_refresh(query);
	}
	g_object_set(query, MY_TASK_PROP_LOG, priv->set->log, MY_TASK_PROP_LOG_FILE,
			priv->set->log_file, NULL);
	gtk_widget_destroy(dialog);

}
;

void my_query_output_refresh(MyQuery *self) {
	MyQueryPrivate *priv = my_query_get_instance_private(self);
	MyQuerySetting *set = priv->set;
	GtkButton *button;
	//清空原有的选项；
	GList *list = gtk_container_get_children(priv->output_box), *p = NULL,
			*link_list = NULL;
	p = list;
	while (p != NULL) {
		g_object_ref(p->data);
		gtk_container_remove(priv->output_box, p->data);
		p = p->next;
	}
	g_list_free(list);
	p = set->add_out_prop;
	if (set->add_out_prop != NULL && g_list_length(set->add_out_prop) > 0) {
		while (p != NULL) {
			button = gtk_button_new_with_label(p->data);
			gtk_button_set_relief(button, GTK_RELIEF_NONE);
			my_task_set_right_clicked(button, self);
			set->output_prop = g_list_append(set->output_prop, button);
			p = p->next;
		}
	}
	g_list_free(set->add_out_prop);

	p = set->del_out_prop;
	if (set->del_out_prop != NULL && g_list_length(set->del_out_prop) > 0) {
		while (p != NULL) {
			set->output_prop = g_list_remove(set->output_prop, p->data);
			link_list = g_hash_table_lookup(priv->link_table, p->data);
			if (link_list != NULL) {
				g_list_free_full(link_list, g_free);
				link_list = NULL;
			}
			p = p->next;
		}
		g_list_free_full(set->del_out_prop, gtk_widget_destroy);
	}

	set->add_out_prop = NULL;
	set->del_out_prop = NULL;
	//载入新的选项；
	guint len = g_list_length(set->output_prop), i = 0;
	list = g_list_first(set->output_prop);
	gtk_box_set_spacing(priv->output_box, len);
	for (i = 0; i < len; i++) {
		gtk_box_pack_start(priv->output_box, list->data, FALSE, TRUE, 0);
		my_task_drag_source_set(list->data);
		g_object_set_data(list->data, "MyQuery", self);
		if (list->next != NULL)
			list = list->next;
	}
	gtk_widget_show_all(priv->output_box);
}
;

MyQuery *my_query_new(WebKitWebView *web_view, gchar *selector, ...) {
	gchar *str;
	GtkButton *button;
	va_list p;
	va_start(p, NULL);
	MyQuery *query = g_object_new(MY_TYPE_QUERY, NULL);
	MyQueryPrivate *priv = my_query_get_instance_private(query);
	g_free(priv->set->selector);
	priv->set->selector = g_strdup(selector);
	gtk_label_set_text(priv->task_label, priv->set->selector);
	priv->web_view = web_view;
	str = va_arg(p, gchar*);
	while (str != NULL) {
		button = gtk_button_new_with_label(str);
		gtk_button_set_relief(button, GTK_RELIEF_NONE);
		my_task_set_right_clicked(button, query);
		priv->set->output_prop = g_list_append(priv->set->output_prop, button);
		str = va_arg(p, char*);
	}
	va_end(p);
	my_query_output_refresh(query);
	return query;
}
;
