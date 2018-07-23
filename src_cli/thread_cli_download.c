#include"factory.h"

/*客户端下载文件*/

void* thread_cli_download(void* p)
{
	pload_t pDown=(pload_t)p;
	int fd,len;
	time_t start,end;
	char* mmap_addr;
	struct trans bag;
	off_t file_size,recv_size_all=0,recv_size_every;

    /*先要得到服务器端反馈再判断是否能进行下载*/

	bzero(&bag,sizeof(struct trans));
	recv(pDown->sfd,(char*)&bag.len,4,0);
	recv(pDown->sfd,bag.buf,bag.len,0);
	printf("%s\n",bag.buf);
	if(strcmp(bag.buf,"can not download directory")==0||strcmp(bag.buf,"No such file")==0)
	{
		pthread_exit((void*)0);
	}

	/*创建文件*/

	fd=open(pDown->file_name,O_RDWR|O_CREAT|O_EXCL,0600);
	if(-1==fd)
	{
		pthread_exit((void*)-1);
	}

	/*接收文件大小*/

	recv(pDown->sfd,(char*)&len,4,0);
	recv(pDown->sfd,(char*)&file_size,len,0);
	/*更改文件大小并mmap*/

	ftruncate(fd,file_size);
	mmap_addr=mmap(NULL,file_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if(MAP_FAILED==mmap_addr)
	{
		pthread_exit((void*)-1);
	}
	start=time(NULL);
	while(recv_size_all<file_size)
	{
		recv_size_every=recv(pDown->sfd,mmap_addr+recv_size_all,file_size-recv_size_all,0);
		if(-1==recv_size_every)
		{
			pthread_exit((void*)-1);
		}
		recv_size_all+=recv_size_every;
		end=time(NULL);
		if(end-start>1)
		{
			printf("downloading.....%5.2f%s\r",(double)recv_size_all/file_size*100,"%");
			fflush(stdout);
			start=end;
		}
	}
	printf("download success.....100%s\n","%");
    munmap(mmap_addr,file_size);
	close(fd);
	pthread_exit((void*)0);
}
