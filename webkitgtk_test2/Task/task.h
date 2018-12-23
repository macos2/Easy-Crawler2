/*
 * task.h
 *
 *  Created on: 2018年10月7日
 *      Author: tom
 */

#ifndef TASK_TASK_H_
#define TASK_TASK_H_

#include <glib-object.h>
#include "msg.h"
#include "io.h"
#include "global.h"
#include "../gresource/gresource.h"

G_BEGIN_DECLS
#define MY_TASK_PROP_NAME "name"
#define MY_TASK_PROP_LOG "log"
#define MY_TASK_PROP_LOG_FILE "log_file"
#define MY_TASK_RIGHT_CLICKED_WIDGET "right_clicked_widget"
#define MY_TYPE_TASK my_task_get_type()

G_DECLARE_DERIVABLE_TYPE(MyTask, my_task, MY, TASK, GtkBox);

typedef struct _MyTaskClass {
	GtkBoxClass parent_class;
	void (*run_init)(MyTask *self);
	gchar* (*run)(MyTask *self, TaskMsg *msg);
	void (*run_finish)(MyTask *self);
	void (*save)(MyTask *self,GOutputStream *out);
	void (*load)(MyTask *self,GInputStream *in);
	void (*stop_run)(MyTask *self);
};

void my_task_run_init(MyTask *self);
void my_task_run(MyTask *self, TaskMsg *msg);
void my_task_send_msg(MyTask *self, MyTask *target, void *view, gchar* context);
void my_task_run_finish(MyTask *self);
void my_task_save(MyTask *self,GOutputStream *out);
void my_task_load(MyTask *self,GInputStream *in);
void my_task_stop_run(MyTask *self);
gboolean my_task_log(MyTask *self, gchar *format, ...);
void my_task_rename_id(MyTask *task, gchar *name, uint id);
MyTask *my_task_get_next_task(MyTask *task);

//task drag and drog default operation setting;
void my_task_drag_dest_set(GtkWidget *widget);
void my_task_drag_source_set(GtkWidget *widget);
void my_task_drag_data_get(GtkWidget *widget, GdkDragContext *context,
		GtkSelectionData *selection_data, guint info, guint time_);
void my_task_drag_data_received(GtkWidget *widget, GdkDragContext *context,
		gint x, gint y, GtkSelectionData *selection_data, guint info,
		guint time_);
void my_task_set_right_clicked(GtkWidget *widget, MyTask *task);

GIOChannel *my_task_get_log_io(MyTask *self);

G_END_DECLS

#endif /* TASK_TASK_H_ */
