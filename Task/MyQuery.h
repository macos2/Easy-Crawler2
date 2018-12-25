/*
 * my_query.h
 *
 *  Created on: 2018年3月19日
 *      Author: tom
 */

#ifndef MYTASK_MYQUERY_H_
#define MYTASK_MYQUERY_H_

#include <stdarg.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <JavaScriptCore/JavaScript.h>
//#include <jsc/jsc.h>
#include "MyQuerySetDialog.h"
#include "task.h"
#include "../gresource/gresource.h"

G_BEGIN_DECLS
#define MY_TYPE_QUERY my_query_get_type()
G_DECLARE_DERIVABLE_TYPE(MyQuery,my_query,MY,QUERY,MyTask);
 typedef struct _MyQueryClass{
	 MyTaskClass Parent_class;
 };
MyQuery *my_query_new(WebKitWebView *web_view, gchar *selector, ...);
G_END_DECLS

#endif /* MYTASK_MYQUERY_H_ */
