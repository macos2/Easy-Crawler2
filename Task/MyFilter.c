/*
 * MyFilter.c
 *
 *  Created on: 2018年4月2日
 *      Author: tom
 */

#include "MyFilter.h"
#include "MyFilterSetDialog.h"
typedef struct {
	MyFilterSetting *set;
	GtkImage *input_img, *output_img, *state_img;
	GtkLabel *task_label;
	GtkEventBox *input, *output;
} MyFilterPrivate;

void my_filter_set_property(GObject *object, guint property_id,
		const GValue *value, GParamSpec *pspec);
void my_filter_get_property(GObject *object, guint property_id, GValue *value,
		GParamSpec *pspec);
void my_filter_finalize(MyFilter *self);

void my_filter_run(MyFilter *self, TaskMsg *msg);
void my_filter_save(MyFilter *self,GOutputStream *out);
void my_filter_load(MyFilter *self,GInputStream *in);

void my_filter_clicked(GtkButton *button, MyFilter *self);
G_DEFINE_TYPE_WITH_CODE(MyFilter, my_filter, MY_TYPE_TASK,
		G_ADD_PRIVATE(MyFilter));

static void my_filter_class_init(MyFilterClass *klass) {
	GObjectClass *obj_class = klass;
	MyTaskClass *task_class=klass;
	obj_class->finalize = my_filter_finalize;
	task_class->run=my_filter_run;
	task_class->save=my_filter_save;
	task_class->load=my_filter_load;
	gtk_widget_class_set_template_from_resource(klass,
			"/mytask/MyFilter.glade");
	gtk_widget_class_bind_template_child_private(klass, MyFilter, input_img);
	gtk_widget_class_bind_template_child_private(klass, MyFilter, output_img);
	gtk_widget_class_bind_template_child_private(klass, MyFilter, state_img);
	gtk_widget_class_bind_template_child_private(klass, MyFilter, task_label);
	gtk_widget_class_bind_template_child_private(klass, MyFilter, input);
	gtk_widget_class_bind_template_child_private(klass, MyFilter, output);
	gtk_widget_class_bind_template_callback(klass, my_filter_clicked);
	obj_class->set_property = my_filter_set_property;
	obj_class->get_property = my_filter_get_property;
}

static void my_filter_init(MyFilter *self) {
	static uint id = 0;
	gtk_widget_init_template(self);
	MyFilterPrivate *priv = my_filter_get_instance_private(self);
	priv->set = g_malloc(sizeof(MyFilterSetting));
	priv->set->regex_pattern = g_strdup("");
	priv->set->test_source = g_strdup("");
	priv->set->prefix= g_strdup("");
	priv->set->suffix= g_strdup("");
	g_object_get(self, MY_TASK_PROP_LOG_FILE, &priv->set->log_file,
			MY_TASK_PROP_LOG, &priv->set->log_result, NULL);
	gtk_label_set_text(priv->task_label, "");
	gtk_image_set_from_resource(priv->input_img, "/mytask/go-next.svg");
	gtk_image_set_from_resource(priv->output_img, "/mytask/go-next.svg");
	gtk_image_set_from_resource(priv->state_img, "/mytask/view-filter.svg");
	my_task_rename_id(self, "Filter", id++);
	my_task_drag_dest_set(priv->input);
	my_task_drag_source_set(priv->output);
	my_task_set_right_clicked(priv->output, self);
}

