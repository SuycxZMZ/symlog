#ifndef TIMER_HEAP_H
#define TIMER_HEAP_H

#include <iostream>
#include <netinet/in.h>
#include <time.h>
using std::exception;

#define BUFFER_SIZE 64

class heap_timer;

// 用户数据结构
struct client_data
{
    // 客户端socket地址
    sockaddr_in address;

    // 对应的客户端socket文件描述符
    int sockfd;

    // 读缓存
    char buf[ BUFFER_SIZE ];

    // 定时器指针
    heap_timer* timer;
};

// 定时器类
class heap_timer
{
public:
    heap_timer( int delay )
    {
        expire = time( NULL ) + delay;
    }

public:
    // 定时器生效绝对时间
    time_t expire;

    // 定时器执行的回调函数指针
    void (*cb_func)( client_data* );

    // 携带的用户数据指针
    client_data* user_data;
};


// 时间堆类
class time_heap
{
public:
    // 初始化一个容量为cap 的空堆
    time_heap( int cap ) throw ( std::exception )
        : capacity( cap ), cur_size( 0 )
    {

        // 构造函数中 new[], 析构函数中 delete [] 
	    array = new heap_timer* [capacity];

        if ( ! array )
        {
            throw std::exception();
        }

        // 初始化每一个指针 heap_timer*
        for( int i = 0; i < capacity; ++i )
        {
            array[i] = NULL;
        }
    }

    // 构造的一个重载
    time_heap( heap_timer** init_array, int size, int capacity ) throw ( std::exception )
        : cur_size( size ), capacity( capacity )
    {
        if ( capacity < size )
        {
            throw std::exception();
        }

         
        array = new heap_timer* [capacity];
        if ( ! array )
        {
            throw std::exception();
        }


        for( int i = 0; i < capacity; ++i )
        {
            array[i] = NULL;
        }

        // 构造出最小堆
        if ( size != 0 )
        {
            // 先插入每个元素
            for ( int i =  0; i < size; ++i )
            {
                array[ i ] = init_array[ i ];
            }

            // 再调整堆, 从下到上, 每个元素都做向下调整的动作
            for ( int i = (cur_size-1)/2; i >=0; --i )
            {
                percolate_down( i );
            }
        }
    }
    ~time_heap()
    {
        for ( int i =  0; i < cur_size; ++i )
        {
            delete array[i];
        }
        delete [] array; 
    }

public:

    // 添加定时器 O(lon n) 复杂度
    void add_timer( heap_timer* timer ) throw ( std::exception )
    {
        if( !timer )
        {
            return;
        }

        // 扩容, 类似 vector 的扩容
        if( cur_size >= capacity )
        {
            resize();
        }
        int hole = cur_size++;
        int parent = 0;

        // 尾部插入, 做上移动作 
        for( ; hole > 0; hole=parent )
        {
            parent = (hole-1)/2;
            if ( array[parent]->expire <= timer->expire )
            {
                break;
            }
            array[hole] = array[parent];
        }
        array[hole] = timer;
    }

    // 删除定时器
    void del_timer( heap_timer* timer )
    {
        if( !timer )
        {
            return;
        }
        // 延迟销毁, 将目标定时器的回调函数设置为空
        // 如果真正的删除操作, 则需要将该定时器后面的元素挨个向前移动, 浪费时间
        // 这是一种空间换时间的做法
        timer->cb_func = NULL;
    }

    // 获得堆顶部计时器
    heap_timer* top() const
    {
        if ( empty() )
        {
            return NULL;
        }
        return array[0];
    }

    // 弹出堆顶元素
    void pop_timer()
    {
        if( empty() )
        {
            return;
        }
        if( array[0] )
        {
            delete array[0];
            // 堆顶元素替换为最后一个元素
            array[0] = array[--cur_size];
            // 下移操作
            percolate_down( 0 );
        }
    }

    // 心跳函数
    void tick()
    {
        heap_timer* tmp = array[0];
        // 当前时间
        time_t cur = time( NULL );

        // 处理堆中到期的定时器,
        // 每次都能取到最小的, 一旦遇到大于当前时间的就跳出循环, 结束函数
        while( !empty() )
        {
            if( !tmp )
            {
                break;
            }
            if( tmp->expire > cur )
            {
                break;
            }

            // 执行堆顶计时器的任务
            if( array[0]->cb_func )
            {
                array[0]->cb_func( array[0]->user_data );
            }

            // 执行之后便弹出堆顶元素
            pop_timer();
            tmp = array[0];
        }
    }

    void adjust_timer( heap_timer* timer, int socketfd)
    {
        // 找到timer对应的索引
        for (int i = 0; i < cur_size; i ++)
        {
            if ( (array[i]->user_data)->sockfd == socketfd )
            {
                percolate_down(i);
                return;
            }
        }
    }

    bool empty() const { return cur_size == 0; }

private:

    // 下坠调整操作, 确保堆数组中以第 hole 个元素为根的子树符合小根堆
    // 用该函数的前提是该子树只有根可能不满足小根堆, 要对其调整
    void percolate_down( int hole )
    {
        heap_timer* temp = array[hole];
        int child = 0;
        for ( ; ((hole*2+1) <= (cur_size-1)); hole=child )
        {
            // 左右孩子找出较小的为 child 
            child = hole*2+1;
            if ( (child < (cur_size-1)) && (array[child+1]->expire < array[child]->expire ) )
            {
                ++child;
            }

            // hole 与 child 作比较, 若 child 小, 则交换
            if ( array[child]->expire < temp->expire )
            {
                array[hole] = array[child];
            }

            // 若 hole 小, 则已经下坠完成, 跳出循环
            else
            {
                break;
            }
        }
        array[hole] = temp;
    }

    // 扩容
    void resize() throw ( std::exception )
    {
        heap_timer** temp = new heap_timer* [2*capacity];
        for( int i = 0; i < 2*capacity; ++i )
        {
            temp[i] = NULL;
        }
        if ( ! temp )
        {
            throw std::exception();
        }
        capacity = 2*capacity;
        for ( int i = 0; i < cur_size; ++i )
        {
            temp[i] = array[i];
        }
        delete [] array;
        array = temp;
    }


private:
    // 堆数组, 每个元素都是一个指向 heap_timer 类型元素的指针, 节省空间
    heap_timer** array;

    // 堆数组的容量
    int capacity;

    // 堆数组当前包含的元素个数
    int cur_size;
};


#endif
