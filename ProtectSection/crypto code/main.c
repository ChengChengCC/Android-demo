#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <fcntl.h>



int main(int argc,char ** argv)
{
    char section_name[] = ".mytext";
    Elf32_Ehdr ehdr;
    Elf32_Shdr shdr;
    char * ptr_shstrtab = NULL;
    Elf32_Off    target_section_offset = 0;
    Elf32_Word target_section_size = 0;
    char * ptr_section_content = NULL;
    int page_size = 4096;
    int fd;
    if(argc < 2)
    {
        puts("input so file\n");
        return -1;
    }
    fd = open(argv[1],O_RDWR);
    if(fd < 0)
    {
        goto _error;
    }
    if(read(fd,&ehdr,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        puts("read elf header failed\n");
        goto _error;
    }

    lseek(fd,ehdr.e_shoff+sizeof(Elf32_Shdr)*ehdr.e_shstrndx,SEEK_SET);
    if(read(fd,&shdr,sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr))
    {
        puts("read elf .shstrtab header failed\n");
        goto _error;
    }

    ptr_shstrtab = (char*)malloc(shdr.sh_size);
    if(NULL == ptr_shstrtab)
    {
        puts("apply mem failed\n");
        goto _error;
    }

    //read shstrtab
    lseek(fd,shdr.sh_offset,SEEK_SET);
    if(read (fd,ptr_shstrtab,shdr.sh_size) != shdr.sh_size)
    {
        goto _error;
    }

    lseek(fd,ehdr.e_shoff,SEEK_SET);
    int i = 0;
    for(i = 0;i < ehdr.e_shnum;i++)
    {
        if(read(fd,&shdr,sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr))
        {
            puts("find target section faile\nd");
            goto _error;
        }
        if( !strcmp(ptr_shstrtab + shdr.sh_name , section_name) )
        {
            target_section_offset  = shdr.sh_offset;
            target_section_size = shdr.sh_size;
            break;
        }
    }

    lseek(fd,target_section_offset,SEEK_SET);

    ptr_section_content = (char*)malloc(target_section_size);
    if(NULL ==ptr_section_content)
    {
        goto _error;
    }

    if(read(fd,ptr_section_content,target_section_size) != target_section_size)
    {
        goto _error;
    }

    int num_page = target_section_size/page_size + 1;
    ehdr.e_entry =  target_section_size;
    ehdr.e_shoff =  target_section_offset;


    for(i = 0; i<target_section_size;i++)
    {
        ptr_section_content[i] =~ ptr_section_content[i];
    }

    lseek(fd,0,SEEK_SET);
    if(write(fd,&ehdr,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
    {
        goto _error;
    }

    lseek(fd , target_section_offset , SEEK_SET);
    if(write(fd , ptr_section_content,target_section_size) != target_section_size)
    {
        goto _error;
    }


    puts("completed\n");


_error:
    if(NULL != ptr_section_content)
    {
        free(ptr_section_content);
    }
    if(NULL != ptr_shstrtab)
    {
        free(ptr_shstrtab);
    }
    if(NULL != fd)
    {
        close(fd);
    }


    return 0;
}
