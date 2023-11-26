#include<netinet/in.h> // for sockaddr_in
#include<sys/types.h> // for socket
#include<sys/socket.h> // for socket
#include<stdio.h> // for printf
#include<stdlib.h> // for exit
#include<string.h> // for bzero
#include<time.h>
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

int main(int argc,char** argv)
{
    if(argc!=2)
    {
        printf("Usage: ./%s ServerIPAddress\n", argv[0]);
        exit(1);
    }

    // 设置一个socket地址结构client_addr, 代表客户机的internet地址和端口
    struct sockaddr_in6 client_addr;
    bzero(&client_addr,sizeof(client_addr));
    client_addr.sin6_family=AF_INET6;//internet协议簇
    client_addr.sin6_addr = in6addr_any; // INADDR_ANY表示自动获取本机地址
    client_addr.sin6_port = htons(2000); // auto allocated, 让系统自动分配一个空闲端口

    // 创建用于internet的流协议(TCP)类型socket，用client_socket代表客户端socket
    int client_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }

    // 把客户端的socket和服务器的socket地址结构绑定
    if(bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr))==-1)
    {
        printf("Client Bind Port Failed!\n");
        exit(1);
    }
    // 设置一个socket地址结构server_addr,代表服务器的internet地址和端口
    struct sockaddr_in6 server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    // 服务器的IP地址来自程序的参数
    if (inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr) < 0)
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin6_port = htons(2000);
    socklen_t server_addr_length = sizeof(server_addr);
    // 向服务器发起连接请求，连接成功后client_socket代表客户端和服务器端的一个socket连接
    if (connect(client_socket, (struct sockaddr*) & server_addr, server_addr_length) < 0)
    {
        printf("Can Not Connect To %s!\n", argv[1]);
        exit(1);
    }

    //输入要从服务器端获取的文件名称
    char file_name[FILE_NAME_MAX_SIZE + 1];
    bzero(file_name, sizeof(file_name));
    printf("Please Input File Name On Server:   ");
    scanf("%s", file_name);

    //询问客户端是否要下载上述文件
    printf("Start tramsmission? y or n \n");
    char m;
    m=getchar();  //消除换行符的影响
    char tfd;
    tfd=getchar();
    if(tfd=='n')
    {
        printf("File transfer canceled!\n");
        exit(1);
    }
    else
    {
        //time_t t0=time(NULL);
        //printf("File transfer start time:   ");
        //fputs(ctime(&t0), stdout);
        char buffer[BUFFER_SIZE];
        bzero(buffer, sizeof(buffer));
        strncpy(buffer, file_name, strlen(file_name) > BUFFER_SIZE ? BUFFER_SIZE : strlen(file_name));
        // 向服务器发送buffer中的数据，此时buffer中存放的是客户端需要接收的文件的名字
        send(client_socket, buffer, BUFFER_SIZE, 0);
        FILE* fp = fopen(file_name, "w");
        if (fp == NULL)
        {
            printf("File:\t%s Can Not Open To Write!\n", file_name);
            exit(1);
        }
        //从服务器端接收数据到buffer中
        bzero(buffer, sizeof(buffer));
        int length = 0;
        printf("Waiting for file transfer...\n");
        //接收文件传输信息
        char s[BUFFER_SIZE];
        bzero(s, sizeof(s));
        recv(client_socket, s, BUFFER_SIZE, 0);
        printf("%s\n",s);

        //传输文件
        while (length = recv(client_socket, buffer, sizeof(buffer), 0))
        {
            if(length<0)
            {
                printf("Recieve Data From Server %s Failed!\n", argv[1]);
                break;
            }
            int write_length = fwrite(buffer, sizeof(char), length, fp);
            if(write_length<length)
            {
                printf("File:\t%s Write Failed!\n", file_name);
                break;
            }
            bzero(buffer,sizeof(buffer));
        }
        printf("Recieve File:   %s From Server[%s] Finished!\n", file_name, argv[1]);
        time_t t1=time(NULL);
        printf("File transfer finish time:   ");
        fputs(ctime(&t1), stdout);
    }
    // 传输完毕，关闭socket
    close(client_socket);
    return 0;
}