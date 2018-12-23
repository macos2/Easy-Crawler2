/*
 * MyLoaderSetDialog.c
 *
 *  Created on: 2018年5月7日
 *      Author: tom
 */

#include "MyLoaderSetDialog.h"

typedef struct {
	GtkCheckButton *load_backstage, *log_process,*skip_same_url;
	GtkAdjustment *timeout,*max_count;
	GtkEntry *file_entry;
	GtkBox *box;
	GtkNotebook *notebook;
} MyLoaderSetDialogPrivate;

void my_loader_set_dialog_select_file(GtkButton *button,
		MyLoaderSetDialog *self);

G_DEFINE_TYPE_WITH_CODE(MyLoaderSetDialog, my_loader_set_dialog,
		GTK_TYPE_DIALOG, G_ADD_PRIVATE(MyLoaderSetDialog));


static void my_loader_set_dialog_class_init(MyLoaderSetDialogClass *klass) {
	GObjectClass *obj_class=klass;
	gtk_widget_class_set_template_from_resource(klass,
			"/mytask/MyLoaderSetDialog.glade");
	gtk_widget_class_bind_template_child_private(klass, MyLoaderSetDialog,
			max_count);
	gtk_widget_class_bind_template_child_private(klass, MyLoaderSetDialog,
			timeout);
	gtk_widget_class_bind_template_child_private(klass, MyLoaderSetDialog,
			file_entry);
	gtk_widget_class_bind_template_child_private(klass, MyLoaderSetDialog,
			load_backstage);
	gtk_widget_class_bind_template_child_private(klass, MyLoaderSetDialog,
			log_process);
	gtk_widget_class_bind_template_child_private(klass, MyLoaderSetDialog,
			skip_same_url);
	gtk_widget_class_bind_template_child_private(klass, MyLoaderSetDialog, box);
	gtk_widget_class_bind_template_callback(klass,
			my_loader_set_dialog_select_file);
}
static void my_loader_set_dialog_init(MyLoaderSetDialog *self) {
	gtk_widget_init_template(self);

}

void my_loader_set_dialog_select_file(GtkButton *button,
		MyLoaderSetDialog *self) {
	gchar *filename = NULL;
	MyLoaderSetDialogPrivate *priv = my_loader_set_dialog_get_instance_private(
			self);
	GtkFileChooserDialog *dialog = gtk_file_chooser_dialog_new("Log File", self,
			GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_OK, GTK_RESPONSE_OK,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_file_chooser_set_filename(dialog, gtk_entry_get_text(priv->file_entry));
	if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
		filename = gtk_file_chooser_get_filename(dialog);
		gtk_entry_set_text(priv->file_entry, filename);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}
;

MyLoaderSetDialog *my_loader_set_dialog_new(MyLoaderSetting *setting,
		GtkNotebook *notebook) {
	gint i;
	MyLoaderSetDialog *dialog = g_object_new(MY_TYPE_LOADER_SET_DIALOG, NULL);
	if (setting != NULL) {
		MyLoaderSetDialogPrivate *priv =
				my_loader_set_dialog_get_instance_private(dialog);
		gtk_toggle_button_set_active(priv->load_backstage,
				setting->load_backstage);
		gtk_toggle_button_set_active(priv->skip_same_url,
				setting->skip_same_url);
		gtk_toggle_button_set_active(priv->log_process, setting->log);
		gtk_entry_set_text(priv->file_entry, setting->log_file);
		gtk_adjustment_set_value(priv->timeout, setting->timeout);
		gtk_adjustment_set_value(priv->max_count,setting->max_count);
		gtk_box_pack_end(priv->box, notebook, TRUE, TRUE, 0);
		priv->notebook=notebook;
	}
	return dialog;
}
;

void my_loader_set_dialog_get_setting(MyLoaderSetDialog *dialog,
		MyLoaderSetting *setting) {
	if (!MY_IS_LOADER_SET_DIALOG(dialog))
		return;
	MyLoaderSetDialogPrivate *priv = my_loader_set_dialog_get_instance_private(
			dialog);
	setting->load_backstage = gtk_toggle_button_get_active(
			priv->load_backstage);
	setting->skip_same_url= gtk_toggle_button_get_active(
			priv->skip_same_url);
	setting->log = gtk_toggle_button_get_active(priv->log_process);
	setting->log_file = g_strdup(gtk_entry_get_text(priv->file_entry));
	setting->timeout = gtk_adjustment_get_value(priv->timeout);
	setting->max_count=gtk_adjustment_get_value(priv->max_count);
}
;

void my_loader_set_dialog_remove_notebook(MyLoaderSetDialog *dialog,GtkNotebook *notebook){
	MyLoaderSetDialogPrivate *priv = my_loader_set_dialog_get_instance_private(
			dialog);
	g_object_ref(notebook);
	gtk_container_remove(priv->box,notebook);
};

