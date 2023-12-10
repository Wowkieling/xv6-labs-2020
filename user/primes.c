#include "kernel/types.h"
#include "user/user.h"

#define RD 0
#define WR 1

const uint INT_LEN = sizeof(int);

/**
 * @description: 读取左邻居通道的第一个数据；
 * 为每一个素数建立一个进程，一个管道，输出每个管道的第一个数据即不重复的输出所有素数
 * @param {int} lpipe 左邻居管道
 * @param {int} *dst 用于存储第一个数据的地址
 * @return {*} 如果没有数据返回-1，有数据返回0
 */
int lpipe_first_data(int lpipe[2], int *dst)
{
    if (read(lpipe[RD], dst, INT_LEN) == INT_LEN)
    {
        printf("prime %d\n", *dst);
        return 0;
    }
    
    return -1;
}

/**
 * @description: 筛选素数的算法：读取左邻居的数据，将不能被first整除的写入右邻居
 * @param {int} lpipe 左邻居的管道
 * @param {int} rpipe 当前管道
 * @param {int} l_first 左邻居管道的第一个数据
 * @return {*}
 */
void transmit_data(int lpipe[2], int pipe[2], int l_first)
{
    int data;
    while(read(lpipe[RD], &data, INT_LEN) == INT_LEN) {//逐INT_LEN读取左管道的数据
        //将不能被first整除的写入右邻居
        if (data % l_first)
        {
            write(pipe[WR], &data, INT_LEN);
        }
    }
    //左管道数据读完时，关闭以上打开的管道
    close(lpipe[RD]);
    close(pipe[WR]);
}

/**
 * @description: 寻找素数
 * @param {int} lpipe 左邻居管道
 * @return {*}
 */
void(primes(int lpipe[2]))
{
    close(lpipe[WR]);
    int first;
    if (lpipe_first_data(lpipe, &first) == 0)//递归不结束的条件
    {
        //新建当前的管道
        int p[2];
        pipe(p);
        //筛选素数的算法：读取左邻居的数据，将不能被first整除的写入右邻居
        transmit_data(lpipe, p, first);
        //递归
        if (fork() == 0)
        {
            primes(p);//在子进程中递归
        }
        else
        {
            //父进程，父进程关闭自己的管道的读写，并等待子进程关闭
            close(p[RD]);//p[WR]在transmit_data中已关闭
            wait(0);
        }
    }
    //递归结束的条件：lpipe_first_data(lpipe, &first)返回-1，即左邻居管道为空，没有第一个数据
    exit(0);
}

int main(int argc, char const *argv[])
{
    int p[2]; //第一个管道 读左边的2-35
    pipe(p);

    for (int i = 2; i <= 35; i++)
    {
        write(p[WR], &i, INT_LEN);
    }

    if (fork() == 0)
    {
        primes(p);
    }
    else
    {
        //父进程关闭自己的管道的读写，并等待子进程关闭
        close(p[WR]);
        close(p[RD]);
        wait(0);
    }
    exit(0);
}
