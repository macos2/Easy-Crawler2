/*
 * MyQuerySetDialog.c
 *
 *  Created on: 2018年3月20日
 *      Author: tom
 */

#include "MyQuerySetDialog.h"

typedef struct {
	GtkEntry *selector_entry, *add_prop_entry;
	GtkTreeView *prop_treeview;
	GtkListStore *prop_store;
	GtkCheckButton *auto_scroll, *wait_finish, *log_result;
	GtkAdjustment *delay_adj,*inspect_times,*inspect_interval;
	GtkMenu *menu;
	GtkDialog *add_output_prop_dialog;
	GList *add_output_prop;
	GList *del_output_prop;
	GtkEntry *file_entry;
} MyQuerySetDialogPrivate;

typedef enum{
	col_prop_name,
	col_prop_label
}OutPutColumn;

G_DEFINE_TYPE_WITH_CODE(MyQuerySetDialog, my_query_set_dialog, GTK_TYPE_DIALOG,
		G_ADD_PRIVATE(MyQuerySetDialog));
void my_query_set_dialog_finalize(MyQuerySetDialog *self);
void menu_add(GtkMenuItem *item, MyQuerySetDialog *self);
void menu_del(GtkMenuItem *item, MyQuerySetDialog *self);
gboolean treeview_button_press(GtkWidget *view, GdkEventButton *event,
		MyQuerySetDialog *self);
void  my_query_file_select_clicked (GtkButton *file_select, MyQuerySetDialog *self);

static void my_query_set_dialog_class_init(MyQuerySetDialogClass *klass) {
	GObjectClass *obj_class=klass;
	obj_class->finalize=my_query_set_dialog_finalize;
	gtk_widget_class_set_template_from_resource(klass,
			"/mytask/MyQuerySetDialog.glade");
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			selector_entry);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			prop_treeview);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			prop_store);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			auto_scroll);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			wait_finish);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			log_result);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			delay_adj);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog, menu);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			add_prop_entry);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			add_output_prop_dialog);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			file_entry);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			inspect_times);
	gtk_widget_class_bind_template_child_private(klass, MyQuerySetDialog,
			inspect_interval);
	gtk_widget_class_bind_template_callback(klass, menu_add);
	gtk_widget_class_bind_template_callback(klass, menu_del);
	gtk_widget_class_bind_template_callback(klass, treeview_button_press);
	gtk_widget_class_bind_template_callback(klass, my_query_file_select_clicked);

}

static void my_query_set_dialog_init(MyQuerySetDialog *self) {
	gtk_widget_init_template(self);
	MyQuerySetDialogPrivate *priv = my_query_set_dialog_get_instance_private(
			self);
	priv->del_output_prop=NULL;
	priv->add_output_prop=NULL;
}

void my_query_set_dialog_finalize(MyQuerySetDialog *self){
	MyQuerySetDialogPrivate *priv = my_query_set_dialog_get_instance_private(
			self);
	//g_print("my_query_set_dialog_finalize\n");
	//if(priv->add_output_prop!=NULL)g_list_free_full(priv->add_output_prop,g_free);
	//if(priv->del_output_prop!=NULL)g_list_free(priv->del_output_prop);
	//g_print("my_query_set_dialog_finalize_finish\n");
};

void menu_add(GtkMenuItem *item, MyQuerySetDialog *self) {
	MyQuerySetDialogPrivate *priv = my_query_set_dialog_get_instance_private(
			self);
	if (gtk_dialog_run(priv->add_output_prop_dialog) == GTK_RESPONSE_OK) {
		GtkTreeIter iter;
		GValue value = G_VALUE_INIT;
		gchar *str = gtk_entry_get_text(priv->add_prop_entry);
		priv->add_output_prop=g_list_append(priv->add_output_prop,g_strdup(str));
		g_value_init(&value, G_TYPE_STRING);
		g_value_set_static_string(&value, str);
		gtk_list_store_append(priv->prop_store, &iter);
		gtk_list_store_set_value(priv->prop_store, &iter, col_prop_name, &value);
		g_value_unset(&value);
	}
}
;
void menu_del(GtkMenuItem *item, MyQuerySetDialog *self) {
	MyQuerySetDialogPrivate *priv = my_query_set_dialog_get_instance_private(
			self);
	GtkTreeSelection *sel = gtk_tree_view_get_selection(priv->prop_treeview);
	GList *list = gtk_tree_selection_get_selected_rows(sel, &priv->prop_store),
			*list_rows = NULL;
	GtkTreePath *path;
	GtkTreeIter iter;
	GValue value=G_VALUE_INIT;
	guint i = g_list_length(list), j = 0;
	for (j = 0; j < i; j++) {
		path = list->data;
		list_rows = g_list_append(list_rows,
				gtk_tree_row_reference_new(priv->prop_store, path));
		if (list->next != NULL)
			list = list->next;
	}
	g_list_free_full(list, gtk_tree_path_free);
	for (j = 0; j < i; j++) {
		path = gtk_tree_row_reference_get_path(list_rows->data);
		gtk_tree_model_get_iter(priv->prop_store, &iter, path);
		gtk_tree_model_get_value(priv->prop_store,&iter,col_prop_label,&value);
		priv->del_output_prop=g_list_append(priv->del_output_prop,g_value_get_pointer(&value));
		g_value_unset(&value);
		gtk_list_store_remove(priv->prop_store, &iter);
		gtk_tree_path_free(path);
		if (list_rows->next != NULL)
			list_rows = list_rows->next;
	}
	g_list_free_full(list_rows, gtk_tree_row_reference_free);
}
;
gboolean treeview_button_press(GtkWidget *view, GdkEventButton *event,
		MyQuerySetDialog *self) {
	MyQuerySetDialogPrivate *priv = my_query_set_dialog_get_instance_private(
			self);
	if (event->button == 3) {
		gtk_menu_popup_at_pointer(priv->menu, NULL);
		return TRUE;
	}
	return FALSE;
}
;

