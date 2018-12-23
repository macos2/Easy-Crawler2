/*
 * MyStartSetDialog.h
 *
 *  Created on: 2018年10月21日
 *      Author: tom
 */

#ifndef TASK_MYSTARTSETDIALOG_H_
#define TASK_MYSTARTSETDIALOG_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include "task.h"
G_BEGIN_DECLS
#define MY_TYPE_START_SET_DIALOG my_start_set_dialog_get_type()
G_DECLARE_DERIVABLE_TYPE(MyStartSetDialog,my_start_set_dialog,MY,START_SET_DIALOG,GtkDialog)
typedef struct _MyStartSetDialogClass{
	GtkDialogClass parent_class;
};

MyStartSetDialog *my_start_set_dialog_new(WebKitWebView *view,GtkTextBuffer *context,GtkWindow *parent);
G_END_DECLS


#endif /* TASK_MYSTARTSETDIALOG_H_ */
