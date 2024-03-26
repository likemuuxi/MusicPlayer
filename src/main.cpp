#include "widget.h"
#include <QApplication>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <QDebug>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//创建管道
int pipe_fd[2];

typedef struct shmpid{
    int pid_1;
    int pid_2;
}PID;

int main(int argc, char *argv[])
{
    //获取键值
    key_t key;
    if((key = ftok(".", 100)) == -1)
    {
        perror("fail to ftok");
    }
    //创建一个共享内存
    int shmid;
    if((shmid = shmget(key, 500, IPC_CREAT | 0777)) == -1)
    {
        perror("fail to shmget");
    }
    //映射共享内存的地址
    PID *addr;
    if((addr = (PID *)shmat(shmid, NULL, 0)) == (PID *)-1)
    {
        perror("fail to shmat");
    }

    //创建有名管道 myfifo
    unlink(FIFO_NAME);
    if (mkfifo(FIFO_NAME, 0664) < 0 && errno != EEXIST)
    {
        perror("mkfifo");
        return 0;
    }

    if(pipe(pipe_fd) == -1) //无名管道
    {
        perror("fail to pipe");
        exit(1);
    }

    pid_t pid;
    int i = 0;
    for(i = 0; i < 2; i++)
    {
        pid = fork();
        if(pid < 0)
        {
            perror("fail to fork");
            exit(1);
        }
        if(pid == 0)
        {
            break;
        }
    }
    if(i == 0)
    {
        addr->pid_1 = getpid();  //获取播放器子进程号

        //重定向1
        dup2(pipe_fd[1], 1);

        execlp("mplayer", "mplayer", "-ac", "mad", "-slave", "-quiet", "-idle", "-input", "file=./myfifo", NULL); // execlp函数执行成功，后面都不会运行
        exit(0);  // 如果execlp失败，结束子进程
    }
    if(i == 1)
    {
        QApplication a(argc, argv);

        addr->pid_2 = getpid(); //获取界面子进程号

        Widget w(pipe_fd[0]);
        w.show();

        return a.exec();
    }
    if(i == 2)
    {
        while(1)
        {
            int ret= waitpid(-1, NULL, WNOHANG);
            if (ret == 0) //如果返回值=0，说明当前进程中还有子进程存在，且没有结束的子进程
            {
                continue;
            }
            else if (ret > 0)//如果返回值>0，说明当前进程中有子进程结束，返回此子进程的进程号
            {
                if(ret == addr->pid_2)
                {
                    kill(addr->pid_1, 9);
                }
            }
            else//ret == -1，当前进程中已没有子进程
            {
                break;
            }
        }
    }
}




