/*
 * MyStart.c
 *
 *  Created on: 2018年10月21日
 *      Author: tom
 */

#include "MyStart.h"

typedef struct {
	GtkLabel *label;
	WebKitWebView *view;
	MyStartSetDialog *dialog;
	GtkEventBox *output;
	GtkTextBuffer *context;
} MyStartPrivate;

G_DEFINE_TYPE_WITH_CODE(MyStart, my_start, MY_TYPE_TASK,
		G_ADD_PRIVATE(MyStart));

void my_start_dispose(MyStart *self) {
	MyStartPrivate *priv = my_start_get_instance_private(self);
	g_object_unref(priv->dialog);
	g_object_unref(priv->view);
	start_list=g_list_remove(start_list,self);
	G_OBJECT_CLASS(my_start_parent_class)->dispose(self);
}

void set_clicked_cb(GtkButton *button,MyStart *self){
	MyStartPrivate *priv = my_start_get_instance_private(self);
	gtk_dialog_run(priv->dialog);
}

gboolean my_start_web_decide_policy (WebKitWebView *web_view,
                  WebKitPolicyDecision *decision,
                  WebKitPolicyDecisionType type)
{
	WebKitNavigationPolicyDecision *navigation_decision;
	WebKitResponsePolicyDecision *response;
	WebKitURIRequest *uri_request;
	WebKitNavigationAction *action;
    switch (type) {
    case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
        navigation_decision = WEBKIT_NAVIGATION_POLICY_DECISION (decision);
        webkit_policy_decision_use(decision);
        /* Make a policy decision here. */
        break;
    case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
        navigation_decision = WEBKIT_NAVIGATION_POLICY_DECISION (decision);
        action=webkit_navigation_policy_decision_get_navigation_action(navigation_decision);
        uri_request=webkit_navigation_action_get_request(action);
        webkit_web_view_load_request(web_view,uri_request);
        g_print("Redirect to %s\n",webkit_uri_request_get_uri(uri_request));
        /* Make a policy decision here. */
        break;
    case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
        response = WEBKIT_RESPONSE_POLICY_DECISION (decision);
        webkit_policy_decision_use(decision);
        break;
        /* Make a policy decision here. */
    default:
        webkit_policy_decision_use(decision);
        /* Making no decision results in webkit_policy_decision_use(). */
    }
    return TRUE;
}

gchar* my_start_run(MyStart *self,TaskMsg *msg){
	GList *list=NULL;
	WebKitWebView *view;
	MyStartPrivate *priv=my_start_get_instance_private(self);
	MyTask *next_task;
	gchar *str;
	GtkTextIter start,end;
		gtk_text_buffer_get_start_iter(priv->context,&start);
		gtk_text_buffer_get_end_iter(priv->context,&end);
		str=gtk_text_buffer_get_text(priv->context,&start,&end,TRUE);
		view=priv->view;
		my_task_log(self,"<div><table>\
				<tr><th>Work Start From</th></tr>\
				<tr><td>Title</td><td>%s</td></tr>\
				<tr><td>Url</td><td>%s</td></tr>\
				<tr><td>Url</td><td>%s</td></tr>\
				</table></div>",webkit_web_view_get_title(view),webkit_web_view_get_uri(view),str);
	if(link_table!=NULL)list=g_hash_table_lookup(link_table,priv->output);
	if(list!=NULL){
		while(list!=NULL){
			my_task_send_msg(self,list->data,view,str);
			list=list->next;
		}
	}
	next_task=my_task_get_next_task(self);
	if(next_task!=NULL)my_task_send_msg(self,next_task,view,str);
	task_msg_free(msg);
	g_free(str);
	return NULL;
}

