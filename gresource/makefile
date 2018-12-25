all:gresource.h gresource.c
gresource.h:MyDownload.glade MyOperater.glade MyQuery.glade MyQuerySetDialog.glade MyFilter.glade MyFilterSetDialog.glade MyJsCmd.glade MyJsCmdSetDialog.glade MyMainUi.glade MyLoader.glade MyLoaderSetDialog.glade gresource.xml MyDl.glade MyTask.glade MyStart.glade MyStartSetDialog.glade
	glib-compile-resources --generate-header --target=gresource.h gresource.xml
gresource.c:MyDownload.glade MyOperater.glade MyQuery.glade MyQuerySetDialog.glade MyFilter.glade MyFilterSetDialog.glade MyJsCmd.glade MyJsCmdSetDialog.glade MyMainUi.glade MyLoader.glade MyLoaderSetDialog.glade gresource.xml MyDl.glade MyTask.glade MyStart.glade MyStartSetDialog.glade
	glib-compile-resources --generate-source --target=gresource.c gresource.xml
