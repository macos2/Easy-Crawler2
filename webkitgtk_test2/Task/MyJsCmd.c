/*
 * MyJsCmd.c
 *
 *  Created on: 2018年5月2日
 *      Author: tom
 */

#include "MyJsCmd.h"

typedef struct {
	MyJsCmdSetting *setting;
	GtkLabel *label;
	GtkImage *input_img, *output_img;
	GtkEventBox *input, *output;
} MyJsCmdPrivate;

G_DEFINE_TYPE_WITH_CODE(MyJsCmd, my_js_cmd, MY_TYPE_TASK,
		G_ADD_PRIVATE(MyJsCmd));

void my_js_cmd_run(MyJsCmd *task, TaskMsg *msg);
void my_js_cmd_run_cb(WebKitWebView *view, GAsyncResult *res, TaskMsg *msg);
void my_js_cmd_save(MyJsCmd *self,GOutputStream *out);
void my_js_cmd_load(MyJsCmd *self,GInputStream *in);
void my_js_cmd_clicked(GtkButton *button, MyJsCmd *cmd);

static void my_js_cmd_class_init(MyJsCmdClass *klass) {
	MyTaskClass *task_class = klass;
	GObjectClass *obj_class = klass;
	task_class->run = my_js_cmd_run;
	task_class->save=my_js_cmd_save;
	task_class->load=my_js_cmd_load;
	gtk_widget_class_set_template_from_resource(klass, "/mytask/MyJsCmd.glade");
	gtk_widget_class_bind_template_callback(klass, my_js_cmd_clicked);
	gtk_widget_class_bind_template_child_private(klass, MyJsCmd, label);
	gtk_widget_class_bind_template_child_private(klass, MyJsCmd, input_img);
	gtk_widget_class_bind_template_child_private(klass, MyJsCmd, output_img);
	gtk_widget_class_bind_template_child_private(klass, MyJsCmd, input);
	gtk_widget_class_bind_template_child_private(klass, MyJsCmd, output);
}

static void my_js_cmd_init(MyJsCmd *self) {
	static uint id = 0;
	MyJsCmdPrivate *priv = my_js_cmd_get_instance_private(self);
	gtk_widget_init_template(self);
	priv->setting = my_js_cmd_setting_new("", "");
	g_object_get(self, MY_TASK_PROP_LOG, &priv->setting->log,
	MY_TASK_PROP_LOG_FILE, &priv->setting->log_file, NULL);
	gtk_label_set_text(priv->label, priv->setting->script_name);
	my_task_rename_id(self, "JavaScript", id++);
	my_task_drag_dest_set(priv->input);
	my_task_drag_source_set(priv->output);
	my_task_set_right_clicked(priv->output, self);
}

void my_js_cmd_run(MyJsCmd *task, TaskMsg *msg) {
	MyJsCmdPrivate *priv = my_js_cmd_get_instance_private(task);
	webkit_web_view_run_javascript(msg->view, priv->setting->javascript, NULL,
			my_js_cmd_run_cb, msg);
	runing_count++;
}
;

void my_js_cmd_run_cb(WebKitWebView *view, GAsyncResult *res, TaskMsg *msg) {
	MyTask *next_task;
	MyJsCmdPrivate *priv = my_js_cmd_get_instance_private(msg->to_task);
	WebKitJavascriptResult *js_res = webkit_web_view_run_javascript_finish(view,
			res, NULL);
	gchar *toString_res = NULL;
	GList *list = NULL;
	if (js_res != NULL) {
		JSValueRef value = webkit_javascript_result_get_value(js_res);
		JSGlobalContextRef ctx = webkit_javascript_result_get_global_context(
				js_res);
		if (JSValueIsString(ctx, value)) {
			toString_res = my_js_value_to_gchar(ctx, value);
		} else if (JSValueIsArray(ctx, value)) {
			JSObjectRef obj = JSValueToObject(ctx, value, NULL);
			value = my_js_obj_method_args(ctx, obj, "join", "\n", NULL);
			toString_res = my_js_value_to_gchar(ctx, value);
		} else if (JSValueIsObject(ctx, value)) {
			JSObjectRef obj = JSValueToObject(ctx, value, NULL);
			value = my_js_obj_method(ctx, obj, "toString");
			toString_res = my_js_value_to_gchar(ctx, value);
		}
		webkit_javascript_result_unref(js_res);
	}
	my_task_log(msg->to_task,
			"<div><table>\
				<tr><td>Title</td><td>%s</td></tr>\
				<tr><td>Uri</td><td>%s</td></tr>\
				<tr><td>Script Name</td><td>%s</td></tr>\
				<tr><td>Script</td><td>%s</td></tr>\
				<tr><td>Result</td><td>%s</td></tr></table></div>", webkit_web_view_get_title(msg->view),
			webkit_web_view_get_uri(msg->view), priv->setting->script_name,
			priv->setting->javascript, toString_res);

		list = g_hash_table_lookup(link_table, priv->output);
		while (list != NULL) {
			my_task_send_msg(msg->to_task, list->data, msg->view, toString_res);
			list = list->next;
		}
		next_task=my_task_get_next_task(msg->to_task);
		if(next_task!=NULL)my_task_send_msg(msg->to_task,next_task, msg->view, toString_res);
		if (toString_res != NULL) 	g_free(toString_res);
	task_msg_free(msg);
	runing_count--;
}
;

