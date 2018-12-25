/*
 * MyFilterSetDialog.c
 *
 *  Created on: 2018年4月2日
 *      Author: tom
 */


#include "MyFilterSetDialog.h"
typedef struct {
GtkCheckButton *log_result;
GtkTextView *source,*test_result;
GtkEntry *pattern,*file_entry,*prefix_entry,*suffix_entry;
}MyFilterSetDialogPrivate;
void my_filter_set_dialog_match_clicked(GtkButton *button,MyFilterSetDialog *self);
void my_filter_file_select_clicked(GtkButton *button,MyFilterSetDialog *self);
G_DEFINE_TYPE_WITH_CODE(MyFilterSetDialog,my_filter_set_dialog,GTK_TYPE_DIALOG,G_ADD_PRIVATE(MyFilterSetDialog));
static void my_filter_set_dialog_class_init(MyFilterSetDialogClass *klass){
	gtk_widget_class_set_template_from_resource(klass,"/mytask/MyFilterSetDialog.glade");
	gtk_widget_class_bind_template_child_private(klass,MyFilterSetDialog,log_result);
	gtk_widget_class_bind_template_child_private(klass,MyFilterSetDialog,source);
	gtk_widget_class_bind_template_child_private(klass,MyFilterSetDialog,pattern);
	gtk_widget_class_bind_template_child_private(klass,MyFilterSetDialog,test_result);
	gtk_widget_class_bind_template_child_private(klass,MyFilterSetDialog,file_entry);
	gtk_widget_class_bind_template_child_private(klass,MyFilterSetDialog,suffix_entry);
	gtk_widget_class_bind_template_child_private(klass,MyFilterSetDialog,prefix_entry);
	gtk_widget_class_bind_template_callback(klass,my_filter_set_dialog_match_clicked);
	gtk_widget_class_bind_template_callback(klass,my_filter_file_select_clicked);
}

static void my_filter_set_dialog_init(MyFilterSetDialog *self){
	gtk_widget_init_template(self);
}

void my_filter_set_dialog_match_clicked(GtkButton *button,MyFilterSetDialog *self){
	MyFilterSetDialogPrivate *priv=my_filter_set_dialog_get_instance_private(self);
	GRegex *regex=g_regex_new(gtk_entry_get_text(priv->pattern),0,0,NULL);
	GMatchInfo *info;
	gchar *temp,*match;
	GString *result=g_string_new("");
	GtkTextIter start,end;
	GtkTextBuffer *buffer=gtk_text_view_get_buffer(priv->source);
	gtk_text_buffer_get_start_iter(buffer,&start);
	gtk_text_buffer_get_end_iter(buffer,&end);
	temp=gtk_text_buffer_get_text(buffer,&start,&end,TRUE);
	g_regex_match(regex,temp,0,&info);
	while(g_match_info_matches(info)){
		match=g_match_info_fetch(info,0);
		if(match!=NULL){
		g_string_append(result,"\"");
		g_string_append(result,gtk_entry_get_text(priv->prefix_entry));
		g_string_append(result,match);
		g_string_append(result,gtk_entry_get_text(priv->suffix_entry));
		g_string_append(result,"\"\n");
		g_free(match);
		}
		g_match_info_next(info,NULL);
	}
	buffer=gtk_text_view_get_buffer(priv->test_result);
	gtk_text_buffer_set_text(buffer,result->str,-1);
	g_string_free(result,TRUE);
	g_match_info_free(info);
	g_regex_unref(regex);
};

void my_filter_file_select_clicked(GtkButton *button,MyFilterSetDialog *self){
	gchar *filename;
	MyFilterSetDialogPrivate *priv = my_filter_set_dialog_get_instance_private(
			self);
	GtkDialog *dialog=gtk_file_chooser_dialog_new("Log File",self,GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_OK,GTK_RESPONSE_OK,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,NULL);
	gtk_file_chooser_set_filename(dialog,gtk_entry_get_text(priv->file_entry));
	if(gtk_dialog_run(dialog)==GTK_RESPONSE_OK){
		filename=gtk_file_chooser_get_filename(dialog);
		gtk_entry_set_text(priv->file_entry,filename);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
};

gboolean my_filter_set_dialog_set_setting(MyFilterSetDialog *self,MyFilterSetting *set){
	MyFilterSetDialogPrivate *priv=my_filter_set_dialog_get_instance_private(self);
	GtkTextBuffer *buffer;
	gtk_entry_set_text(priv->pattern,set->regex_pattern);
	buffer=gtk_text_view_get_buffer(priv->source);
	gtk_text_buffer_set_text(buffer,set->test_source,-1);
	gtk_toggle_button_set_active(priv->log_result,set->log_result);
	gtk_entry_set_text(priv->file_entry,set->log_file);
	gtk_entry_set_text(priv->prefix_entry,set->prefix);
	gtk_entry_set_text(priv->suffix_entry,set->suffix);
	return TRUE;
};
gboolean my_filter_set_dialog_get_setting(MyFilterSetDialog *self,MyFilterSetting *set){
	MyFilterSetDialogPrivate *priv=my_filter_set_dialog_get_instance_private(self);
	if(set->regex_pattern!=NULL)g_free(set->regex_pattern);
	if(set->test_source!=NULL)g_free(set->test_source);
	if(set->log_file!=NULL)g_free(set->log_file);
	g_free(set->prefix);
	g_free(set->suffix);
	GtkTextBuffer *buffer=gtk_text_view_get_buffer(priv->source);
	GtkTextIter start,end;
	gtk_text_buffer_get_start_iter(buffer,&start);
	gtk_text_buffer_get_end_iter(buffer,&end);
	set->regex_pattern=g_strdup(gtk_entry_get_text(priv->pattern));
	set->test_source=gtk_text_buffer_get_text(buffer,&start,&end,TRUE);
	set->log_result=gtk_toggle_button_get_active(priv->log_result);
	set->log_file=g_strdup(gtk_entry_get_text(priv->file_entry));
	set->prefix=g_strdup(gtk_entry_get_text(priv->prefix_entry));
	set->suffix=g_strdup(gtk_entry_get_text(priv->suffix_entry));

	return TRUE;
};

