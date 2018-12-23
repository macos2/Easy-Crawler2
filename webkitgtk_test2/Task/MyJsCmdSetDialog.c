/*
 * MyJsCmdSetDialog.c
 *
 *  Created on: 2018年5月2日
 *      Author: tom
 */


#include "MyJsCmdSetDialog.h"
typedef struct{
GtkEntry *script_name,*file_entry;
GtkTextBuffer *javascript_buffer;
GtkCheckButton *log_progress;
}MyJsCmdSetDialogPrivate;
G_DEFINE_TYPE_WITH_CODE(MyJsCmdSetDialog,my_js_cmd_set_dialog,GTK_TYPE_DIALOG,G_ADD_PRIVATE(MyJsCmdSetDialog));
void file_selected_clicked(GtkButton *button,MyJsCmdSetDialog *dialog);
static void my_js_cmd_set_dialog_class_init(MyJsCmdSetDialogClass *klass){
gtk_widget_class_set_template_from_resource(klass,"/mytask/MyJsCmdSetDialog.glade");
gtk_widget_class_bind_template_child_private(klass,MyJsCmdSetDialog,script_name);
gtk_widget_class_bind_template_child_private(klass,MyJsCmdSetDialog,javascript_buffer);
gtk_widget_class_bind_template_child_private(klass,MyJsCmdSetDialog,log_progress);
gtk_widget_class_bind_template_child_private(klass,MyJsCmdSetDialog,file_entry);
gtk_widget_class_bind_template_callback(klass,file_selected_clicked);
}

static void my_js_cmd_set_dialog_init(MyJsCmdSetDialog *self){
	MyJsCmdSetDialogPrivate *priv=my_js_cmd_set_dialog_get_instance_private(self);
gtk_widget_init_template(self);
}


void file_selected_clicked(GtkButton *button,MyJsCmdSetDialog *dialog){
	gchar *filename;
	MyJsCmdSetDialogPrivate *priv=my_js_cmd_set_dialog_get_instance_private(dialog);
	GtkDialog *file_dialog=gtk_file_chooser_dialog_new("Log File",dialog,GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_OK,GTK_RESPONSE_OK,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,NULL);
	gtk_file_chooser_set_filename(file_dialog,gtk_entry_get_text(priv->file_entry));
	if(gtk_dialog_run(file_dialog)==GTK_RESPONSE_OK){
		filename=gtk_file_chooser_get_filename(file_dialog);
		gtk_entry_set_text(priv->file_entry,filename);
		g_free(filename);
	}
	gtk_widget_destroy(file_dialog);
};

MyJsCmdSetDialog * my_js_cmd_set_dialog_new(MyJsCmdSetting *setting){
	MyJsCmdSetDialog *dialog= g_object_new(MY_TYPE_JS_CMD_SET_DIALOG,NULL);
	if(setting!=NULL)my_js_cmd_set_dialog_set_setting(dialog,setting);
	return dialog;
};
void my_js_cmd_set_dialog_set_setting(MyJsCmdSetDialog * dialog,MyJsCmdSetting *setting){
	MyJsCmdSetDialogPrivate *priv=my_js_cmd_set_dialog_get_instance_private(dialog);
	if(setting->script_name!=NULL)gtk_entry_set_text(priv->script_name,setting->script_name);
	if(setting->javascript!=NULL)gtk_text_buffer_set_text(priv->javascript_buffer,setting->javascript,-1);
	if(setting->log_file!=NULL){
		gtk_entry_set_text(priv->file_entry,setting->log_file);;
	}
	gtk_toggle_button_set_active(priv->log_progress,setting->log);
};
MyJsCmdSetting *my_js_cmd_set_dialog_get_setting(MyJsCmdSetDialog * dialog){
	MyJsCmdSetDialogPrivate *priv=my_js_cmd_set_dialog_get_instance_private(dialog);
	MyJsCmdSetting *set=g_malloc(sizeof(MyJsCmdSetting));
	GtkTextIter start,end;
	gtk_text_buffer_get_bounds(priv->javascript_buffer,&start,&end);
	set->javascript=gtk_text_buffer_get_text(priv->javascript_buffer,&start,&end,TRUE);
	set->script_name=g_strdup(gtk_entry_get_text(priv->script_name));
	set->log_file=g_strdup(gtk_entry_get_text(priv->file_entry));
	set->log=gtk_toggle_button_get_active(priv->log_progress);
	return set;
};

MyJsCmdSetting *my_js_cmd_setting_new(gchar *script_name,gchar *script){
	MyJsCmdSetting *set=g_malloc(sizeof(MyJsCmdSetting));
	set->javascript=g_strdup(script);
	set->script_name=g_strdup(script_name);
	return set;
};
void my_js_cmd_setting_free(MyJsCmdSetting *setting){
	if(setting->javascript!=NULL)g_free(setting->javascript);
	if(setting->log_file!=NULL)g_free(setting->log_file);
	if(setting->script_name!=NULL)g_free(setting->script_name);
	g_free(setting);
};
