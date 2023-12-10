/*
 * @Author: lhl
 * @Date: 2023-09-24 16:27:25
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2023-12-10 16:18:09
 * @Description: find <directory> <filename>
 */
#include "kernel/types.h"

#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

void find(char *path, const char *filename)
{
    char buf[512], *p; // 缓冲区和对应的当前指针
    int fd;
    struct dirent de; // 存储directory的信息
    struct stat st; //存储file system的信息

    // 打开path，如果失败, 向error描述符2写入错误信息，并返回
    if((fd = open(path, 0)) < 0) 
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // 打开path成功后，获取fd的状态，如果获取状态失败，向2写入错误信息，并关闭打开的fd，并返回
    if(fstat(fd, &st) < 0)
    {
        fprintf(2, "fstat: cannot fstat %s\n", path);
        close(fd);
        return;
    }

    // 获取fd状态成功，path应为directory，如果path不为dir，报错并返回
    if(st.type != T_DIR)
    {
        fprintf(2, "usage: find <directory> <filename>\n");
        return;
    }

    // 如果path为dir，判断path的长度不能超过缓冲区buf
    // 具体来说：strlen(path) + 1 是路径字符串长度及分隔符'/'
    //          DIRSIZ + 1 是预留的目录内的文件名字符串长度及最后一个null结束标志字符
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
    {
        printf("sizeof(buf): %d\n", sizeof(buf));
        printf("sizeof buf: %d\n", sizeof buf);
        fprintf(2, "find: path too long\n");
        return;
    }

    strcpy(buf, path); //path复制到buf，二者都是指针
    p = buf + strlen(buf); // p指向目录最后一个字符null
    *p++ = '/';  // *p = '/', 并令p指向最后一个'/'之后

    while(read(fd, &de, sizeof de) == sizeof de) {
        if(de.inum == 0)
            continue; //表示这是一个空目录项，代码会跳过这个目录项，继续下一次循环。
        memmove(p, de.name, DIRSIZ); // 使用 memmove 函数将目录项的名称 de.name 复制到缓冲区 buf 中 p 指向的位置。这相当于在路径末尾添加了目录项的名称。
        p[DIRSIZ] = 0;//字符串结束标志

        //使用 stat 函数获取新路径（包含目录项名称）的状态信息，并将结果存储在 st 结构体中。
        //如果获取失败，输出错误信息，并继续下一次循环。
        if(stat(buf, &st) < 0)
        {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }
         //不要在“.”和“..”目录中递归
        if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) 
        {
            find(buf, filename);
        } 
        else if (strcmp(filename, p) == 0)
            printf("%s\n", buf);   
    }

    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(2, "usage: find <directory> <filename>\n");
        exit(1);
    }

    find(argv[1], argv[2]); // find(directory, filename)
    exit(0);
}