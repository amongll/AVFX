#include <android/log.h>
#include <jni.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int testjni_module1_func()
{
    __android_log_print(ANDROID_LOG_ERROR,"IJKMEDIA","test:");
    prebuild_shared_ndkbuild_func(); 
    prebuild_shared_toolchain_func();
}

struct MyCtx {
    jclass clazz;
} gCtx;


static bool JNI_CatchJVMException(JNIEnv* env) {
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return true;
    }

    return false;
}

#define FIND_JAVA_CLASS(env__, var__, classsign__) \
do { \
	jclass clazz = (*env__)->FindClass(env__, classsign__); \
	if (JNI_CatchJVMException(env) || !(clazz)) { \
		return -1; \
	} \
	var__ = (*env__)->NewGlobalRef(env__, clazz); \
	if (JNI_CatchJVMException(env) || !(var__)) { \
		(*env__)->DeleteLocalRef(env__, clazz); \
		return -1; \
	} \
	(*env__)->DeleteLocalRef(env__, clazz); \
} while(0);


#define FIND_JAVA_METHOD(env__, var__, clazz__, name__, sign__) \
do { \
	(var__) = (*env__)->GetMethodID((env__), (clazz__), (name__), (sign__)); \
	if (JNI_CatchJVMException(env) || !(var__)) { \
		return -1; \
	} \
} while(0);

#define FIND_JAVA_FIELD(env__, var__, clazz__, name__, sign__ ) \
do {\
	(var__) = (*env__)->GetFieldID((env__),(clazz__), (name__), (sign__));\
	if (JNI_CatchJVMException(env) || !(var__)) { \
		return -1; \
	} \
}while(0);

#define JNTTESTCLASS "org/lilei/testjni"

static jstring JNICall1(JNIEnv* env, jobject thiz)
{
    __android_log_print(ANDROID_LOG_ERROR,"IJKMEDIA","call1:");
    prebuild_shared_ndkbuild_func();
    return (*env)->NewStringUTF(env,"testjni_native_module1 call1 ndkbuilded prebuild lib");
}

static jstring JNICall2(JNIEnv* env, jobject thiz)
{
    __android_log_print(ANDROID_LOG_ERROR,"IJKMEDIA","call2:");
    prebuild_shared_toolchain_func();
    return (*env)->NewStringUTF(env,"testjni_native_module1 call1 standalone prebuild lib");
}

static jstring JNICallGetPwd(JNIEnv* env, jobject thiz)
{
    char buf[1024];
    getcwd(buf,sizeof(buf));
    __android_log_print(ANDROID_LOG_ERROR,"IJKMEDIA","getpwd:%s", buf);
    return (*env)->NewStringUTF(env,buf);
}

typedef int (*module_call_fp)();

static jstring JNICallDlopen(JNIEnv* env, jobject thiz)
{
    int flags = RTLD_NOW;
    void* handle = dlopen("libtestjni_module2.so", flags);
    if ( handle == NULL ) {
        __android_log_print(ANDROID_LOG_ERROR, "IJKMEDIA","dlopen failed:%s", strerror(errno));
        return NULL;
    }

    module_call_fp fp = dlsym(handle, "module_init");

    fp();
    return (*env)->NewStringUTF(env,"testjni_native_module1 call1 dlopened module2 lib");
}

static void  JNICatFile(JNIEnv* env, jobject thiz, jstring jpath) {
   const char *path = (*env)->GetStringUTFChars(env, jpath, NULL);
    int fd = open(path,O_RDONLY );
    if ( fd == -1) {
        __android_log_print(ANDROID_LOG_ERROR,"IJKMEDIA", "open file %s failed:%s", path, strerror(errno));
        return;
    }

    char buf[1024];
    int ret = -1;
    do {
        ret = read(fd, buf, sizeof(buf));
        if (ret > 0 ) {
            __android_log_print(ANDROID_LOG_ERROR, "IJKMEDIA", "content:%s", buf);
        }
    } while (ret > 0);

    close(fd);
    return;

}

static JNINativeMethod gMethods[] = {
        {"call1", "()Ljava/lang/String;", (void*)JNICall1},
        {"call2", "()Ljava/lang/String;", (void*)JNICall2},
        {"call_dlopened_module2", "()Ljava/lang/String;", (void*)JNICallDlopen},
        {"getPwd", "()Ljava/lang/String;", (void*)JNICallGetPwd},
        {"catfile", "(Ljava/lang/String;)V", (void*)JNICatFile}
};

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    if ( (*vm)->GetEnv(vm, (void**)&env,JNI_VERSION_1_4) != JNI_OK )
        return -1;

    FIND_JAVA_CLASS(env, gCtx.clazz, JNTTESTCLASS);

    (*env)->RegisterNatives(env, gCtx.clazz, gMethods, sizeof(gMethods)/sizeof(JNINativeMethod));
    return JNI_VERSION_1_4;
}

