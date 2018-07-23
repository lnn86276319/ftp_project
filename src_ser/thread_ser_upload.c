#include"factory.h"

/*服务端接收客户端上传文件*/

void* thread_ser_upload(void* p)
{
	errno=0;
	pload_t pUp=(pload_t)p;
	int fd,len,ret;
	char* mmap_addr;
	off_t file_size,recv_size_all=0,recv_size_every;
	char MD5[33]={0};

	/*接收文件大小，FileSize*/

	recv(pUp->pcur->new_sfd,(char*)&len,4,0);
	recv(pUp->pcur->new_sfd,(char*)&file_size,len,0);

	/*接收文件MD5，MD5*/

	recv(pUp->pcur->new_sfd,(char*)&len,4,0);
	recv(pUp->pcur->new_sfd,MD5,len,0);

	/*数据库匹配,第一步查询是否有重名文件，有就回复并驳回请求，没有则进行第二步，文件池中是否有该文件如果有这个文件即MD5相等，就直接插入一条信息就行，无需接收可直接结束线程，没有就接收*/

	ret=mysql_check_upload(pUp->pcur,pUp->file_name); /*ret等于0说明没有重名文件*/
	if(0==ret)
	{
		/*未重名情况下，无论是否正常接收还是秒传，都需要向数据库插入新信息*/
		mysql_insert_upload(pUp->pcur,pUp->file_name,file_size,MD5);
		bzero(pUp->file_name,PATH_MAX);
		sprintf(pUp->file_name,"%s/%s","/home/lnn/ftp/FilePool",MD5);
		fd=open(pUp->file_name,O_RDWR);
		if(ENOENT==errno)  /*不存在该文件,老实接收*/
		{
			len=6;
			send(pUp->pcur->new_sfd,(char*)&len,4,0);
			send(pUp->pcur->new_sfd,"start",6,0);  /*防止客户端阻塞*/
			fd=open(pUp->file_name,O_RDWR|O_CREAT,0600);
			if(-1==fd)
			{
				send(pUp->pcur->new_sfd,"upload fail",12,0);
				pthread_exit((void*)-1);
			}

			/*更改文件大小并mmap*/

			ftruncate(fd,file_size);
			mmap_addr=mmap(NULL,file_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
			if(MAP_FAILED==mmap_addr)
			{
				send(pUp->pcur->new_sfd,"upload fail",12,0);
				pthread_exit((void*)-1);
			}

			while(recv_size_all<file_size)
			{
				recv_size_every=recv(pUp->pcur->new_sfd,mmap_addr+recv_size_all,file_size-recv_size_all,0);
				if(-1==recv_size_every)
				{
				    send(pUp->pcur->new_sfd,"upload fail",12,0);
					pthread_exit((void*)-1);
				}
				recv_size_all+=recv_size_every;
			}
			munmap(mmap_addr,file_size);
		}
		else
		{
			len=11;
			send(pUp->pcur->new_sfd,(char*)&len,4,0);
			send(pUp->pcur->new_sfd,"file exist",11,0);  /*客户端的recv判断条件*/
		}
		close(fd);
		send(pUp->pcur->new_sfd,"upload success",15,0);
		pthread_exit((void*)0);
	}
	else
	{
		len=16;
		send(pUp->pcur->new_sfd,(char*)&len,4,0);
		send(pUp->pcur->new_sfd,"filename repeat",16,0); /*客户端的recv判断条件*/
		send(pUp->pcur->new_sfd,"upload fail",12,0);
		pthread_exit((void*)-1);
	}
}
