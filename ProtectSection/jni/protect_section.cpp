#include <jni.h>
#include <android/log.h>
#include <stddef.h>
#include <elf.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

/*
#if defined (__arm__)
	#if defined(__ARM_ARCH_7A__)
		#if defined(__ARM_NEON__)
			#define ABI "armeabi-v7a/NEON"
		#else
			#define ABI "armeabi-v7a"
		#endif
	#else
		#define ABI "armabi"
	#endif
#elif defined (__x86__)
	#define ABI "x86"
#elif defined (__mips__)
	#define ABI "mips"
#else
	#define ABI "unkown"
#endif

*/

#define LOG_TAG "CCDebug"
/*
#define LOGD(msg) \
	__android_log_write(ANDROID_LOG_ERROR , LOG_TAG , msg);
	*/


#define LOGI(msg)	\
	__android_log_print(ANDROID_LOG_INFO,LOG_TAG,msg)
#define LOGW(msg) 	\
	__android_log_print(ANDROID_LOG_WARN,LOG_TAG,msg)
#define LOGE(msg)	\
	__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,msg)
#define LOGD(msg)	\
	__android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,msg)

#ifdef __cplusplus
extern "C" {
#endif
//JNIEXPORT jstring JNICALL Java_com_example_protect_MainActivity_getString(JNIEnv * env,jobject clazz);
//JNIEXPORT void JNICALL Java_com_example_protect_MainActivity_initSo(JNIEnv * env,jobject clazz);

JNIEXPORT jstring JNICALL getString(JNIEnv*,jobject)__attribute__((section(".mytext")));

void init_getString()__attribute__((constructor));  //initarray
unsigned long get_cur_lib_addr();

#ifdef __cplusplus
}
#endif



static JNINativeMethod g_methods[] = {
		{
			"getString",
			"()Ljava/lang/String;",
			(void*)getString
		}
};


unsigned long get_cur_lib_addr()
{
	unsigned long addr = 0;
	char section_name[] = "libprotect_section.so";
 	pid_t pid = 0;     //<types.h>
 	char buf[1024] = {0};
 	FILE* fp = NULL;
 	char* tmp = NULL;
	pid = getpid();   //<unistd.h>
	sprintf(buf,"/proc/%d/maps",pid);    //<stdio.h>
	fp = fopen(buf,"r");
	if(NULL == fp)
	{
		return 0;
	}
	while(fgets(buf,sizeof(buf),fp))
	{
		if(strstr(buf,section_name))
		{
			tmp = strtok(buf,"-");
			addr = strtoul(tmp,NULL,0x10);   //<stdlib.h>
			break;
		}
	}

	fclose(fp);
	return addr;
}


void init_getString()
{
	//LOGD("Hello init_getString");

	unsigned long lib_addr;
	Elf32_Ehdr* ptr_ehdr = NULL;
	Elf32_Shdr* ptr_shdr = NULL;
	unsigned long mytext_addr;
	unsigned long mytext_size = 0;
	unsigned long page_size = 0x1000;
	lib_addr = get_cur_lib_addr();
	if(NULL == lib_addr)
	{
		return ;
	}

	ptr_ehdr = (Elf32_Ehdr*)lib_addr;
	mytext_size = ptr_ehdr->e_entry; //size
	mytext_addr = ptr_ehdr->e_shoff + lib_addr;  //offset
	unsigned long offset = mytext_addr % page_size;
	LOGD("invoke mprotect first");
	if(mprotect((const void*)(mytext_addr-offset) , mytext_size,PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
	{
		LOGD("change the mem failed");
		return ;
	}

	for(int i=0;i<mytext_size;i++)
	{
		((char*)mytext_addr)[i] =~ ((char*)mytext_addr)[i];
	}
	LOGD("invoke mprotect to resume the page");
	if(mprotect((const void*)(mytext_addr-offset),mytext_size,PROT_READ | PROT_EXEC)!=0)
	{
		LOGD("resume mem failed");
		return ;
	}

}



JNIEXPORT jstring JNICALL getString(JNIEnv* env,jobject clazz)
{

	LOGD("getString invoke");
	return env->NewStringUTF("Hello_CC");

}


/*
JNIEXPORT jstring JNICALL Java_com_example_protect_MainActivity_getString(JNIEnv * env,jobject clazz)
{
	LOGD("getString invoke");
	return env->NewStringUTF("Hello_CC");
}
*/


//JNIEXPORT void JNICALL Java_com_example_protect_MainActivity_initSo(JNIEnv * env,jobject clazz)
//{
	/*
	jclass clz_main_activity = env->FindClass("com/example/protect/MainActivity");
	if(clz_main_activity == NULL)
	{
		LOGD("find class failed");
		return ;
	}


	if(env->RegisterNatives(clz_main_activity,g_methods,sizeof(g_methods)/sizeof(g_methods[0]))<0)
	{
		LOGD("register failed");
	//	abort();
		return;
	}
*/

//	return;
//}




jint JNI_OnLoad(JavaVM *vm,void * reserved)
{
	jint result = -1;
	JNIEnv *env = NULL;
	if(vm->GetEnv((void**)&env,JNI_VERSION_1_4) == JNI_FALSE)
	{
		result = -1;
	}
	//com.example.protectsection
	jclass clazz = env->FindClass("com/example/protectsection/MainActivity");
	if(clazz == NULL)
	{
		LOGD("find class failed");
		return result;
	}

	if(env->RegisterNatives(clazz,g_methods,sizeof(g_methods)/sizeof(g_methods[0]))<0)
	{
		LOGD("register natives failed");
		return result;
	}

	result  = JNI_VERSION_1_4;
	return result;
}


