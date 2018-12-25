/*
 * MyJSCmd.h
 *
 *  Created on: 2018年5月2日
 *      Author: tom
 */

#ifndef MYTASK_MYJSCMD_H_
#define MYTASK_MYJSCMD_H_



#include "webkit2/webkit2.h"
#include "JavaScriptCore/JavaScript.h"
#include "task.h"
#include "MyJsCmdSetDialog.h"

G_BEGIN_DECLS
#define MY_TYPE_JS_CMD my_js_cmd_get_type()
G_DECLARE_DERIVABLE_TYPE(MyJsCmd,my_js_cmd,MY,JS_CMD,MyTask);
typedef struct _MyJsCmdClass{
	MyTaskClass parent_class;
};
MyJsCmd *my_js_cmd_new(gchar *name,gchar *script);
G_END_DECLS


#endif /* MYTASK_MYJSCMD_H_ */
