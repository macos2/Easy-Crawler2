/*
 * MyStart.h
 *
 *  Created on: 2018年10月21日
 *      Author: tom
 */

#ifndef TASK_MYSTART_H_
#define TASK_MYSTART_H_

#include "MyStartSetDialog.h"
G_BEGIN_DECLS
//extern WebKitWebView *default_view;
extern GList *start_list;
#define MY_TYPE_START my_start_get_type()
G_DECLARE_DERIVABLE_TYPE(MyStart,my_start,MY,START,MyTask)
typedef struct _MyStartClass{
	MyTaskClass parent_class;
};

MyStart *my_start_new(void);
G_END_DECLS

#endif /* TASK_MYSTART_H_ */
