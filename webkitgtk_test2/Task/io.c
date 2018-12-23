/*
 * io.c
 *
 *  Created on: 2018年12月16日
 *      Author: tom
 */
#include "io.h"

gchar *read_string(GInputStream *in) {
	gsize size = 0;
	gchar *str = NULL;
	g_input_stream_read(in, &size, sizeof(gsize), NULL, NULL);
	if (size > 0) {
		str = g_malloc0(size+1);
		g_input_stream_read(in, str, size, NULL, NULL);
	}else{
		str=g_strdup("");
	}
	return str;
}
;

void write_string(GOutputStream *out, gchar *str) {
	gsize size=0 ;
	if(str!=NULL)size= strlen(str);
	g_output_stream_write(out, &size, sizeof(gsize), NULL, NULL);
	if (size > 0) {
		g_output_stream_write(out, str, size, NULL, NULL);
	}
}
;

void write_to_file(GOutputStream *out, ...) {
	gulong type;
	gpointer value;
	guint len,i;
	GList *list,*data_list;
	GHashTable *table;
	va_list l;
	va_start(l, out);
	type = va_arg(l, GType);
	while (type != MY_TYPE_NONE) {
		value = va_arg(l, gpointer);
		switch (type) {
		case MY_TYPE_STRING:
			write_string(out, value);
			break;
		case MY_TYPE_GTYPE:
			g_output_stream_write(out, value, sizeof(GType), NULL, NULL);
			break;
		case MY_TYPE_BOOLEAN:
			g_output_stream_write(out, value, sizeof(gboolean), NULL, NULL);
			break;
		case MY_TYPE_UINT:
			g_output_stream_write(out, value, sizeof(guint), NULL, NULL);
			break;
		case MY_TYPE_INT:
			g_output_stream_write(out, value, sizeof(gint), NULL, NULL);
			break;
		case MY_TYPE_DOUBLE:
			g_output_stream_write(out, value, sizeof(gdouble), NULL, NULL);
			break;
		case MY_TYPE_POINTER:
			g_output_stream_write(out, value, sizeof(gpointer), NULL, NULL);
			break;
		case MY_TYPE_GLIST:
			list=g_list_first(value);
			len=g_list_length(list);
			g_output_stream_write(out, &len, sizeof(guint), NULL, NULL);
			while(list!=NULL){
				g_output_stream_write(out, &list->data, sizeof(gpointer), NULL, NULL);
				list=list->next;
			};
			break;
		case MY_TYPE_LINK_TABLE:
			table=value;
			list=g_hash_table_get_keys(table);
			len=g_list_length(list);
			write_to_file(out,MY_TYPE_GLIST,list,MY_TYPE_NONE);
			for(i=0;i<len;i++){
				data_list=g_hash_table_lookup(table,list->data);
				write_to_file(out,MY_TYPE_GLIST,data_list,MY_TYPE_NONE);
				if(list->next!=NULL)list=list->next;
			}
			g_list_free(list);
			break;
		default:
			break;
		}
		type = va_arg(l, GType);
	};
	va_end(l);
}
;

