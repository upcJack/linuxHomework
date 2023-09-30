#include<pthread.h>
#include<stdio.h>
#include<pthread.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<signal.h>
#include<sys/shm.h>
#include<time.h>
#include<pthread.h>
//#include"apue.h"
#define NRTHREAD 1  //只有一个读线程
#define NWTHREAD 3   //有三个写线程
#define BLOCKSIZE 4096
#define MYMODE S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH //允许文件的所有者/组阅读/写它。

static unsigned long int blknum=0; //写程序的偏移量，已经写了的块数
static int outfd,infd;
static int pfd[2]; //管程的数量

pthread_mutex_t wmutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rmutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pmutex=PTHREAD_MUTEX_INITIALIZER;
void *thr_w(void *arg)
{
    unsigned long int temp;
    int n=1;
    char tempbuf[BLOCKSIZE];
    while (1)
    {
        pthread_mutex_lock(&wmutex);
        n=read(pfd[0],tempbuf,BLOCKSIZE); 
        //从管道中读取最多BLOCKSIZE个字节的数据，并将其存储在tempbuf缓冲区中。
        //如果管道中没有数据可读，则read函数会阻塞，直到有数据可读为止。
        //读取成功后，read函数将返回实际读取的字节数，如果出现错误则返回-1
        if (n==0) //到达文件EOF，这可能是由于写入管道的进程已经关闭了写入端
        {
            pthread_mutex_unlock(&wmutex);
            break;
        }
        temp=blknum++; //偏移量是blknum
        pthread_mutex_unlock(&wmutex);
        if (pwrite(outfd,tempbuf,n,temp*BLOCKSIZE)!=n)
        {
            // 使用原子写入函数pwrite来将数据写入到指定的文件描述符中。
            // 在这段代码中，outfd是要写入的文件描述符，
            // tempbuf是包含要写入数据的缓冲区，n是要写入的字节数，
            // temp*BLOCKSIZE是写入的偏移量。
            printf("thread write file error!\n");
            pthread_exit((void *)0);//当前线程的执行结束时不携带任何退出状态
        }
        if (n<0) //读取失败
        {
            printf("thread read pipe error!\n");
            pthread_exit((void *)0);//当前线程的执行结束时不携带任何退出状态
        }
    }
    pthread_exit((void *)0);//当前线程的执行结束时不携带任何退出状态
}
void *thr_r(void *arg)
{
    unsigned long int temp;
    int n=1;
    int i_rnum;
    char tempbuf[BLOCKSIZE];
    while (1)
    {
        pthread_mutex_lock(&rmutex);//加锁确保只有一个线程进行以下读程序
        n=read(infd,tempbuf,BLOCKSIZE);
        //是从文件描述符infd中读取数据，并将其存储到tempbuf缓冲区中，
        //同时将成功读取的字节数存储在变量n中
        if (n==0)
        {
            pthread_mutex_unlock(&rmutex);
            break;
        }
        pthread_mutex_lock(&pmutex); //确保只有一个线程可以访问接下来的代码块
        pthread_mutex_unlock(&rmutex);//在该线程读完并且写入到管程中后才能释放这个读锁
        i_rnum=write(pfd[1],tempbuf,n); //写入到pfd[1]中
        //这段代码的含义是将tempbuf缓冲区中的数据写入到管道的写端文件描述符pfd[1]所代表的管道中。
        //写入的数据字节数由变量n指定。如果写入成功，则返回值等于参数n
        pthread_mutex_unlock(&pmutex);
        if (i_rnum!=n)
        {
            printf("write pipe error!\n");
            pthread_exit((void *)0);
        }
        if (n<0)
        {
            printf("read infile.txt error!\n");
            pthread_exit((void *)0);
        }
    }
    pthread_exit((void *)0);
}
int main(int argc,char *argv[])
{
    clock_t start_time,finish_time;
    double total_time;
    start_time=clock(); //记录文件复制开始时间

    int n,err;
    pthread_t wtid[NWTHREAD]; //创建NWTHREAD个进程
    pthread_t rtid[NRTHREAD]; //创建NRTHREAD个进程

    if (argc != 3) //使用规则
    {
        printf("usage:./producer-consumer src dst\n");
        return -1;
    }

    outfd=open(argv[2],O_WRONLY | O_CREAT | O_TRUNC, MYMODE); //首先打开/创建output.txt
    if (outfd<0)
    {
        printf(" create outfile.txt failed!\n");
        return -1;
    }

    infd=open(argv[1],O_RDONLY); //只读形式打开input.txt
    if (infd<0)
    {
        printf("open infile.txt error!\n");
        return -1;
    }
    if (pipe(pfd)<0) //创建管程,其中pfd[0]用于读取管道，pfd[1]用于写入管道
    {
        printf("create pipe error!\n");
        return -1;
    }

    for (n=0;n<NRTHREAD;n++) //创建NRTHREAD=1个线程读取文件
    {
        err=pthread_create(&rtid[n],NULL,thr_r,NULL);
        if (err!=0)
        {
            printf("create the %dth thread error!\n",n);
            return -1;
        }
    }
    for (n=0;n<NWTHREAD;n++) //创建NWTHREAD=3个线程读取文件
    {
        err=pthread_create(&wtid[n],NULL,thr_w,NULL);
        if (err!=0)
        {
            printf("create the %dth thread error!\n",n);
            return -1;
        }
    }
    for (n=0;n<NRTHREAD;n++)
    {
        err=pthread_join(rtid[n],NULL);//阻塞等待id为thread的线程退出
        if (err!=0)
        {
            printf("join the %dth thread error!\n",n);
            return -1;
        }
    }
    close(pfd[1]);//关闭管程1 
    for (n=0;n<NWTHREAD;n++)
    {
        err=pthread_join(wtid[n],NULL); //阻塞等待id为thread的线程退出
        if (err!=0)
        {
            printf("join the %dth thread error!\n",n);
            return -1;
        }
    }
    close(infd);
    close(outfd);
    close(pfd[0]);//关闭管程0

    finish_time=clock();
    total_time=((double)(finish_time-start_time))/CLOCKS_PER_SEC;
    printf( "%f seconds/n\n", total_time);
    return 0;
}
