/*
 * MyDl.c
 *
 *  Created on: 2018年10月15日
 *      Author: tom
 */

#include "MyDl.h"

typedef struct {
	GtkButton *start, *pause;
	GtkLabel *label;
	MyDownload *download;
	GtkImage *input_img,*output_img;
	GtkEventBox *input,*output;
} MyDlPrivate;
G_DEFINE_TYPE_WITH_CODE(MyDl, my_dl, MY_TYPE_TASK, G_ADD_PRIVATE(MyDl));

void my_dl_run(MyDl *self,TaskMsg *msg);
void my_dl_save(MyTask *self,GOutputStream *out);
void my_dl_load(MyTask *self,GInputStream *in);
void my_dl_stop_run(MyTask *self);
void my_dl_count_notify(MyDownload    *down,
               GParamSpec *pspec,
               MyDl    *self);


void start_clicked_cb(GtkButton *button, MyDl *self);
void label_clicked_cb(GtkButton *button, MyDl *self);
void pause_clicked_cb(GtkButton *button, MyDl *self);



static void my_dl_class_init(MyDlClass *klass) {
	GObjectClass *obj_class=klass;
	MyTaskClass *task_class=klass;
	task_class->run=my_dl_run;
	task_class->save=my_dl_save;
	task_class->load=my_dl_load;
	task_class->stop_run=my_dl_stop_run;
	gtk_widget_class_set_template_from_resource(klass, "/mytask/MyDl.glade");
	gtk_widget_class_bind_template_child_private(klass, MyDl, start);
	gtk_widget_class_bind_template_child_private(klass, MyDl, pause);
	gtk_widget_class_bind_template_child_private(klass, MyDl, label);
	gtk_widget_class_bind_template_child_private(klass, MyDl, input_img);
	gtk_widget_class_bind_template_child_private(klass, MyDl, output_img);
	gtk_widget_class_bind_template_child_private(klass, MyDl, input);
	gtk_widget_class_bind_template_child_private(klass, MyDl, output);
	gtk_widget_class_bind_template_callback(klass, start_clicked_cb);
	gtk_widget_class_bind_template_callback(klass, label_clicked_cb);
	gtk_widget_class_bind_template_callback(klass, pause_clicked_cb);
}
static void my_dl_init(MyDl *self) {
	static uint id=0;
	gchar *name;
	MyDlPrivate *priv = my_dl_get_instance_private(self);
	my_task_rename_id(self,"DownLoader",id++);
	gtk_widget_init_template(self);
	if (!WEBKIT_IS_WEB_VIEW(default_view))
		default_view = webkit_web_view_new();
	priv->download = my_download_new(default_view, NULL, NULL, NULL);
	g_object_get(self,MY_TASK_PROP_NAME,&name,NULL);
	g_object_set(priv->download,"name",name,NULL);
	g_free(name);
	gtk_widget_hide(priv->download);
	g_signal_connect(priv->download, "delete_event", gtk_widget_hide, priv->download);
	gtk_label_set_text(priv->label,"No Task");
	my_task_drag_dest_set(priv->input);
	my_task_drag_source_set(priv->output);
	my_task_set_right_clicked(priv->output,self);
	g_signal_connect(priv->download,"notify::count",my_dl_count_notify,self);
}

void my_dl_run(MyDl *self,TaskMsg *msg){
	MyTask *next_task=NULL;
	guint i=0,j=0;
	MyDlPrivate *priv=my_dl_get_instance_private(self);
	GList *list=g_hash_table_lookup(link_table,priv->output);
	if(msg->msg==NULL){
		task_msg_free(msg);
		return;
	}
gchar **url=g_strsplit(msg->msg,"\n",-1),**strv=NULL;
SoupURI *b_uri,*s_uri;
b_uri=soup_uri_new(webkit_web_view_get_uri(msg->view));
GString *string=g_string_new("");
GValue task_name=G_VALUE_INIT;

g_value_init(&task_name,G_TYPE_STRING);
g_object_get_property(msg->from_task,MY_TASK_PROP_NAME,&task_name);
my_task_log(self,"<div><table><tr><th>DownLoad</th></tr><tr><td>");
if(WEBKIT_IS_WEB_VIEW(msg->view))strv=g_strsplit(webkit_web_view_get_title(msg->view),"/",-1);
if(strv!=NULL){
	while(strv[j+1]!=NULL){
		g_string_append(string,strv[j]);
		g_string_append(string,"_");
		j++;
	}
	g_string_append(string,strv[j]);
	g_strfreev(strv);
	}else{
		g_value_init(&task_name,G_TYPE_STRING);
		g_object_get_property(msg->from_task,MY_TASK_PROP_NAME,&task_name);
		g_string_append(string,g_value_get_string(&task_name));
		g_value_unset(&task_name);
	}
while(url[i]!=NULL){
	if(g_strcmp0(url[i],"")==0){
		i++;
		continue;
	}
	s_uri=soup_uri_new_with_base(b_uri,url[i]);
	my_download_add(priv->download,soup_uri_to_string(s_uri,FALSE),string->str);
	my_task_log(self,"%s</br>",url[i]);
	i++;
	soup_uri_free(s_uri);
}
my_task_log(self,"</td></tr></table></div>");
while(list!=NULL){
	my_task_send_msg(self,list->data,msg->view,msg->msg);
	list=list->next;
}
next_task=my_task_get_next_task(self);
if(next_task!=NULL)	my_task_send_msg(self,next_task,msg->view,msg->msg);

//my_task_log(self,"<div><table><tr><th>DownLoad</th></tr><tr><td>%s</td></tr></table></div>",msg->msg);
soup_uri_free(b_uri);
g_strfreev(url);
g_string_free(string,TRUE);
task_msg_free(msg);
};