void my_filter_set_property(GObject *object, guint property_id,
		const GValue *value, GParamSpec *pspec) {
	MyFilterPrivate *priv = my_filter_get_instance_private(object);
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}

}
;
void my_filter_get_property(GObject *object, guint property_id, GValue *value,
		GParamSpec *pspec) {
	MyFilterPrivate *priv = my_filter_get_instance_private(object);
	switch (property_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}
;

void my_filter_finalize(MyFilter *self) {
	MyFilterPrivate *priv = my_filter_get_instance_private(self);
	g_free(priv->set->regex_pattern);
	g_free(priv->set->test_source);
	g_free(priv->set->prefix);
	g_free(priv->set->suffix);
	g_free(priv->set->log_file);
	G_OBJECT_CLASS(my_filter_parent_class)->finalize(self);
}
;

void my_filter_run(MyFilter *self, TaskMsg *msg) {
	g_return_if_fail(msg->msg!=NULL);
	MyTask *next_task=NULL;
	GList *list = NULL;
	MyFilterPrivate *priv = my_filter_get_instance_private(self);
	GRegex *regex = g_regex_new(priv->set->regex_pattern, 0, 0, NULL);
	GMatchInfo *info;
	gchar *match = NULL;
	GString *result = g_string_new("");
	g_regex_match(regex, msg->msg, 0, &info);
	while (g_match_info_matches(info)) {
		match = g_match_info_fetch(info, 0);
		if (match != NULL) {
			g_string_append(result,priv->set->prefix);
			g_string_append(result, match);
			g_string_append(result,priv->set->suffix);
			g_string_append(result, "\n");
			g_free(match);
		}
		g_match_info_next(info, NULL);
	}
	my_task_log(msg->to_task,
			"	<div><table border='2px'>\
		<tr><td>Regex Express</td><td>%s</td></tr>\
		<tr><td>Eval Content</td><td>%s</td></tr>\
		<tr><td>Result</td><td>%s</td></tr>\
		</table></div></br>",	priv->set->regex_pattern, msg->msg, result->str);

	list = g_hash_table_lookup(link_table, priv->output);
	while (list != NULL) {
		my_task_send_msg(msg->to_task, list->data, msg->view, result->str);
		list = list->next;
	}
	next_task=my_task_get_next_task(msg->to_task);
	if(next_task!=NULL)my_task_send_msg(msg->to_task, next_task, msg->view, result->str);
	g_string_free(result, TRUE);
	task_msg_free(msg);
}
;

void my_filter_save(MyFilter *self,GOutputStream *out){
	GType t;
	MyFilterPrivate *priv = my_filter_get_instance_private(self);
	t=my_filter_get_type();
	g_output_stream_write(out,&t,sizeof(GType),NULL,NULL);
	write_string(out,priv->set->log_file);
	write_string(out,priv->set->regex_pattern);
	write_string(out,priv->set->test_source);
	write_string(out,priv->set->prefix);
	write_string(out,priv->set->suffix);
	g_output_stream_write(out,&priv->set->log_result,sizeof(gboolean),NULL,NULL);
	g_output_stream_write(out,&priv->input,sizeof(gpointer),NULL,NULL);
	g_output_stream_write(out,&priv->output,sizeof(gpointer),NULL,NULL);
};
void my_filter_load(MyFilter *self,GInputStream *in){
	g_print("Load filter\n");
	gpointer pointer;
	MyFilterPrivate *priv = my_filter_get_instance_private(self);
	g_free(priv->set->log_file);
	g_free(priv->set->regex_pattern);
	g_free(priv->set->test_source);
	g_free(priv->set->prefix);
	g_free(priv->set->suffix);
	priv->set->log_file=read_string(in);
	priv->set->regex_pattern=read_string(in);
	priv->set->test_source=read_string(in);
	priv->set->prefix=read_string(in);
	priv->set->suffix=read_string(in);
	g_input_stream_read(in,&priv->set->log_result,sizeof(gboolean),NULL,NULL);
	g_input_stream_read(in,&pointer,sizeof(gpointer),NULL,NULL);
	g_hash_table_insert(load_table,pointer,priv->input);
	g_input_stream_read(in,&pointer,sizeof(gpointer),NULL,NULL);
	g_hash_table_insert(load_table,pointer,priv->output);
	gtk_label_set_text(priv->task_label,priv->set->regex_pattern);
};

void my_filter_clicked(GtkButton *button, MyFilter *self) {
	gint respon = 0;
	MyFilterPrivate *priv = my_filter_get_instance_private(self);
	MyFilterSetDialog *dialog = g_object_new(MY_TYPE_FILTER_SET_DIALOG, NULL);
	my_filter_set_dialog_set_setting(dialog, priv->set);
	gtk_window_set_transient_for(dialog, gtk_widget_get_toplevel(self));
	respon = gtk_dialog_run(dialog);
	gtk_widget_hide(dialog);
	if (respon == GTK_RESPONSE_OK) {
		my_filter_set_dialog_get_setting(dialog, priv->set);
		gtk_label_set_text(priv->task_label, priv->set->regex_pattern);
	}
	g_object_set(self, MY_TASK_PROP_LOG, priv->set->log_result,
			MY_TASK_PROP_LOG_FILE, priv->set->log_file, NULL);
	gtk_widget_destroy(dialog);
}
;

MyFilter *my_filter_new(gchar *pattern, gchar *test_source) {
	MyFilter *filter = g_object_new(MY_TYPE_FILTER, NULL);
	MyFilterPrivate *priv = my_filter_get_instance_private(filter);
	if (pattern != NULL) {
		g_free(priv->set->regex_pattern);
		priv->set->regex_pattern = g_strdup(pattern);
	}
	if (test_source != NULL) {
		g_free(priv->set->test_source);
		priv->set->test_source = g_strdup(test_source);
	}
	gtk_label_set_text(priv->task_label, priv->set->regex_pattern);
	return filter;
}
;