void read_from_file(GInputStream *in, ...) {
	gulong type;
		gpointer value,*temp,data;
		GList *list,*list2,*data_list,*temp_list;
		GHashTable *table;
		guint len=0,i;
		va_list l;
		va_start(l, in);
		type = va_arg(l, GType);
		while (type != MY_TYPE_NONE) {
			value = va_arg(l, gpointer);
			switch (type) {
			case MY_TYPE_STRING:
				temp=value;
				*temp=read_string(in);
				break;
			case MY_TYPE_GTYPE:
				g_input_stream_read(in, value, sizeof(GType), NULL, NULL);
				break;
			case MY_TYPE_BOOLEAN:
				g_input_stream_read(in, value, sizeof(gboolean), NULL, NULL);
				break;
			case MY_TYPE_UINT:
				g_input_stream_read(in, value, sizeof(guint), NULL, NULL);
				break;
			case MY_TYPE_INT:
				g_input_stream_read(in, value, sizeof(gint), NULL, NULL);
				break;
			case MY_TYPE_DOUBLE:
				g_input_stream_read(in, value, sizeof(gdouble), NULL, NULL);
				break;
			case MY_TYPE_POINTER:
				g_input_stream_read(in, value, sizeof(gpointer), NULL, NULL);
				break;
			case MY_TYPE_GLIST:
				list=NULL;
				g_input_stream_read(in, &len, sizeof(guint), NULL, NULL);
				for(i=0;i<len;i++){
					g_input_stream_read(in, &data, sizeof(gpointer), NULL, NULL);
					list=g_list_append(list,data);
				}
				temp=value;
				*temp=g_list_first(list);
				break;
			case MY_TYPE_LINK_TABLE:
				table=g_hash_table_new(g_direct_hash,g_direct_equal);
				read_from_file(in,MY_TYPE_GLIST,&list,MY_TYPE_NONE);
				len=g_list_length(list);
				for(i=0;i<len;i++){
					read_from_file(in,MY_TYPE_GLIST,&data_list,MY_TYPE_NONE);
					list2=data_list;
					temp_list=NULL;
					while(list2!=NULL){
						temp_list=g_list_append(temp_list,g_hash_table_lookup(load_table,list2->data));
						list2=list2->next;
					}
					g_list_free(data_list);
					g_hash_table_insert(link_table,g_hash_table_lookup(load_table,list->data),temp_list);
					if(list->next!=NULL)list=list->next;
				}
				g_list_free(list);
				temp=value;
				*temp=table;
				break;
			default:
				break;
			}
			type = va_arg(l, GType);
		};
		va_end(l);
	}
	;

void io_debug(){
	GList *list=NULL,*table_list[2];
	gboolean b=TRUE;
	gint I=-1234;
	guint U=1234;
	gdouble dig=12.3;
	gchar *str="Hello";
	gchar *r_str=NULL;
	gpointer *r_p=NULL;
	list=g_list_append(list,GUINT_TO_POINTER(9));
	list=g_list_append(list,GUINT_TO_POINTER(5));
	list=g_list_append(list,GUINT_TO_POINTER(4));
	list=g_list_append(list,GUINT_TO_POINTER(3));
	list=g_list_append(list,GUINT_TO_POINTER(2));
	list=g_list_append(list,GUINT_TO_POINTER(1));
	GHashTable *table=g_hash_table_new(g_direct_hash,g_direct_equal);
	g_hash_table_insert(table,GUINT_TO_POINTER(1),list);
	g_hash_table_insert(table,GUINT_TO_POINTER(5),list);
	GFile *file=g_file_new_for_path("test.txt");
	if(g_file_query_exists(file,NULL))unlink("test.txt");
	GOutputStream *out=g_file_create(file,G_FILE_CREATE_REPLACE_DESTINATION,NULL,NULL);
	write_to_file(out,\
			MY_TYPE_BOOLEAN ,&b,\
			MY_TYPE_INT,&I,\
			MY_TYPE_UINT,&U,\
			MY_TYPE_DOUBLE,&dig,\
			MY_TYPE_STRING,str,\
			MY_TYPE_GLIST,list,\
			MY_TYPE_POINTER,&list,\
			MY_TYPE_NONE
	);
	g_output_stream_close(out,NULL,NULL);
	g_print("pointer:%d\nWrite File Finish!!\n",list);
	g_list_free(list);
	g_hash_table_destroy(table);
	b=FALSE;I=0;U=0;dig=0;
	GInputStream *in=g_file_read(file,NULL,NULL);
	read_from_file(in,
			MY_TYPE_BOOLEAN ,&b,\
			MY_TYPE_INT,&I,\
			MY_TYPE_UINT,&U,\
			MY_TYPE_DOUBLE,&dig,\
			MY_TYPE_STRING,&r_str,\
			MY_TYPE_GLIST,&list,\
			MY_TYPE_POINTER,&r_p,\
			MY_TYPE_NONE
	);
	table_list[0]=g_hash_table_lookup(table,GUINT_TO_POINTER(1));
	table_list[1]=g_hash_table_lookup(table,GUINT_TO_POINTER(5));
};
