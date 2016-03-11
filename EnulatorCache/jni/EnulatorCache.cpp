#include <jni.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

typedef int (*PFNcall)(void);
#define PROT PROT_EXEC|PROT_WRITE|PROT_READ
#define FLAGS MAP_ANONYMOUS| MAP_FIXED |MAP_SHARED


#define PAGE_START(addr) (~(getpagesize() - 1) & (addr))

/*__asm __volatile (
1 "stmfd sp!,{r4-r8,lr}\n"
2 "mov r6,#0\n"  用来统计循环次数，debug用的
3 "mov r7,#0\n"  为r7赋初值

4 "mov r8,pc\n"  4、7行用来获得覆盖$address“新指令”的地址 pc--> add r7,#1
5 "mov r4,#0\n"  为r4赋初值
6 "add r7,#1\n"  用来覆盖$address的“新指令”
7 "ldr r5,[r8]\n"   r5 里面存放的就是add r7,#1


8 "code:\n"
9 "add r4,#1\n"  这就是$address，是对r4加1

10 "mov r8,pc\n"  10,11,12行的作用就是把第6行的指令写到第9行
11 "sub r8,#12\n"   r8--> add r4,#1
12 "str r5,[r8]\n"    将add r7,#1写入到add r4,#1中


13 "add r6,#1\n"   r6用来计数


14 "cmp r4,#10\n"  控制循环次数
15 "bge out\n"


16 "cmp r7,#10\n"   控制循环次数
17 "bge out\n"
18 "b code\n"      10次内的循环调回去

19 "out:\n"
20 "mov r0,r4\n"    把r4的值作为返回值
21 "ldmfd sp!,{r4-r8,pc}\n"
);
 * */


char code[]=
"\xF0\x41\x2D\xE9\x00\x60\xA0\xE3\x00\x70\xA0\xE3\x0F\x80\xA0\xE1"
"\x00\x40\xA0\xE3\x01\x70\x87\xE2\x00\x50\x98\xE5\x01\x40\x84\xE2"
"\x0F\x80\xA0\xE1\x0C\x80\x48\xE2\x00\x50\x88\xE5\x01\x60\x86\xE2"
"\x0A\x00\x54\xE3\x02\x00\x00\xAA\x0A\x00\x57\xE3\x00\x00\x00\xAA"
"\xF5\xFF\xFF\xEA\x04\x00\xA0\xE1\xF0\x81\xBD\xE8";


extern "C" JNIEXPORT jint Java_com_example_enulatorcache_MainActivity_start(JNIEnv *env, jobject thiz){
	int a;
	PFNcall call = 0;
	//用mmap会有bug，在真机上只能执行一次，第二次崩溃
	//void *exec = mmap((void*)0x10000000,(size_t)4096 ,PROT ,FLAGS,-1,(off_t)0);
	void *exec = malloc(0x1000);
	memcpy(exec ,code,sizeof(code)+1);
	void *page_start_addr = (void *)PAGE_START((uint32_t)exec);
	mprotect(page_start_addr, getpagesize(), PROT);

	call=(PFNcall)exec;
	call();

	__asm __volatile (
	"mov %0,r0\n"
	:"=r"(a)
	:
	:
	);

	free(exec);
	//munmap((void*)0x10000000,(size_t)4096);
	if(a==0xA){ //真机
		return 0;
	}else{
		return 1;
	}

	return a;
}
