/*
 * my_operater.h
 *
 *  Created on: 2018年3月18日
 *      Author: tom
 */

#ifndef MYOPERATER_MYOPERATER_H_
#define MYOPERATER_MYOPERATER_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include "../Task/task.h"
#include "../Task/MyDl.h"
#include "../Task/MyFilter.h"
#include "../Task/MyJsCmd.h"
#include "../Task/MyLoader.h"
#include "../Task/MyQuery.h"
#include "../Task/MyStart.h"

G_BEGIN_DECLS
#define MY_TYPE_OPERATER my_operater_get_type()
G_DECLARE_DERIVABLE_TYPE(MyOperater,my_operater,MY,OPERATER,GtkBox);

typedef struct _MyOperaterClass{
GtkBoxClass Parent_class;
};

MyOperater *my_operater_new(gchar *title);
void my_operater_set_layout(MyOperater *operater,GtkLayout *layout);
void my_operater_set_title(MyOperater *operater,const gchar *str);
const gchar *my_operater_get_title(MyOperater *operater);
G_END_DECLS



#endif /* MYOPERATER_MYOPERATER_H_ */
