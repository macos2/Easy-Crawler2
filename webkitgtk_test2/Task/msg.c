/*
 * msg.c
 *
 *  Created on: 2018年10月7日
 *      Author: tom
 */

#include "msg.h"

TaskMsg * task_msg_new(void *view, gchar *msg, void *from_task, void *to_task) {
	TaskMsg *new = g_malloc0(sizeof(TaskMsg));
	GMutex *mutex = NULL;
	if (WEBKIT_IS_WEB_VIEW(view)) {
		new->view = g_object_ref(view);
		mutex = g_object_get_data(view, "mutex");
		if (mutex == NULL) {
			mutex = g_mutex_new();
			g_object_set_data(view, "mutex", mutex);
		}
	}
	new->msg = g_strdup(msg);
	if (G_IS_OBJECT(to_task))
		new->to_task = g_object_ref(to_task);
	if (G_IS_OBJECT(from_task))
		new->from_task = g_object_ref(from_task);
	new->preserve = NULL;
	msg_count++;
	return new;
}
;

void task_msg_free(TaskMsg *msg) {
	GObject *obj = NULL;
	GMutex *mutex;
	g_free(msg->msg);
	if (G_IS_OBJECT(msg->from_task))
		g_object_unref(msg->from_task);
	if (G_IS_OBJECT(msg->to_task))
		g_object_unref(msg->to_task);
	if (WEBKIT_IS_WEB_VIEW(msg->view)) {
		obj = msg->view;
		mutex = g_object_get_data(msg->view, "mutex");
		if (mutex != NULL)
			g_mutex_unlock(mutex);
		if (obj->ref_count == 1) {
			g_mutex_free(mutex);
		}
		g_object_unref(msg->view);
	}
	g_free(msg);
	msg_count--;
}
;
