#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lsplt.hpp"

#define LOG_TAG "LSPlant"

// headers
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
template<typename T, size_t N>
[[gnu::always_inline]] constexpr inline size_t arraysize(T(&)[N]) {
    return N;
}

// Get the process name from /proc/self/cmdline
static void get_process_name(char *buffer, size_t size) {
    FILE *cmdline = fopen("/proc/self/cmdline", "r");
    if (cmdline) {
        size_t read = fread(buffer, 1, size - 1, cmdline);
        fclose(cmdline);
        buffer[read] = '\0';
        // Replace null bytes with underscores
        for (char *p = buffer; *p; ++p) {
            if (*p == '\0' && *(p+1) != '\0') *p = '_';
        }
    } else {
        strncpy(buffer, "unknown", size);
    }
}

static bool hookInitialized = false;
static jint MODIFIER_NATIVE = 0;
static jmethodID member_getModifiers = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_vn_vinhjaxt_LSPHook_NativeHookMethod(JNIEnv* env, jclass _, jobject target, jobject hook, jobject backup) {
    if (!hookInitialized) {
        hookInitialized = true;

        auto classMember = lsplant::JNI_FindClass(env, "java/lang/reflect/Member");
        if (classMember != nullptr) member_getModifiers = lsplant::JNI_GetMethodID(env, classMember, "getModifiers", "()I");
        auto classModifier = lsplant::JNI_FindClass(env, "java/lang/reflect/Modifier");
        if (classModifier != nullptr) {
            auto fieldId = lsplant::JNI_GetStaticFieldID(env, classModifier, "NATIVE", "I");
            if (fieldId != nullptr) MODIFIER_NATIVE = lsplant::JNI_GetStaticIntField(env, classModifier, fieldId);
        }
        if (member_getModifiers == nullptr || MODIFIER_NATIVE == 0) return;

        if (!lsplant::art::ArtMethod::Init(env)) {
            LOGE("failed to init ArtMethod");
            return;
        }
    }

}

static JNINativeMethod global_JNI_Methods[] = {
    {"NativeHookMethod", "(Ljava/lang/reflect/Executable;Ljava/lang/reflect/Executable;Ljava/lang/reflect/Executable;)V", (void*) Java_vn_vinhjaxt_LSPHook_NativeHookMethod},
};

extern "C" void envRegisterNatives(JNIEnv* env, jclass clazz) {
    jint ret = env->RegisterNatives(clazz, global_JNI_Methods, arraysize(global_JNI_Methods));
    if (ret != 0) {
        LOGE("envRegisterNatives: %d", ret);
    }
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}