void my_js_cmd_save(MyJsCmd *self,GOutputStream *out){
GType type=my_js_cmd_get_type();
MyJsCmdPrivate *priv=my_js_cmd_get_instance_private(self);
g_output_stream_write(out,&type,sizeof(GType),NULL,NULL);
write_string(out,priv->setting->javascript);
write_string(out,priv->setting->log_file);
write_string(out,priv->setting->script_name);
g_output_stream_write(out,&priv->setting->log,sizeof(gboolean),NULL,NULL);
g_output_stream_write(out,&priv->input,sizeof(gpointer),NULL,NULL);
g_output_stream_write(out,&priv->output,sizeof(gpointer),NULL,NULL);
};
void my_js_cmd_load(MyJsCmd *self,GInputStream *in){
	g_print("Load js cmd\n");
	gpointer pointer;
	MyJsCmdPrivate *priv=my_js_cmd_get_instance_private(self);
	g_free(priv->setting->javascript);
	g_free(priv->setting->log_file);
	g_free(priv->setting->script_name);
	priv->setting->javascript=read_string(in);
	priv->setting->log_file=read_string(in);
	priv->setting->script_name=read_string(in);
	g_input_stream_read(in,&priv->setting->log,sizeof(gboolean),NULL,NULL);
	g_input_stream_read(in,&pointer,sizeof(gpointer),NULL,NULL);
	g_hash_table_insert(load_table,pointer,priv->input);
	g_input_stream_read(in,&pointer,sizeof(gpointer),NULL,NULL);
	g_hash_table_insert(load_table,pointer,priv->output);
	gtk_label_set_text(priv->label,priv->setting->script_name);
};

void my_js_cmd_clicked(GtkButton *button, MyJsCmd *cmd) {
	MyJsCmdPrivate *priv = my_js_cmd_get_instance_private(cmd);
	MyJsCmdSetDialog *dialog = my_js_cmd_set_dialog_new(priv->setting);
	gtk_window_set_transient_for(dialog, gtk_widget_get_toplevel(cmd));
	if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
		my_js_cmd_setting_free(priv->setting);
		priv->setting = my_js_cmd_set_dialog_get_setting(dialog);
		g_object_set(cmd, MY_TASK_PROP_LOG_FILE, priv->setting->log_file,
		MY_TASK_PROP_LOG, priv->setting->log, NULL);
	}
	gtk_label_set_text(priv->label, priv->setting->script_name);
	gtk_widget_destroy(dialog);
}
;

MyJsCmd *my_js_cmd_new(gchar *name, gchar *script) {
	MyJsCmd *jscmd = g_object_new(MY_TYPE_JS_CMD, NULL);
	MyJsCmdPrivate *priv = my_js_cmd_get_instance_private(jscmd);
	if (name != NULL) {
		g_free(priv->setting->script_name);
		priv->setting->script_name = g_strdup(name);
	};
	if (script != NULL) {
		g_free(priv->setting->javascript);
		priv->setting->javascript = g_strdup(script);
	}
	gtk_label_set_text(priv->label, priv->setting->script_name);
	return jscmd;
}
;
