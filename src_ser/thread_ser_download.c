#include"factory.h"

/*服务器端响应客户端下载文件*/

void* thread_ser_download(void* p)
{
	pload_t pDown=(pload_t)p;
	int fd,ret;
	struct stat fmsg;
	struct trans bag;
	char path[PATH_MAX]={0};

	/*在数据库中查询该用户在当前目录是否有此文件以及判断该文件是否为目录*/

	ret=mysql_check_download(pDown->pcur,pDown->file_name);
	if(1==ret)  /*如果文件存在且为目录*/
	{
		bag.len=27;
		send(pDown->pcur->new_sfd,(char*)&bag.len,4,0);
		send(pDown->pcur->new_sfd,"can not download directory",27,0);
		pthread_exit((void*)-1);
	}
	else if(-1==ret) /*文件不存在*/
	{
		bag.len=13;
		send(pDown->pcur->new_sfd,(char*)&bag.len,4,0);
		send(pDown->pcur->new_sfd,"No such file",13,0);
		pthread_exit((void*)-1);
	}
	else
    {
        /*防止客户端阻塞*/

		bag.len=6;
		send(pDown->pcur->new_sfd,(char*)&bag.len,4,0);
		send(pDown->pcur->new_sfd,"start",6,0);
		
		/*打开文件*/
        
        sprintf(path,"/home/lnn/ftp/FilePool/%s",pDown->file_name);
		fd=open(path,O_RDONLY);
		if(-1==fd)
		{
			pthread_exit((void*)-1);
		}

		/*发送文件大小*/

		bzero(&fmsg,sizeof(struct stat));
		fstat(fd,&fmsg);
		bzero(&bag,sizeof(struct trans));
		bag.len=sizeof(off_t);
		memcpy(bag.buf,&fmsg.st_size,sizeof(off_t));
		send(pDown->pcur->new_sfd,(char*)&bag,4+bag.len,0);

		/*发送文件*/

		sendfile(pDown->pcur->new_sfd,fd,NULL,fmsg.st_size);
		close(fd);
		pthread_exit((void*)0);
	}
}
