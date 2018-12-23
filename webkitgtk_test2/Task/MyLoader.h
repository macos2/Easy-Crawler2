/*
 * MyOpener.h
 *
 *  Created on: 2018年5月6日
 *      Author: tom
 */

#ifndef MYTASK_MYLOADER_H_
#define MYTASK_MYLOADER_H_

#include "task.h"
#include "MyLoaderSetDialog.h"
#include <webkit2/webkit2.h>
G_BEGIN_DECLS
#define  MY_TYPE_LOADER my_loader_get_type()
G_DECLARE_DERIVABLE_TYPE(MyLoader,my_loader,MY,LOADER,MyTask);
typedef struct _MyLoaderClass{
	MyTaskClass Parent_class;
};

MyLoader *my_loader_new();
G_END_DECLS


#endif /* MYTASK_MYLOADER_H_ */
