/*
 * MyFilter.h
 *
 *  Created on: 2018年4月2日
 *      Author: tom
 */

#ifndef MYTASK_MYFILTER_H_
#define MYTASK_MYFILTER_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "task.h"
G_BEGIN_DECLS
#define  MY_TYPE_FILTER my_filter_get_type()
G_DECLARE_DERIVABLE_TYPE(MyFilter,my_filter,MY,FILTER,MyTask);
typedef struct _MyFilterClass{
MyTaskClass parent_class;
};

MyFilter *my_filter_new(gchar *pattern,gchar *source);
G_END_DECLS

#endif /* MYTASK_MYFILTER_H_ */
