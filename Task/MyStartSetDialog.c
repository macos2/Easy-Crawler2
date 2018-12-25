/*
 * MyStartSetDialogSetDialog.c
 *
 *  Created on: 2018年10月21日
 *      Author: tom
 */

#include "MyStartSetDialog.h"
typedef struct {
	GtkEntry *url;
	GtkBox *view_box;
	WebKitWebView *webview;
	GtkTextView *context_view;
	GtkSwitch *javascript, *auto_load_image, *media_stream, *smooth_scrolling,*plugins;
} MyStartSetDialogPrivate;

G_DEFINE_TYPE_WITH_CODE(MyStartSetDialog, my_start_set_dialog, GTK_TYPE_DIALOG,
		G_ADD_PRIVATE(MyStartSetDialog));

void load_clicked_cb(GtkButton *object, MyStartSetDialog *self) {
	MyStartSetDialogPrivate *priv = my_start_set_dialog_get_instance_private(
			self);
	webkit_web_view_load_uri(priv->webview, gtk_entry_get_text(priv->url));
}
void my_start_set_dialog_dispose(MyStartSetDialog *self) {
	MyStartSetDialogPrivate *priv = my_start_set_dialog_get_instance_private(
			self);
	gtk_container_remove(priv->view_box, priv->webview);
	priv->webview = NULL;
	G_OBJECT_CLASS(my_start_set_dialog_parent_class)->dispose(self);
}

gboolean url_key_press(GtkEntry *entry, GdkEvent *event, MyStartSetDialog *self) {
	MyStartSetDialogPrivate *priv = my_start_set_dialog_get_instance_private(
			self);
	if (event->key.keyval == GDK_KEY_Return) {
		webkit_web_view_load_uri(priv->webview, gtk_entry_get_text(entry));
		return TRUE;
	}
	return FALSE;
}
;

static void my_start_set_dialog_class_init(MyStartSetDialogClass *klass) {
	GObjectClass *obj_class = klass;
	obj_class->dispose = my_start_set_dialog_dispose;
	gtk_widget_class_set_template_from_resource(klass,
			"/mytask/MyStartSetDialog.glade");
	gtk_widget_class_bind_template_child_private(klass, MyStartSetDialog, url);
	gtk_widget_class_bind_template_child_private(klass, MyStartSetDialog,
			view_box);
	gtk_widget_class_bind_template_child_private(klass, MyStartSetDialog,
			context_view);
	gtk_widget_class_bind_template_child_private(klass, MyStartSetDialog,
			javascript);
	gtk_widget_class_bind_template_child_private(klass, MyStartSetDialog,
			auto_load_image);
	gtk_widget_class_bind_template_child_private(klass, MyStartSetDialog,
			media_stream);
	gtk_widget_class_bind_template_child_private(klass, MyStartSetDialog,
			smooth_scrolling);
	gtk_widget_class_bind_template_child_private(klass, MyStartSetDialog,
			plugins);
	gtk_widget_class_bind_template_callback(klass, load_clicked_cb);
	gtk_widget_class_bind_template_callback(klass, url_key_press);

}

static void my_start_set_dialog_init(MyStartSetDialog *self) {
	gtk_widget_init_template(self);
	MyStartSetDialogPrivate *priv = my_start_set_dialog_get_instance_private(
			self);
	priv->webview = NULL;
}

void web_view_load_changed2(WebKitWebView *web_view, WebKitLoadEvent load_event,
		MyStartSetDialog *self) {
	MyStartSetDialogPrivate *priv = my_start_set_dialog_get_instance_private(
			self);
	switch (load_event) {
	case WEBKIT_LOAD_STARTED:
		gtk_entry_set_text(priv->url, webkit_web_view_get_uri(web_view));
		break;
	case WEBKIT_LOAD_REDIRECTED:
		gtk_entry_set_text(priv->url, webkit_web_view_get_uri(web_view));
		break;
	case WEBKIT_LOAD_COMMITTED:
		gtk_window_set_title(self, "Loading...");
		break;
	case WEBKIT_LOAD_FINISHED:
		gtk_window_set_title(self, webkit_web_view_get_title(web_view));
		break;
	default:
		break;
	};
}
;

MyStartSetDialog *my_start_set_dialog_new(WebKitWebView *view,
		GtkTextBuffer *context, GtkWindow *parent) {
	if (!WEBKIT_IS_WEB_VIEW(view))
		return NULL;
	WebKitSettings *view_setting=webkit_web_view_get_settings(view);
	MyStartSetDialog *dialog = g_object_new(MY_TYPE_START_SET_DIALOG, NULL);
	MyStartSetDialogPrivate *priv = my_start_set_dialog_get_instance_private(
			dialog);
	priv->webview = g_object_ref(view);
	gtk_text_view_set_buffer(priv->context_view, context);
	gtk_box_pack_end(priv->view_box, priv->webview, TRUE, TRUE, 0);
	g_signal_connect(priv->webview, "load-changed", web_view_load_changed2,
			dialog);
	g_object_bind_property(view_setting,"enable-javascript",priv->javascript,"active",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(view_setting,"enable-smooth-scrolling",priv->smooth_scrolling,"active",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(view_setting,"enable-media-stream",priv->media_stream,"active",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(view_setting,"auto-load-images",priv->auto_load_image,"active",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(view_setting,"enable-plugins",priv->plugins,"active",G_BINDING_BIDIRECTIONAL);
	gtk_widget_show_all(priv->webview);
	return dialog;
}
;

