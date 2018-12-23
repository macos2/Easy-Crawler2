/*
 * global.h
 *
 *  Created on: 2018年12月17日
 *      Author: tom
 */

#ifndef TASK_GLOBAL_H_
#define TASK_GLOBAL_H_

#include <glib.h>
GHashTable *link_table,*load_table;
void *main_window;
GList *start_list;
GList *task_list,*operater_list;//储存task、operater的列表
GArray *my_type_arr;
gboolean MAIN_START;
gint runing_count;
gint msg_count;
guint complete_check;
typedef GType (*GetTypeFunc)() ;


#endif /* TASK_GLOBAL_H_ */
