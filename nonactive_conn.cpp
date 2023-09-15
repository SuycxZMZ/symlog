#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "timer_heap.h"

// 服务端代码

#define FD_LIMIT 65535
#define MAX_EVENT_NUMBER 1024
// 测试定时时间间隔
#define TIMESLOT 5

static int pipefd[2];
static time_heap timer_heap(100);
static int epollfd = 0;

// 设置文件描述符非阻塞
int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

// 往 epoll 实例中添加监控的描述符
void addfd( int epollfd, int fd )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );

    // 全部设置为非阻塞
    setnonblocking( fd );
}

// 信号处理函数 把信号写进管道
void sig_handler( int sig )
{
    int save_errno = errno;
    int msg = sig;
    send( pipefd[1], ( char* )&msg, 1, 0 );
    errno = save_errno;
}

// 添加信号
void addsig( int sig )
{
    struct sigaction sa;
    memset( &sa, '\0', sizeof( sa ) );
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset( &sa.sa_mask );
    assert( sigaction( sig, &sa, NULL ) != -1 );
}


void timer_handler()
{
    // 定时处理任务
    timer_heap.tick();
    
    // 重新定时, 一次alarm调用只会引起一次SIGALARM信号
    alarm( TIMESLOT );
}

// 定时器回调函数, 删除非活动连接 socket 上的注册事件, 并关闭 socket
void cb_func( client_data* user_data )
{
    epoll_ctl( epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0 );
    assert( user_data );
    close( user_data->sockfd );
    printf( "close fd %d\n", user_data->sockfd );
}

int main( int argc, char* argv[] )
{
    if( argc <= 2 )
    {
        printf( "usage: %s ip_address port_number\n", basename( argv[0] ) );
        return 1;
    }

    // 转换 ip地址和端口号
    const char* ip = argv[1];
    int port = atoi( argv[2] );

    int ret = 0;
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons( port );

    // 创建 socket 文件描述符
    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert( listenfd >= 0 );

    // 绑定到前面转换得到的 ip地址和端口号
    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret != -1 );

    // 监听 连接socket
    ret = listen( listenfd, 5 );
    assert( ret != -1 );

    // 创建epoll实例, 先加入 连接socket
    epoll_event events[ MAX_EVENT_NUMBER ];
    int epollfd = epoll_create( 5 );
    assert( epollfd != -1 );
    addfd( epollfd, listenfd );

    // 绑定管道
    ret = socketpair( PF_UNIX, SOCK_STREAM, 0, pipefd );
    assert( ret != -1 );
    // 信号写入端
    setnonblocking( pipefd[1] );
    // epoll监控端
    addfd( epollfd, pipefd[0] );

    // add all the interesting signals here
    addsig( SIGALRM );
    addsig( SIGTERM );
    bool stop_server = false;

    // 用户数据
    client_data* users = new client_data[FD_LIMIT]; 
    bool timeout = false;
    // 定时
    alarm( TIMESLOT );

    while( !stop_server )
    {
        // 阻塞等待 IO 事件
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );
        if ( ( number < 0 ) && ( errno != EINTR ) )
        {
            printf( "epoll failure\n" );
            break;
        }
    
        // 挨个处理
        for ( int i = 0; i < number; i++ )
        {
            int sockfd = events[i].data.fd;

            // 处理新到达的连接
            if( sockfd == listenfd )
            {
                // 接受并保存客户数据
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
                addfd( epollfd, connfd );
                users[connfd].address = client_address;
                users[connfd].sockfd = connfd;

                // 为该用户创建定时器, 设置其回调函数与超时时间
                heap_timer* timer = new heap_timer(1);
                timer->user_data = &users[connfd];

                // 超时处理函数
                timer->cb_func = cb_func;

                time_t cur = time( NULL );
                // 超时时间
                timer->expire = cur + 3 * TIMESLOT;
                users[connfd].timer = timer;

                // 添加到定时器堆
                timer_heap.add_timer( timer );
            }

            // 处理信号
            else if( ( sockfd == pipefd[0] ) && ( events[i].events & EPOLLIN ) )
            {
                int sig;
                char signals[1024];

                // 从 pipefd[0] 读出信号到 signals
                ret = recv( pipefd[0], signals, sizeof( signals ), 0 );
                if( ret == -1 )
                {
                    // handle the error
                    continue;
                }
                else if( ret == 0 )
                {
                    continue;
                }
                // 解析信号
                else
                {
                    for( int i = 0; i < ret; ++i )
                    {
                        switch( signals[i] )
                        {
                            case SIGALRM:
                            {
                                // 更改定时事件标志
                                timeout = true;
                                break;
                            }
                            case SIGTERM:
                            {
                                stop_server = true;
                            }
                        }
                    }
                }
            }

            // 处理客户连接上的数据
            else if(  events[i].events & EPOLLIN )
            {
                memset( users[sockfd].buf, '\0', BUFFER_SIZE );
                ret = recv( sockfd, users[sockfd].buf, BUFFER_SIZE-1, 0 );
                printf( "get %d bytes of client data %s from %d\n", ret, users[sockfd].buf, sockfd );
                heap_timer* timer = users[sockfd].timer;

                if( ret < 0 )
                {
                    // 非阻塞socket没有数据到达返回EAGAIN 不用管
                    // 错误处理, 直接删除对应socket与timer
                    if( errno != EAGAIN )
                    {
                        cb_func( &users[sockfd] );
                        if( timer )
                        {
                            timer_heap.del_timer( timer );
                        }
                    }
                }

                // 对方关闭连接
                else if( ret == 0 )
                {
                    cb_func( &users[sockfd] );
                    if( timer )
                    {
                        timer_heap.del_timer( timer );
                    }
                }
                else
                {
                    // 该客户连接上可读, 调整定时器
                    //send( sockfd, users[sockfd].buf, BUFFER_SIZE-1, 0 );
                    if( timer )
                    {
                        time_t cur = time( NULL );
                        timer->expire = cur + 3 * TIMESLOT;
                        printf( "adjust timer once\n" );
                        timer_heap.adjust_timer( timer, sockfd );
                    }
                }
            }
            else
            {
                // others
            }
        }

        // 最后处理定时任务, 缺点是导致定时任务不能精确按照预期的时间执行
        if( timeout )
        {
            timer_handler();
            timeout = false;
        }
    }

    close( listenfd );
    close( pipefd[1] );
    close( pipefd[0] );
    delete [] users;
    return 0;
}