void my_start_save(MyStart *self,GOutputStream *out){
	GtkTextIter start,end;
	MyStartPrivate *priv=my_start_get_instance_private(self);
	gchar *url,*context;
	GType type=my_start_get_type();
	gboolean javascript,media_stream,plugins,auto_load_image,smooth_scrolling;
	WebKitSettings *setting=webkit_web_view_get_settings(priv->view);
	g_object_get(setting,"enable-javascript",&javascript,"enable-media-stream",&media_stream,"enable-plugins",&plugins,"auto-load-images",&auto_load_image,"enable-smooth-scrolling",&smooth_scrolling,NULL);
	url=webkit_web_view_get_uri(priv->view);
	gtk_text_buffer_get_start_iter(priv->context,&start);
	gtk_text_buffer_get_end_iter(priv->context,&end);
	context=gtk_text_buffer_get_text(priv->context,&start,&end,TRUE);
	write_to_file(out,
	MY_TYPE_GTYPE,&type,\
	MY_TYPE_STRING,url,\
	MY_TYPE_STRING,context,\
	MY_TYPE_POINTER,&priv->output,\
	MY_TYPE_BOOLEAN,&javascript,\
	MY_TYPE_BOOLEAN,&media_stream,\
	MY_TYPE_BOOLEAN,&plugins,\
	MY_TYPE_BOOLEAN,&auto_load_image,\
	MY_TYPE_BOOLEAN,&smooth_scrolling,\
	MY_TYPE_NONE
	);
	g_free(context);
};
void my_start_load(MyStart *self,GInputStream *in){
	g_print("Load start\n");
	MyStartPrivate *priv=my_start_get_instance_private(self);
	gchar *url=NULL;
	gchar *context=NULL;
	gpointer out;
	gboolean javascript,media_stream,plugins,auto_load_image,smooth_scrolling;
	WebKitSettings *setting=webkit_web_view_get_settings(priv->view);
	read_from_file(in,\
			MY_TYPE_STRING,&url,\
			MY_TYPE_STRING,&context,\
			MY_TYPE_POINTER,&out,\
			MY_TYPE_BOOLEAN,&javascript,\
			MY_TYPE_BOOLEAN,&media_stream,\
			MY_TYPE_BOOLEAN,&plugins,\
			MY_TYPE_BOOLEAN,&auto_load_image,\
			MY_TYPE_BOOLEAN,&smooth_scrolling,\
			MY_TYPE_NONE
	);
	webkit_web_view_load_uri(priv->view,url);
	g_object_set(setting,"enable-javascript",javascript,"enable-media-stream",media_stream,"enable-plugins",plugins,"auto-load-images",auto_load_image,"enable-smooth-scrolling",smooth_scrolling,NULL);
	gtk_text_buffer_set_text(priv->context,context,-1);
	g_hash_table_insert(load_table,out,priv->output);
	g_free(url);
	g_free(context);
};


static void my_start_class_init(MyStartClass *klass) {
	GObjectClass *obj_class = klass;
	MyTaskClass *task_class=klass;
	obj_class->dispose = my_start_dispose;
	task_class->run=my_start_run;
	task_class->save= my_start_save;
	task_class->load=my_start_load;
	gtk_widget_class_set_template_from_resource(klass, "/mytask/MyStart.glade");
	gtk_widget_class_bind_template_child_private(klass, MyStart, label);
	gtk_widget_class_bind_template_child_private(klass, MyStart, output);
	gtk_widget_class_bind_template_callback(klass,set_clicked_cb);
}

void web_view_load_changed(WebKitWebView *web_view, WebKitLoadEvent load_event,
		MyStart *self) {
	MyStartPrivate *priv=my_start_get_instance_private(self);
	switch (load_event) {
	case WEBKIT_LOAD_STARTED:
		gtk_label_set_text(priv->label,webkit_web_view_get_uri(web_view));
		break;
	case WEBKIT_LOAD_REDIRECTED:
		gtk_label_set_text(priv->label,webkit_web_view_get_uri(web_view));
		break;
	case WEBKIT_LOAD_COMMITTED:
		gtk_label_set_text(priv->label,"Loading...");
		break;
	case WEBKIT_LOAD_FINISHED:
		gtk_label_set_text(priv->label, webkit_web_view_get_title(web_view));
		break;
	default:
		break;
	};
}
;

static void my_start_init(MyStart *self) {
	static guint i=0;
	MyStartPrivate *priv;
	WebKitSettings *view_setting;
	gtk_widget_init_template(self);
	my_task_rename_id(self,"Start",i++);
	priv = my_start_get_instance_private(self);
	gtk_label_set_label(priv->label, "");
	priv->view = webkit_web_view_new();
	priv->context=gtk_text_buffer_new(NULL);
	priv->dialog = my_start_set_dialog_new(priv->view,priv->context,self);
	gtk_window_set_transient_for(priv->dialog, main_window);
	g_signal_connect(priv->view, "load-changed", web_view_load_changed,
			self);
	g_signal_connect(priv->view,"decide-policy",my_start_web_decide_policy,self);
	start_list=g_list_append(start_list,self);
	webkit_web_view_load_uri(priv->view,"http://www.baidu.com");
	my_task_drag_source_set(priv->output);
	my_task_set_right_clicked(priv->output,self);
}


MyStart *my_start_new(void){
	MyStart *start=g_object_new(MY_TYPE_START,NULL);
	return start;
};
