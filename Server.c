#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<sys/stat.h>
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

int main(int argc, char** argv)
{
    // set socket's address information
    // 设置一个socket地址结构server_addr,代表服务器internet的地址和端口
    struct sockaddr_in6 server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(2000);
    // create a stream socket
    // 创建用于internet的流协议(TCP)socket，用server_socket代表服务器向客户端提供服务的接口
    int server_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    // 把socket和socket地址结构绑定
    if (bind(server_socket, (struct sockaddr*) & server_addr, sizeof(server_addr)) == -1)
    {
        printf("Server Bind Port: %d Failed!\n", 2000);
        exit(1);
    }
    // server_socket用于监听
    if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE) == -1)
    {
        printf("Server Listen Failed!\n");
        exit(1);
    }
    while(1)// 服务器端一直运行，用以持续为客户端提供服务
    {
        // 定义客户端的socket地址结构client_addr，当收到来自客户端的请求后，调用accept
        // 接受此请求，同时将client端的地址和端口等信息写入client_addr中
        struct sockaddr_in6 client_addr;
        socklen_t length = sizeof(client_addr);
        // 接受一个从client端到达server端的连接请求,将客户端的信息保存在client_addr中
        // 如果没有连接请求，则一直等待直到有连接请求为止，这是accept函数的特性，可以用select()来实现超时检测
        // accpet返回一个新的socket,这个socket用来与此次连接到server的client进行通信
        // 这里的file_socket代表了这个通信通道
        int file_socket = accept(server_socket, (struct sockaddr*) & client_addr, &length);
        if (file_socket < 0)
        {
            printf("Server Accept Failed!\n");
            break;
        }
        char buffer[BUFFER_SIZE];
        bzero(buffer, sizeof(buffer));
        length = recv(file_socket, buffer, BUFFER_SIZE, 0);//服务器从用户端接收要传输的文件名
        if (length < 0)
        {
            printf("Server Recieve Data Failed!\n");
            break;
        }
        char file_name[FILE_NAME_MAX_SIZE + 1];
        bzero(file_name, sizeof(file_name));
        strncpy(file_name, buffer,strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));
        printf("The file to be sent is: %s\n",file_name);
        FILE* fp = fopen(file_name, "rb");
        if(fp==NULL)
        {
            printf("File:\t%s Not Found!\n", file_name);
        }
        else
        {
            //获取待传输文件的大小
            int len;
            struct stat fil;
            stat(file_name,&fil);
            len=fil.st_size;
            printf("File:\t%s is %d Byte.\n",file_name,len);
            //用户自定义传输速度
            printf("transmission speed:?Byte/s (If you want to transmit at maximum speed, please enter -1)\n");
            int speed;
            scanf("%d", &speed);
            //当输入-1时以最大速度传输
            if (speed==-1)
            {
                speed=5000;
            }
            char buffer2[speed];
            bzero(buffer2, speed);
            int file_block_length = 0;
            if(speed==5000)
            {
                //发送文件传输信息，包括开始时间、传输速率和预计传输时间等信息
                time_t t0=time(NULL);
                printf("File transfer start time:   ");
                fputs(ctime(&t0), stdout);
                printf("The transmission rate is the maximum!\n");

                char speed_1[]="The transmission rate is the maximum!\nEstimated transmission time < 5s!";
                char s_1[BUFFER_SIZE];//向发送端发送传输速度
                bzero(s_1, sizeof(s_1));
                strncpy(s_1, speed_1, strlen(speed_1) > BUFFER_SIZE ? BUFFER_SIZE : strlen(speed_1));
                // 向服务器发送s_1中的信息
                send(file_socket, s_1, BUFFER_SIZE, 0);

                while ((file_block_length = fread(buffer2, sizeof(char), speed, fp)) > 0)
                {
                    // 发送buffer中的字符串到new_server_socket,实际上就是发送给客户端
                    if (send(file_socket, buffer2, file_block_length, 0) < 0)
                    {
                        printf("Send File:  %s Failed!\n", file_name);
                        break;
                    }
                    bzero(buffer2, sizeof(buffer2));
                    usleep(1000000 * (1 - speed / 2560));
                }
            }
            else
            {
                time_t t0=time(NULL);
                printf("File transfer start time:   ");
                fputs(ctime(&t0), stdout);

                //计算传输时间
                int t;
                t=len/speed;
                t=t/60;
                printf("Estimated transmission time: %d min\n",t);

                //发送文件传输信息，包括开始时间、传输速率和预计传输时间等信息
                char speed_2[BUFFER_SIZE];
                sprintf(speed_2,"The transmission rate is %d Byte/s\nEstimated transmission time: %d min!\n",speed,t);
                char s_2[BUFFER_SIZE];//向发送端发送传输速度
                bzero(s_2, sizeof(s_2));
                strncpy(s_2, speed_2, strlen(speed_2) > BUFFER_SIZE ? BUFFER_SIZE : strlen(speed_2));
                // 向服务器发送s_2中的信息
                send(file_socket, s_2, BUFFER_SIZE, 0);

                while ((file_block_length = fread(buffer2, sizeof(char), speed, fp)) > 0)
                {
                     printf("file_block_length = %d Byte/s\n", file_block_length);
                     // 发送buffer中的字符串到new_server_socket,实际上就是发送给客户端
                     if (send(file_socket, buffer2, file_block_length, 0) < 0)
                     {
                           printf("Send File:\t%s Failed!\n", file_name);
                           break;
                     }
                bzero(buffer2, sizeof(buffer2));
                usleep(1000000 * (1 - speed / 2560));
                }
            }
            fclose(fp);
            printf("File:   %s Transfer Finished!\n", file_name);
            time_t t1=time(NULL);
            printf("File transfer finish time:   ");
            fputs(ctime(&t1), stdout);
        }
        close(file_socket);
    }
    close(server_socket);
    return 0;
}