void my_query_file_select_clicked(GtkButton *button,MyQuerySetDialog *self){
	gchar *filename;
	MyQuerySetDialogPrivate *priv = my_query_set_dialog_get_instance_private(
			self);
	GtkDialog *dialog=gtk_file_chooser_dialog_new("Log File",self,GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_OK,GTK_RESPONSE_OK,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,NULL);
	gtk_file_chooser_set_filename(dialog,gtk_entry_get_text(priv->file_entry));
	if(gtk_dialog_run(dialog)==GTK_RESPONSE_OK){
		filename=gtk_file_chooser_get_filename(dialog);
		gtk_entry_set_text(priv->file_entry,filename);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
};

MyQuerySetting *my_query_set_new() {
	MyQuerySetting *set = g_malloc(sizeof(MyQuerySetting));
	set->selector=g_strdup("");
	set->log=g_strdup("");
	set->output_prop = NULL;
	set->selector = NULL;
	set->del_out_prop=NULL;
	set->add_out_prop=NULL;
	return set;
}
;

void my_query_set_dialog_get_set(MyQuerySetDialog *self, MyQuerySetting *set) {
	MyQuerySetDialogPrivate *priv = my_query_set_dialog_get_instance_private(
			self);
	set->auto_scroll = gtk_toggle_button_get_active(priv->auto_scroll);
	set->wait_for_load_finish = gtk_toggle_button_get_active(priv->wait_finish);
	set->delay = gtk_adjustment_get_value(priv->delay_adj);
	if (set->selector != NULL)
		g_free(set->selector);
	set->selector = g_strdup(gtk_entry_get_text(priv->selector_entry));
	set->log = gtk_toggle_button_get_active(priv->log_result);
	set->add_out_prop=priv->add_output_prop;
	set->del_out_prop=priv->del_output_prop;
	set->log_file=g_strdup(gtk_entry_get_text(priv->file_entry));
	set->inspect_interval=(guint)gtk_adjustment_get_value(priv->inspect_interval);
	set->inspect_times=(guint)gtk_adjustment_get_value(priv->inspect_times);
}
;
void my_query_set_dialog_set_set(MyQuerySetDialog *self,MyQuerySetting *set) {
	MyQuerySetDialogPrivate *priv = my_query_set_dialog_get_instance_private(
			self);
	GtkTreeIter iter;
	GValue value = G_VALUE_INIT;
	gint i = 0;
	guint j = 0;
	if (set != NULL) {
		gtk_entry_set_text(priv->selector_entry, set->selector);
		gtk_adjustment_set_value(priv->delay_adj, set->delay);
		gtk_adjustment_set_value(priv->inspect_interval,set->inspect_interval);
		gtk_adjustment_set_value(priv->inspect_times,set->inspect_times);
		gtk_toggle_button_set_active(priv->auto_scroll, set->auto_scroll);
		gtk_toggle_button_set_active(priv->wait_finish,
				set->wait_for_load_finish);
		gtk_toggle_button_set_active(priv->log_result, set->log);
		gtk_entry_set_text(priv->file_entry,set->log_file);
		if (set->output_prop != NULL) {
			j = g_list_length(set->output_prop);
			GList *p=g_list_first(set->output_prop);
			while (i < j) {
				gtk_list_store_append(priv->prop_store, &iter);
				//设置列表的name列（Gpointer）
				g_value_init(&value,G_TYPE_STRING);
				g_value_set_string(&value,
						gtk_button_get_label(p->data));
				gtk_list_store_set_value(priv->prop_store, &iter, col_prop_name, &value);
				g_value_unset(&value);
				//设置列表的label列（Gpointer）
				g_value_init(&value,G_TYPE_POINTER);
				g_value_set_pointer(&value,p->data);
				gtk_list_store_set_value(priv->prop_store, &iter, col_prop_label, &value);
				g_value_unset(&value);
				p=p->next;
				i++;
			}
		}
	}
}
;
MyQuerySetDialog *my_query_set_dialog_new(MyQuerySetting *set) {
	MyQuerySetDialog *self = g_object_new(MY_TYPE_QUERY_SET_DIALOG, NULL);
	if (set != NULL) {
		my_query_set_dialog_set_set(self, set);
	};
	return self;
}
;
