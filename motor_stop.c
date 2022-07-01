#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

const char *enableMotorStop="/opt/wz_mini/tmp/.ms";
static uint32_t (*real_local_sdk_motor_init)(void);

local_sdk_motor_init(void) {
	if( access( enableMotorStop, F_OK ) != -1 ) {
		printf("[command] [motor_stop.c] Motor disabled\n");
		return 0;
	} else {
               printf("[command] [motor_stop.c] Motor enabled!\n");
		real_local_sdk_motor_init();
	}
}

static void __attribute ((constructor)) motor_stop_init(void) {
	real_local_sdk_motor_init = dlsym(dlopen("/system/lib/liblocalsdk_motor.so", RTLD_LAZY), "local_sdk_motor_init");
}
