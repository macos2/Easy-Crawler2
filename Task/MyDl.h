/*
 * MyDl.h
 *
 *  Created on: 2018年10月15日
 *      Author: tom
 */

#ifndef TASK_MYDL_H_
#define TASK_MYDL_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "../gresource/gresource.h"
#include "task.h"
#include "MyDownload.h"
G_BEGIN_DECLS
#define MY_TYPE_DL my_dl_get_type()
G_DECLARE_DERIVABLE_TYPE(MyDl,my_dl,MY,DL,MyTask);
static struct _MyDlClass{
	MyTaskClass parent_class;
};

MyDl *my_dl_new();

G_END_DECLS
#endif /* TASK_MYDL_H_ */
