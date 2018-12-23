/*
 * MyLoaderSetDialog.h
 *
 *  Created on: 2018年5月7日
 *      Author: tom
 */

#ifndef MYTASK_MYLOADERSETDIALOG_H_
#define MYTASK_MYLOADERSETDIALOG_H_
#include "glib-object.h"
#include "gtk/gtk.h"
G_BEGIN_DECLS
typedef struct {
	gboolean log;
	gchar *log_file;
	gboolean load_backstage,skip_same_url;
	gdouble timeout;
	guint max_count;
}MyLoaderSetting;
#define MY_TYPE_LOADER_SET_DIALOG my_loader_set_dialog_get_type()
G_DECLARE_DERIVABLE_TYPE(MyLoaderSetDialog,my_loader_set_dialog,MY,LOADER_SET_DIALOG,GtkDialog);
typedef struct _MyLoaderSetDialogClass{
	GtkDialogClass Parent_Class;
};

MyLoaderSetDialog *my_loader_set_dialog_new(MyLoaderSetting *setting,GtkNotebook *notebook);
void my_loader_set_dialog_get_setting(MyLoaderSetDialog *dialog ,MyLoaderSetting *setting);
void my_loader_set_dialog_remove_notebook(MyLoaderSetDialog *dialog,GtkNotebook *notebook);
G_END_DECLS


#endif /* MYTASK_MYLOADERSETDIALOG_H_ */
