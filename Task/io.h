/*
 * io.h
 *
 *  Created on: 2018年12月16日
 *      Author: tom
 */

#ifndef TASK_IO_H_
#define TASK_IO_H_
#include <glib.h>
#include <gio/gio.h>
#include "global.h"
G_BEGIN_DECLS
enum{
	MY_TYPE_GTYPE,
	MY_TYPE_BOOLEAN,
	MY_TYPE_INT,
	MY_TYPE_UINT,
	MY_TYPE_POINTER,
	MY_TYPE_DOUBLE,
	MY_TYPE_STRING,
	MY_TYPE_GLIST,//GList
//	MY_TYPE_DD_HASH_TABLE,//direct to direct GHashTable
	MY_TYPE_LINK_TABLE,
	MY_TYPE_NONE,
}MY_TYPE;



gchar *read_string(GInputStream *in);
void write_string(GOutputStream *out,gchar *str);
void write_to_file(GOutputStream *out,...);//MY_TYPE,data-Pointer ...,MY_TYPE_NONE for end
void read_from_file(GInputStream *in,...);//MY_TYPE,data-Pointer ...,MY_TYPE_NONE for end
void io_debug();//Use for test or debug the io function;
G_END_DECLS
#endif /* TASK_IO_H_ */
