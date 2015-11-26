#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <elf.h>
#include <fcntl.h>

#define SYMTAB  0x01
#define HASH    0X02
#define STRTAB  0x04
#define STRSZ   0X08

static unsigned elfhash(const char *_name)
{
    const unsigned char *name = (const unsigned char *) _name;
    unsigned h = 0, g;
    while(*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

int main(int argc ,char** argv)
{
    Elf32_Ehdr ehdr;
    Elf32_Phdr phdr;
    Elf32_Word dyn_size,dyn_strsz;
    Elf32_Off  dyn_off;
    Elf32_Dyn   dyn;
    Elf32_Addr  dyn_sym,dyn_str,dyn_hash;
    unsigned   func_hash,nbucket,nchain,func_index;
    char * ptr_dynstr = NULL;
    char * ptr_func_content = NULL;
    Elf32_Sym   func_sym;

    int   flag = 0;
    int i = 0;

    char func_name[] = "getString";
    if(argc < 2)
    {
        printf("input the so file\n");
        return 0;
    }
    int fd = open(argv[1],O_RDWR);
    if(fd < 0)
    {

        printf("open file failed\n");
        goto _error;
    }
    lseek(fd,0,SEEK_SET);
    if(read(fd,&ehdr,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        printf("read elf header failed\n");
        goto _error;

    }

    lseek(fd,ehdr.e_phoff,SEEK_SET);
    for(i=0; i<ehdr.e_phnum;i++)
    {
        memset(&phdr,0,sizeof(phdr));
        if(read(fd,&phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr))
        {
            printf("read segment failed\n");
            goto _error;
        }
        if(phdr.p_type == PT_DYNAMIC)
        {
            dyn_off = phdr.p_offset;
            dyn_size = phdr.p_filesz;
            printf("find .dynamic section\n");
            break;
        }
    }
    lseek(fd,dyn_off,SEEK_SET);
    //for(i = 0;i<dyn_size/sizeof(Elf32_Dyn);i++)


    do{
        if(read(fd,&dyn,sizeof(Elf32_Dyn)) != sizeof(Elf32_Dyn))
        {
            printf("read .dynamic failed\n");
            goto _error;
        }
        if(dyn.d_tag == DT_SYMTAB)
        {
            flag |= SYMTAB;
            dyn_sym = dyn.d_un.d_ptr;
        }
        if(dyn.d_tag == DT_STRTAB)
        {
            flag |= STRTAB;
            dyn_str = dyn.d_un.d_ptr;
        }
        if(dyn.d_tag == DT_STRSZ)
        {
            flag |= STRSZ;
            dyn_strsz = dyn.d_un.d_val;
        }
        if(dyn.d_tag == DT_HASH)
        {
            flag |= HASH;
            dyn_hash = dyn.d_un.d_ptr;
        }
    } while(dyn.d_tag != DT_NULL);

    if((flag & 0x0f) != 0x0f)
    {
        printf("find the needed dynamic section failed\n");
        goto _error;
    }

    ptr_dynstr = (char*)malloc(dyn_strsz);
    if(ptr_dynstr == NULL)
    {
        printf("malloc .dynstr failed\n");
        goto _error;
    }
    lseek(fd,dyn_str,SEEK_SET);
    if(read(fd,ptr_dynstr,dyn_strsz) != dyn_strsz)
    {
        printf("read .dynstr failed\n");
        goto _error;
    }

    func_hash = elfhash(func_name);
    lseek(fd,dyn_hash,SEEK_SET);
    if(read(fd,&nbucket,4) != 4)
    {
        printf("read hash nbucket failed\n");
        goto _error;
    }
    if(read(fd,&nchain,4) != 4)
    {
        printf("read hash nchain failed\n");
        goto _error;
    }
    func_hash = func_hash%nbucket;

    lseek(fd,func_hash*4,SEEK_CUR);
    if(read(fd,&func_index,4) != 4)//索引是符号表或者chain
    {
        printf("read funck index failed\n");
        goto _error;
    }
    lseek(fd,dyn_sym+func_index*sizeof(Elf32_Sym),SEEK_SET);
    if(read(fd,&func_sym,sizeof(Elf32_Sym)) != sizeof(Elf32_Sym))
    {
        printf("read func sym entry failed\n'");
    }
    if(strcmp(ptr_dynstr+func_sym.st_name,func_name) != 0)
    {
        while(1)  //纯C语言是没有true的
        {
            lseek(fd,dyn_hash+4*(2+nbucket+func_index),SEEK_SET);
            if(read(fd,&func_index,4) != 4)
            {
                printf("read func index failed\n");
                goto _error;
            }

            lseek(fd,dyn_sym + func_index*sizeof(Elf32_Sym),SEEK_SET);
            memset(&func_sym,0,sizeof(Elf32_Sym));
            if(read(fd,&func_sym,sizeof(Elf32_Sym)) != sizeof(Elf32_Sym))
            {
                goto _error;
            }
            if(strcmp(func_name,dyn_str+func_sym.st_name) == 0)
            {
                break;
            }

        }
    }
    printf("find target func addr: %x,sizeo:%x\n",func_sym.st_value,func_sym.st_size);
    ptr_func_content = (char*)malloc(func_sym.st_size);
    if(ptr_func_content == NULL)
    {
        printf("alloc for func failed\n");
        goto _error;
    }

    lseek(fd,func_sym.st_value,SEEK_SET);

    if(read(fd,ptr_func_content,func_sym.st_size) != func_sym.st_size)
    {
        printf("read func content failed\n");
        goto _error;
    }


    for(i=0;i<func_sym.st_size;i++)
    {
        ptr_func_content[i] = ~ptr_func_content[i];
    }


    lseek(fd,func_sym.st_value,SEEK_SET);
    if(write(fd,ptr_func_content,func_sym.st_size) != func_sym.st_size)
    {
        printf("write to func failed\n");
        goto _error;
    }

    printf("Complete \n");

_error:
    if(ptr_dynstr != NULL)
    {
        free(ptr_dynstr);
    }
    if(ptr_func_content != NULL)
    {
        free(ptr_func_content);
    }

    return 0;
}
