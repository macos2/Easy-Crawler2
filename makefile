build:$(shell find *.c */*.c */*.h)
		cd build&&ninja
all:$(shell find *.c */*.c */*.h)
		meson build
		cd build&&ninja