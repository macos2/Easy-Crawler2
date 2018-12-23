/*
 * main.c
 *
 *  Created on: 2018年10月7日
 *      Author: tom
 */
#include <stdarg.h>
#include <stdio.h>
#include <webkit2/webkit2.h>
#include <JavaScriptCore/JSBase.h>
#include "Mainui/MyMainUi.h"

WebKitWebView *default_view;

GetTypeFunc my_type[] = { my_dl_get_type, my_filter_get_type, my_js_cmd_get_type,
		my_loader_get_type, my_query_get_type, my_start_get_type,
		NULL };

void main_init(){
	gint i=0;
	GType t;
	start_list = NULL;
	task_list = NULL;
	operater_list=NULL;
	runing_count = 0;
	msg_count=0;
	link_table=g_hash_table_new(g_direct_hash,g_direct_equal);
	my_type_arr=g_array_new(FALSE,TRUE,sizeof(GType));
	while(my_type[i]!=NULL){
		t=my_type[i]();
		g_array_append_val(my_type_arr,t);
		i++;
	}
	WebKitWebContext *ctx=webkit_web_context_get_default();
	webkit_web_context_set_process_model(ctx,WEBKIT_PROCESS_MODEL_MULTIPLE_SECONDARY_PROCESSES);
	default_view=webkit_web_view_new();

}

int main(int argc, char *argv[]) {
	MyMainUi *mainui;

	gtk_init(&argc, &argv);
	main_init();
	mainui = my_main_ui_new();
	main_window = mainui;
	gtk_widget_show_all(mainui);
	g_signal_connect(mainui, "delete_event", gtk_main_quit, NULL);
	gtk_main();
	return 0;
}
