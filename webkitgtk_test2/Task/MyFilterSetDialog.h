/*
 * MyFilterSetDialog.h
 *
 *  Created on: 2018年4月2日
 *      Author: tom
 */

#ifndef MYTASK_MYFILTERSETDIALOG_H_
#define MYTASK_MYFILTERSETDIALOG_H_
#include <glib-object.h>
#include <gtk/gtk.h>
G_BEGIN_DECLS
typedef struct {
	gchar *regex_pattern;
	gchar *test_source;
	gboolean log_result;
	gchar *log_file,*prefix,*suffix;
}MyFilterSetting;
#define  MY_TYPE_FILTER_SET_DIALOG my_filter_set_dialog_get_type()
G_DECLARE_DERIVABLE_TYPE(MyFilterSetDialog,my_filter_set_dialog,MY,MY_FILTER_SET_DIALOG,GtkDialog)
typedef struct _MyFilterSetDialogClass{
	GtkDialogClass parent_class;
};

gboolean my_filter_set_dialog_set_setting(MyFilterSetDialog *self,MyFilterSetting *set);
gboolean my_filter_set_dialog_get_setting(MyFilterSetDialog *self,MyFilterSetting *set);
G_END_DECLS

#endif /* MYTASK_MYFILTERSETDIALOG_H_ */
