#include <android/log.h>

int testjni_module2_func()
{
    __android_log_print(ANDROID_LOG_ERROR,"IJKMEDIA","test: module2 called");    
}

int module_init()
{
    __android_log_print(ANDROID_LOG_ERROR, "IJKMEDIA", "test: moduel2 dlopen called");
}

