/*
 * msg.h
 *
 *  Created on: 2018年10月7日
 *      Author: tom
 */

#ifndef TASK_MSG_H_
#define TASK_MSG_H_
#include <glib.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include "global.h"

typedef struct {
	void *view;
	gchar *msg;
	void *from_task;
	void *to_task;
	void *preserve;
} TaskMsg; //任务间的信息,接收方处理后将其释放

typedef struct {
	void *source;
	void *target;
	guint id;
} TaskLink;

TaskMsg * task_msg_new(void *view, gchar *msg, void *from_task, void *to_task);
void task_msg_free(TaskMsg *msg);


#endif /* TASK_MSG_H_ */