void my_dl_save(MyTask *self,GOutputStream *out){
	GType t=my_dl_get_type();
MyDlPrivate *priv=my_dl_get_instance_private(self);
MyDownloadSetting *set=my_download_get_setting(priv->download);
g_output_stream_write(out,&t,sizeof(GType),NULL,NULL);
write_string(out,set->global_prefix);
write_string(out,set->global_suffix);
write_string(out,set->save_local);
write_string(out,set->name_format);
g_output_stream_write(out,&set->same_op,sizeof(set->same_op),NULL,NULL);

g_output_stream_write(out,&set->u_special_name_format,sizeof(gboolean),NULL,NULL);

g_output_stream_write(out,&set->auto_backup,sizeof(gboolean),NULL,NULL);
g_output_stream_write(out,&set->skip_same_url,sizeof(gboolean),NULL,NULL);
//save in-out addr
g_output_stream_write(out,&priv->input,sizeof(gpointer),NULL,NULL);
g_output_stream_write(out,&priv->output,sizeof(gpointer),NULL,NULL);
my_download_setting_free(set);
};

void my_dl_load(MyTask *self,GInputStream *in){
	g_print("Load DL\n");
	gpointer pointer=NULL;
	MyDlPrivate *priv=my_dl_get_instance_private(self);
	MyDownloadSetting *set=g_malloc0(sizeof(MyDownloadSetting));
	set->global_prefix=read_string(in);
	set->global_suffix=read_string(in);
	set->save_local=read_string(in);
	set->name_format=read_string(in);
	g_input_stream_read(in,&set->same_op,sizeof(set->same_op),NULL,NULL);

	g_input_stream_read(in,&set->u_special_name_format,sizeof(gboolean),NULL,NULL);

	g_input_stream_read(in,&set->auto_backup,sizeof(gboolean),NULL,NULL);
	g_input_stream_read(in,&set->skip_same_url,sizeof(gboolean),NULL,NULL);
	my_download_set_setting(priv->download,set);
	my_download_setting_free(set);
	g_input_stream_read(in,&pointer,sizeof(gpointer),NULL,NULL);
	g_hash_table_insert(load_table,pointer,priv->input);
	g_input_stream_read(in,&pointer,sizeof(gpointer),NULL,NULL);
	g_hash_table_insert(load_table,pointer,priv->output);
};

void my_dl_stop_run(MyTask *self){
	MyDlPrivate *priv = my_dl_get_instance_private(self);
	my_download_stop_all(priv->download);
};


void my_dl_count_notify(MyDownload    *down,
               GParamSpec *pspec,
               MyDl    *self){
	gchar buf[20];
	gint count,total_count;
	g_object_get(down,"count",&count,"total_count",&total_count,NULL);
	g_sprintf(buf,"%d/%d",count,total_count);
	MyDlPrivate *priv=my_dl_get_instance_private(self);
	gtk_label_set_text(priv->label,buf);
};

void start_clicked_cb(GtkButton *button, MyDl *self){
	MyDlPrivate *priv = my_dl_get_instance_private(self);
	my_download_start_all(priv->download);
};

void label_clicked_cb(GtkButton *button, MyDl *self) {
	MyDlPrivate *priv = my_dl_get_instance_private(self);
	gtk_widget_show(priv->download);
}
;
void pause_clicked_cb(GtkButton *button, MyDl *self){
	MyDlPrivate *priv = my_dl_get_instance_private(self);
	my_download_stop_all(priv->download);
};

MyDl *my_dl_new(){
	return g_object_new(MY_TYPE_DL,NULL);
};

