/*
 * MyJsCmdSetDialog.h
 *
 *  Created on: 2018年5月2日
 *      Author: tom
 */

#ifndef MYTASK_MYJSCMDSETDIALOG_H_
#define MYTASK_MYJSCMDSETDIALOG_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "../gresource/gresource.h"
G_BEGIN_DECLS

typedef struct{
	gchar *log_file;
	gchar *javascript;
	gchar *script_name;
	gboolean *log;
}MyJsCmdSetting;

#define MY_TYPE_JS_CMD_SET_DIALOG my_js_cmd_set_dialog_get_type()
G_DECLARE_DERIVABLE_TYPE(MyJsCmdSetDialog,my_js_cmd_set_dialog,MY,JS_CMD_SET_DIALOG,GtkDialog)

typedef struct _MyJsCmdSetDialogClass{
	GtkDialogClass parent_class;
};

MyJsCmdSetDialog * my_js_cmd_set_dialog_new();
void my_js_cmd_set_dialog_set_setting(MyJsCmdSetDialog * dialog,MyJsCmdSetting *setting);
MyJsCmdSetting *my_js_cmd_set_dialog_get_setting(MyJsCmdSetDialog * dialog);
MyJsCmdSetting *my_js_cmd_setting_new(gchar *script_name,gchar *script);
void my_js_cmd_setting_free(MyJsCmdSetting *setting);
G_END_DECLS
#endif /* MYTASK_MYJSCMDSETDIALOG_H_ */
