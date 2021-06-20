
#include <cstring>
#include <string>

#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <jni.h>
#include <unistd.h>

#include <riru.h>

#include "log.h"

static std::string app_data_dir;

const char *ARCHITECTURE =
	#if defined(__aarch64__)
		"arm64-v8a"
	#elif defined(__ARM_ARCH_7A__)
		"armeabi-v7a"
	#elif defined(__i386__)
		"x86"
	#elif defined(__x86_64__)
		"x86_64"
	#else
		#error "Unsupported architecture!"
	#endif
;

static int shouldSkipUid(int uid) { return false; }

static void forkAndSpecializePre(
		JNIEnv *env, jclass clazz, jint *uid, jint *gid, jintArray *gids, jint *runtimeFlags,
		jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
		jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
		jstring *instructionSet, jstring *appDataDir, jboolean *isTopApp, jobjectArray *pkgDataInfoList,
		jobjectArray *whitelistedDataInfoList, jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {
	if (!*appDataDir) return;
	const char *app_data_dir = env->GetStringUTFChars(*appDataDir, nullptr);
	::app_data_dir = app_data_dir;
	env->ReleaseStringUTFChars(*appDataDir, app_data_dir);
}

static void forkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
	if (res == 0) {
		// In app process
		// riru_set_unload_allowed(true);
		std::string path = app_data_dir + "/files/reedle";
		if (access(path.data(), F_OK) == -1) return;
		path += '/'; path += ARCHITECTURE;
		auto *dir = opendir(path.data());
		if (!dir) {
			LOGW("Reedle directory found but the library directory for the architecture (%s) is not found", ARCHITECTURE);
			LOGW("A.K.A. %s", path.data());
			return;
		}
		LOGI("Loading reedles in %s", path.data());
		path += '/'; dirent *ent;
		while ((ent = readdir(dir))) {
			auto len = strlen(ent->d_name);
			if (len >= 3 && !strcmp(ent->d_name + len - 3, ".so")) {
				auto library_path = path + ent->d_name;
				LOGI("Loading reedle from %s", library_path.data());
				void *handle = dlopen(library_path.data(), RTLD_LAZY);
				if (!handle) {
					LOGW("Failed to load reedle: %s. Maybe permission denied?", dlerror());
					return;
				}
				auto entry = (void (*)(JNIEnv*, const char*))dlsym(handle, "reedle_main");
				if (!entry) {
					LOGW("Entry not found in reedle");
					return;
				}
				entry(env, app_data_dir.data());
			}
		}
		closedir(dir);
	} else {
		// In zygote process
	}
}

static void specializeAppProcessPre(
		JNIEnv *env, jclass clazz, jint *uid, jint *gid, jintArray *gids, jint *runtimeFlags,
		jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
		jboolean *startChildZygote, jstring *instructionSet, jstring *appDataDir,
		jboolean *isTopApp, jobjectArray *pkgDataInfoList, jobjectArray *whitelistedDataInfoList,
		jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {
}

static void specializeAppProcessPost(JNIEnv *env, jclass clazz) {
	riru_set_unload_allowed(true);
}

static void forkSystemServerPre(
		JNIEnv *env, jclass clazz, uid_t *uid, gid_t *gid, jintArray *gids, jint *runtimeFlags,
		jobjectArray *rlimits, jlong *permittedCapabilities, jlong *effectiveCapabilities) {
}

static void forkSystemServerPost(JNIEnv *env, jclass clazz, jint res) {
	if (res == 0) {
		// In system server process
	} else {
		// In zygote process
	}
}

static void onModuleLoaded() {
}

extern "C" {

int riru_api_version;
const char *riru_magisk_module_path = nullptr;
int *riru_allow_unload = nullptr;

static auto module = RiruVersionedModuleInfo{
		.moduleApiVersion = RIRU_MODULE_API_VERSION,
		.moduleInfo = RiruModuleInfo{
				.supportHide = true,
				.version = RIRU_MODULE_VERSION,
				.versionName = RIRU_MODULE_VERSION_NAME,
				.onModuleLoaded = onModuleLoaded,
				.forkAndSpecializePre = forkAndSpecializePre,
				.forkAndSpecializePost = forkAndSpecializePost,
				.forkSystemServerPre = forkSystemServerPre,
				.forkSystemServerPost = forkSystemServerPost,
				.specializeAppProcessPre = specializeAppProcessPre,
				.specializeAppProcessPost = specializeAppProcessPost
		}
};

#ifndef RIRU_MODULE_LEGACY_INIT
RiruVersionedModuleInfo *init(Riru *riru) {
	auto core_max_api_version = riru->riruApiVersion;
	riru_api_version = core_max_api_version <= RIRU_MODULE_API_VERSION ? core_max_api_version : RIRU_MODULE_API_VERSION;
	module.moduleApiVersion = riru_api_version;

	riru_magisk_module_path = strdup(riru->magiskModulePath);
	if (riru_api_version >= 25) riru_allow_unload = riru->allowUnload;
	return &module;
}
#else
RiruVersionedModuleInfo *init(Riru *riru) {
	static int step = 0;
	step += 1;

	switch (step) {
		case 1: {
			auto core_max_api_version = riru->riruApiVersion;
			riru_api_version = core_max_api_version <= RIRU_MODULE_API_VERSION ? core_max_api_version : RIRU_MODULE_API_VERSION;
			if (riru_api_version < 25) module.moduleInfo.unused = (void *) shouldSkipUid;
			else riru_allow_unload = riru->allowUnload;
			if (riru_api_version >= 24) {
				module.moduleApiVersion = riru_api_version;
				riru_magisk_module_path = strdup(riru->magiskModulePath);
				return &module;
			} else return (RiruVersionedModuleInfo *) &riru_api_version;
		}
		case 2: return (RiruVersionedModuleInfo *) &module.moduleInfo;
		case 3: default: return nullptr;
	}
}
#endif
}
