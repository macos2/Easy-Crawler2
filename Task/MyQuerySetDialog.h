/*
 * MyQuerySetDialog.h
 *
 *  Created on: 2018年3月20日
 *      Author: tom
 */

#ifndef MYTASK_MYQUERYSETDIALOG_H_
#define MYTASK_MYQUERYSETDIALOG_H_
#include <glib-object.h>
#include <gtk/gtk.h>
G_BEGIN_DECLS
typedef struct {
gchar *selector;
gchar *log_file;
GList *output_prop;//GtkLabel 列表
GList *del_out_prop;//GtkLabel 列表
GList *add_out_prop;//Gchar *列表
gboolean auto_scroll;
gboolean wait_for_load_finish;
gboolean log;
gdouble delay;
guint inspect_times,inspect_interval;
}MyQuerySetting;
#define  MY_TYPE_QUERY_SET_DIALOG my_query_set_dialog_get_type()
G_DECLARE_DERIVABLE_TYPE(MyQuerySetDialog,my_query_set_dialog,MY,QUERY_SET_DIALOG,GtkDialog);
typedef struct _MyQuerySetDialogClass{
	GtkDialogClass Parent_class;
};
MyQuerySetting *my_query_set_new();
void my_query_set_dialog_get_set(MyQuerySetDialog *self,MyQuerySetting *set);
void my_query_set_dialog_set_set(MyQuerySetDialog *self,MyQuerySetting *set);
MyQuerySetDialog *my_query_set_dialog_new(MyQuerySetting *set);


G_END_DECLS


#endif /* MYTASK_MYQUERYSETDIALOG_H_ */
