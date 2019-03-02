/*
 * my_download.h
 *
 *  Created on: 2018年3月12日
 *      Author: tom
 */

#ifndef MY_DOWNLOAD_H_
#define MY_DOWNLOAD_H_
#include <glib-object.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include "../gresource/gresource.h"
#include "io.h"
G_BEGIN_DECLS
typedef enum {
	MY_DOWNLOAD_DOWNLOADING,
	MY_DOWNLOAD_WAIT,
	MY_DOWNLOAD_STOP,
	MY_DOWNLOAD_FINISH,
	MY_DOWNLOAD_ERROR,
}my_download_state;

typedef enum  {
	same_filename_skip,
	same_filename_over_write,
	same_filename_rename_add_suffix
}same_filename_operation;

typedef struct {
	guchar state;
	gchar *name;
	gchar *url;
	guint progress;
	guint time_elapsed;
	GDate *start_time;
	gsize size;
}Download_State;

typedef struct{
	gchar *global_prefix,*global_suffix,*save_local;
	gboolean u_prefix,u_suffix,u_dir,auto_backup,skip_same_url;
	same_filename_operation same_op;
}MyDownloadSetting;

#define MY_TYPE_DOWNLOAD my_download_get_type()
G_DECLARE_DERIVABLE_TYPE(MyDownload,my_download,MY,DOWNLOAD,GtkWindow);
typedef struct _MyDownloadClass{
	GtkWindowClass parent_class;
	void (*download_finish)(MyDownload *self,WebKitDownload *download,GtkListStore *download_store,GtkTreeRowReference *row_ref);
	void (*download_start)(MyDownload *self,WebKitDownload *download,GtkListStore *download_store,GtkTreeRowReference *row_ref);
};
MyDownload * my_download_new(WebKitWebView *web_view, gchar *save_dir,gchar *prefix, gchar *suffix);
void my_download_set(MyDownload *self,gchar *save_dir,gchar *prefix, gchar *suffix);
void my_download_add(MyDownload *self ,gchar *url,gchar *prefix,gchar *suffix,gchar *dir);
void my_download_add_webkitdownload(MyDownload *self ,WebKitDownload *download);
Download_State *my_download_get_download_state(MyDownload *self,WebKitDownload *download);
gint my_download_get_downloading_count(MyDownload *self);
WebKitWebView *my_download_get_web_view(MyDownload *self);
void my_download_stop_all(MyDownload *self);
void my_download_start_all(MyDownload *self);
MyDownloadSetting *my_download_get_setting(MyDownload *self);
void my_download_set_setting(MyDownload *self,MyDownloadSetting *set);
void my_download_setting_free(MyDownloadSetting *set);
G_END_DECLS


#endif /* MY_DOWNLOAD_H_ */
