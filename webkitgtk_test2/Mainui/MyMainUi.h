/*
 * MyMainUi.h
 *
 *  Created on: 2018年5月4日
 *      Author: tom
 */

#ifndef MYTASK_MYMAINUI_H_
#define MYTASK_MYMAINUI_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include  "../Operater/MyOperater.h"
G_BEGIN_DECLS
extern GHashTable *link_table;
#define MY_TYPE_MAINUI my_main_ui_get_type()
G_DECLARE_DERIVABLE_TYPE(MyMainUi,my_main_ui,MY,MAIN_UI,GtkWindow);
typedef struct _MyMainUiClass{
	GtkWindowClass parent_class;
	void (*save)(MyMainUi *self);
	void (*load)(MyMainUi *self);
};

MyMainUi *my_main_ui_new();
G_END_DECLS

#endif /* MYTASK_MYMAINUI_H_ */
