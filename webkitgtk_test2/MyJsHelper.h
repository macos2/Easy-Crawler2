/*
 * Jsvalue_helper.h
 *
 *  Created on: 2018年3月31日
 *      Author: tom
 */

#ifndef MYJSHELPER_H_
#define MYJSHELPER_H_

#include <webkit2/webkit2.h>
#include <JavaScriptCore/JavaScript.h>


gchar * my_js_value_to_gchar(JSContextRef ctx,JSValueRef value){
	if(!JSValueIsString(ctx,value))return NULL;
	JSStringRef js_str=JSValueToStringCopy(ctx,value,NULL);
	gsize size=JSStringGetMaximumUTF8CStringSize(js_str);
	gchar *str=g_malloc(size);
	JSStringGetUTF8CString(js_str,str,size);
	JSStringRelease(js_str);
	return str;
};

JSValueRef my_js_evaluate(JSGlobalContextRef ctx,JSObjectRef this_obj,gchar *script){
if(script==NULL)return NULL;
JSStringRef js_str=JSStringCreateWithUTF8CString(script);
JSValueRef js_res=JSEvaluateScript(ctx,js_str,this_obj,NULL,1,NULL);
JSStringRelease(js_str);
return js_res;
}

JSValueRef *my_js_obj_method(JSGlobalContextRef ctx,JSObjectRef object,gchar *method){
	JSValueRef func,result;
	JSStringRef str;
	JSObjectRef func_obj;
	str=JSStringCreateWithUTF8CString(method);
	func=JSObjectGetProperty(ctx,object,str,NULL);
	JSStringRelease(str);
	func_obj=JSValueToObject(ctx,func,NULL);
	result =JSObjectCallAsFunction(ctx,func_obj,object,0,NULL,NULL);
	return result;
}

JSValueRef *my_js_obj_method_args(JSGlobalContextRef ctx,JSObjectRef object,gchar *method,...){
	JSValueRef func,result,*args;
	JSStringRef str;
	JSObjectRef func_obj;
	gchar *arg=NULL;
	guint n=0,i;
	va_list p;
	va_start(p,method);
	arg=va_arg(p,gchar*);
	while(arg!=NULL){
		n++;
		arg=va_arg(p,gchar*);
	}
	args=g_malloc(n*sizeof(JSValueRef));
	i=0;
	va_start(p,method);
	while(i<n){
	arg=va_arg(p,gchar*);
		str=JSStringCreateWithUTF8CString(arg);
		args[i]=JSValueMakeString(ctx,str);
		JSStringRelease(str);
		i++;
	}
	str=JSStringCreateWithUTF8CString(method);
	func=JSObjectGetProperty(ctx,object,str,NULL);
	JSStringRelease(str);
	func_obj=JSValueToObject(ctx,func,NULL);
	result =JSObjectCallAsFunction(ctx,func_obj,object,n,args,NULL);
	va_end(p);
	g_free(args);
	return result;
}

JSValueRef *my_js_obj_get_prop(JSGlobalContextRef ctx,JSObjectRef obj,gchar *prop){
	JSValueRef value;
	JSStringRef str=JSStringCreateWithUTF8CString(prop);
	value=JSObjectGetProperty(ctx,obj,str,NULL);
	JSStringRelease(str);
	return value;
}

#endif /* MYJSHELPER_H_ */
