#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "platform.h"

const char *enableMotorStop="/opt/wz_mini/tmp/.ms";
const char *product_T20="/opt/wz_mini/tmp/.T20";

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
	if( access( product_T20, F_OK ) != -1 ) {
		real_local_sdk_motor_init = dlsym(dlopen("/system/lib/liblocalsdk.so", RTLD_LAZY), "sdk_motor_init");
	} else {
		real_local_sdk_motor_init = dlsym(dlopen("/system/lib/liblocalsdk_motor.so", RTLD_LAZY), "local_sdk_motor_init");
	}
}
