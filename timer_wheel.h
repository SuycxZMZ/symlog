#ifndef TIMER_WHEEL_H
#define TIMER_WHEEL_H

#include <time.h>
#include <netinet/in.h>
#include <stdio.h>

#define BUFFER_SIZE 64
class tw_timer;

// 用户数据结构
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[ BUFFER_SIZE ];
    tw_timer* timer;
};

// 时间轮的定时器类
class tw_timer
{
public:
    tw_timer( int rot, int ts ) 
    : next( NULL ), prev( NULL ), rotation( rot ), time_slot( ts ){}

public:
    // 记录定时器在时间轮转多少圈之后生效
    int rotation;

    // 记录定时器属于哪一个槽
    int time_slot;

    // 回调
    void (*cb_func)( client_data* );

    // 指向用户数据的指针
    client_data* user_data;

    // 链表指针
    tw_timer* next;
    tw_timer* prev;
};

// 时间轮类
class time_wheel
{
public:
    time_wheel() : cur_slot( 0 )
    {
        // 初始化每个槽
        for( int i = 0; i < N; ++i )
        {
            slots[i] = NULL;
        }
    }
    ~time_wheel()
    {
        // 遍历每个槽以及每个槽所指向的链表, 依次销毁每个定时器
        for( int i = 0; i < N; ++i )
        {
            tw_timer* tmp = slots[i];
            while( tmp )
            {
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
        }
    }

    // 根据 timeout 创建一个定时器, 并插入时间轮, 
    // 返回创建的定时器指针, 用户拿到指针后再加入对应的用户数据
    tw_timer* add_timer( int timeout )
    {
        if( timeout < 0 )
        {
            return NULL;
        }

        // 计算定时器在多少个 SI 时间后触发
        int ticks = 0;
        if( timeout < TI )
        {
            ticks = 1;
        }
        else
        {
            ticks = timeout / TI;
        }

        // 计算定时器在轮子转动多少圈之后触发
        int rotation = ticks / N;

        // 计算插入槽
        int ts = ( cur_slot + ( ticks % N ) ) % N;

        // 创建定时器
        tw_timer* timer = new tw_timer( rotation, ts );

        // 插入对应槽, 头插法, 速度快
        if( !slots[ts] )
        {
            printf( "add timer, rotation is %d, ts is %d, cur_slot is %d\n", rotation, ts, cur_slot );
            slots[ts] = timer;
        }
        else
        {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }

    // 删除目标定时器, 当轮子的槽较多时, 复杂度接近 O(n)
    void del_timer( tw_timer* timer )
    {
        if( !timer )
        {
            return;
        }

        // 拿出所在槽索引
        int ts = timer->time_slot;

        // 删除操作
        // 头节点特殊处理
        if( timer == slots[ts] )
        {
            slots[ts] = slots[ts]->next;
            if( slots[ts] )
            {
                slots[ts]->prev = NULL;
            }
            delete timer;
        }
        // 其他节点, 双链表的删除操作
        else
        {
            timer->prev->next = timer->next;
            if( timer->next )
            {
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
    }

    // 心跳函数, 每次跑完之后轮子向前走一个槽
    void tick()
    {
        tw_timer* tmp = slots[cur_slot];
        printf( "current slot is %d\n", cur_slot );

        // 遍历当前槽所指向的链表
        while( tmp )
        {
            printf( "tick the timer once\n" );

            // rotation > 0, 则本轮先不做处理
            if( tmp->rotation > 0 )
            {
                tmp->rotation--;
                tmp = tmp->next;
            }

            // 处理到期定时器
            else
            {
                // 跑回调
                tmp->cb_func( tmp->user_data );

                // 头节点删除特殊处理
                if( tmp == slots[cur_slot] )
                {
                    printf( "delete header in cur_slot\n" );
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if( slots[cur_slot] )
                    {
                        slots[cur_slot]->prev = NULL;
                    }
                    tmp = slots[cur_slot];
                }
                else
                {
                    tmp->prev->next = tmp->next;
                    if( tmp->next )
                    {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer* tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }
        }

        // 轮子向后走一个槽
        cur_slot = ++cur_slot % N;
    }

private:
    // 轮子上槽的数量
    static const int N = 60;

    // 轮子转一下的时间间隔为 1s , 即为槽间距
    static const int TI = 1; 

    // 轮子的槽, 每个槽里存放一个指向定时器链表的指针
    tw_timer* slots[N];

    // 记录轮子当前槽, 随心跳函数更新
    int cur_slot;
};

#endif
