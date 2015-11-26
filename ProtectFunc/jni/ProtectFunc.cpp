#include <jni.h>
#include <stdio.h>
#include <android/log.h>
#include <sys/types.h>
#include <elf.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/errno.h>

#define LOG_TAG "CCDebug"


#define LOGI(msg)	\
	__android_log_print(ANDROID_LOG_INFO,LOG_TAG,msg)
#define LOGW(msg) 	\
	__android_log_print(ANDROID_LOG_WARN,LOG_TAG,msg)
#define LOGE(msg)	\
	__android_log_print(ANDROID_LOG_ERROR,LOG_TAG,msg)
#define LOGD(msg)	\
	__android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,msg)


#define SYMTAB  0x01
#define HASH    0X02
#define STRTAB  0x04
#define STRSZ   0X08


#ifdef __cplusplus
extern "C"{
#endif

//JNIEXPORT void JNICALL Java_com_example_protectfunc_MainActivity_initSo(JNIEnv* env,jobject clazz);
JNIEXPORT jstring getString(JNIEnv* env,jobject clzz);
//JNIEXPORT void JNICALL Java_com_example_protectfunc_MainActivity_initgetString();//__attribute__ ((constructor)); //.initArray
void init_getString() __attribute__((constructor));

#ifdef __cplusplus
}
#endif

JNINativeMethod g_method[] = {
	{
		"getString",
		"()Ljava/lang/String;",
		(void*)getString
	}
};


void Java_com_example_protectfunc_MainActivity_initSo(JNIEnv* env,jobject clazz)
{
	jclass clz_mainActivity = env->FindClass("com/example/protectfunc/MainActivity");
	if(NULL == clz_mainActivity)
	{
		LOGD("find class failed");
		return;
	}
	if(env->RegisterNatives(clz_mainActivity,g_method,sizeof(g_method)/sizeof(JNINativeMethod)) <0)
	{
		LOGD("register failed");
		return;
	}

}




jint JNI_OnLoad(JavaVM* vm,void* reserved)
{
	JNIEnv *env = NULL;

	if(vm->GetEnv((void**)&env,JNI_VERSION_1_6) != JNI_OK)
	{
		LOGD("get Env failed");
		return JNI_ERR;
	}

	jclass clz_mainActivity = env->FindClass("com/example/protectfunc/MainActivity");
	if(NULL == clz_mainActivity)
	{
		LOGD("find class failed");
		return JNI_ERR;
	}
	if(env->RegisterNatives(clz_mainActivity,g_method,sizeof(g_method)/sizeof(JNINativeMethod)) <0)
	{
		LOGD("register failed");
		return JNI_ERR;
	}

	LOGD("register success!");
	return JNI_VERSION_1_6;
}



unsigned long get_cur_lib_addr()
{
	FILE *fp = NULL;
	pid_t pid = -1;
	char sz_maps_path[24] = {0};
	char sz_lib_name[] = "libProtectFunc.so";
	char sz_buf[512] = {0};
	char *ptr_tmp = NULL;
	unsigned long addr = 0;
	pid = getpid();
	sprintf(sz_maps_path,"/proc/%d/maps",pid);
	fp = fopen(sz_maps_path,"r");
	if(NULL == fp)
	{
		return 0;
	}
	while(fgets(sz_buf,sizeof(sz_buf),fp))
	{
		if(strstr(sz_buf,sz_lib_name))
		{
			ptr_tmp = strtok(sz_buf,"-");
			addr = strtoul(ptr_tmp,NULL,0x10);
			break;
		}
	}
	fclose(fp);
	return addr;
}


