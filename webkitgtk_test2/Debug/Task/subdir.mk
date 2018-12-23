################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Task/MyDl.c \
../Task/MyDownload.c \
../Task/MyFilter.c \
../Task/MyFilterSetDialog.c \
../Task/MyJsCmd.c \
../Task/MyJsCmdSetDialog.c \
../Task/MyLoader.c \
../Task/MyLoaderSetDialog.c \
../Task/MyQuery.c \
../Task/MyQuerySetDialog.c \
../Task/MyStart.c \
../Task/MyStartSetDialog.c \
../Task/io.c \
../Task/msg.c \
../Task/task.c 

OBJS += \
./Task/MyDl.o \
./Task/MyDownload.o \
./Task/MyFilter.o \
./Task/MyFilterSetDialog.o \
./Task/MyJsCmd.o \
./Task/MyJsCmdSetDialog.o \
./Task/MyLoader.o \
./Task/MyLoaderSetDialog.o \
./Task/MyQuery.o \
./Task/MyQuerySetDialog.o \
./Task/MyStart.o \
./Task/MyStartSetDialog.o \
./Task/io.o \
./Task/msg.o \
./Task/task.o 

C_DEPS += \
./Task/MyDl.d \
./Task/MyDownload.d \
./Task/MyFilter.d \
./Task/MyFilterSetDialog.d \
./Task/MyJsCmd.d \
./Task/MyJsCmdSetDialog.d \
./Task/MyLoader.d \
./Task/MyLoaderSetDialog.d \
./Task/MyQuery.d \
./Task/MyQuerySetDialog.d \
./Task/MyStart.d \
./Task/MyStartSetDialog.d \
./Task/io.d \
./Task/msg.d \
./Task/task.d 


# Each subdirectory must supply rules for building sources it contributes
Task/%.o: ../Task/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/webkitgtk-4.0 -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/gtk-3.0 -I/usr/include/pango-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/harfbuzz -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/gio-unix-2.0 -I/usr/include/atk-1.0 -I/usr/include/at-spi2-atk/2.0 -I/usr/include/at-spi-2.0 -I/usr/include/dbus-1.0 -I/usr/lib64/dbus-1.0/include -I/usr/include/libsoup-2.4 -I/usr/include/libxml2 -O0 -g3 -w -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