// \bonic\linker\Linker.cpp
static unsigned elfhash(const char* _name) {
    const unsigned char* name = (const unsigned char*) _name;
    unsigned h = 0, g;

    while(*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

void init_getString()
{
	unsigned long lib_addr = 0;
	Elf32_Ehdr*  ptr_ehdr = NULL;
	Elf32_Phdr*  ptr_phdr = NULL;
	Elf32_Dyn*   ptr_dyn = NULL;;
	Elf32_Sym*   ptr_dynsym = NULL;
	Elf32_Sym* 	 sym = NULL;
	const char* ptr_hashtab = NULL;
	const char* ptr_dynstr = NULL;
	int flag = 0;
	unsigned long strtab_size = 0;
	unsigned func_hash = 0;
	const char func_name[] = "getString";
	unsigned long func_addr = 0;
	unsigned   func_size =0;
	unsigned long func = (unsigned long)getString;
	size_t nbucket;
	size_t nchain;
	unsigned* bucket;
	unsigned* chain;
	const unsigned page_size = 0x1000;

	int i =0;
	lib_addr = get_cur_lib_addr();
	if(0 == lib_addr)
	{
		LOGD("get_cur_lib_addr failed");
		return;
	}
	ptr_ehdr = (Elf32_Ehdr*)lib_addr;
	ptr_phdr = (Elf32_Phdr*)(lib_addr+ptr_ehdr->e_phoff);
	for(i=0;i<ptr_ehdr->e_phnum;i++,ptr_phdr++)
	{
		if(PT_DYNAMIC == ptr_phdr->p_type)
		{
			ptr_dyn = (Elf32_Dyn*)(lib_addr+ptr_phdr->p_vaddr);
			break;
		}

	}
	if(NULL == ptr_dyn)
	{
		LOGD("find .dynamic failed");
		return ;
	}
	// .dynsym  .dynstr .hash .
	for (Elf32_Dyn* d = ptr_dyn; d->d_tag != DT_NULL; ++d)
	{
		switch(d->d_tag)
		{
		case DT_SYMTAB:
			ptr_dynsym = (Elf32_Sym*)(lib_addr+d->d_un.d_ptr);
			flag |= SYMTAB;
			break;
		case DT_STRTAB:
		    ptr_dynstr = (const char*)(lib_addr + d->d_un.d_ptr);
		    flag |= STRTAB;
		    break;
		case DT_STRSZ:
			strtab_size = d->d_un.d_val;
			flag |= STRSZ;
			break;
		case DT_HASH:
			ptr_hashtab = (const char*)(lib_addr + d->d_un.d_ptr);
			flag |= HASH;
			break;
		}
	}

	if(flag & 0xf == 0xf)
	{
		LOGD("all segement get");
	}

	nbucket = *(unsigned*)ptr_hashtab;
	nchain = *(unsigned*)(ptr_hashtab+4);
	bucket = (unsigned*)(ptr_hashtab+8);
	chain = bucket + nbucket;

	func_hash = elfhash(func_name);

	for (unsigned n = bucket[func_hash % nbucket]; n != 0; n = chain[n])
	{
	    sym = ptr_dynsym + n;
		if (strcmp(ptr_dynstr + sym->st_name, func_name)) continue;

		switch(ELF32_ST_BIND(sym->st_info))
		{
		case STB_GLOBAL:
	    case STB_WEAK:
	    	if (sym->st_shndx == SHN_UNDEF)
	    	{
	              continue;
	        }
		}

		func_addr = lib_addr+sym->st_value;
		func_size = sym->st_size;
		break;
	}

	unsigned page_off = func_addr % page_size;

	if(mprotect((const void*)(func_addr-page_off),func_size+page_off,PROT_WRITE|PROT_READ|PROT_EXEC)!= JNI_OK)
	{
		int n = errno;
		char *msg = strerror(errno);
		LOGD("change page protect failed");
		LOGD(msg);
		return;

	}

	for(int i =0;i<func_size;i++)
	{
		((char*)func_addr)[i] = ~((char*)func_addr)[i];
	}


	if(mprotect((const void*)(func_addr-page_off),func_size+page_off,PROT_READ|PROT_EXEC) != JNI_OK)
	{
		int n = errno;
		char *msg = strerror(errno);
		LOGD("resume page protect failed");
		LOGD(msg);
		return;
	}

}





jstring getString(JNIEnv* env,jobject clazz)
{
	LOGD("get string invoke");
	return env->NewStringUTF("hello getstring");
}
