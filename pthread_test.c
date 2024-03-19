//pthread example--zhihu

#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include <unistd.h>

void producer();
void consumer();

int buf = 0;//确保多个线程按顺序执行？
pthread_mutex_t mut;//互斥锁

//生产者：生成成品
void producer()
{
    while(1)//？
    {
        pthread_mutex_lock(&mut);//锁住，占用资源空间
        if(buf == 0)
        {
            buf = 1;
            printf("produced an item.\n");
            sleep(1);
        }
        pthread_mutex_unlock(&mut);//解锁
    }

}

//消费者：取走成品
void consumer()
{
    while(1)
    {
        pthread_mutex_lock(&mut);
        if(buf == 1)
        {
            buf = 0;
            printf("consumed an item.\n");
            sleep(1);
        }
        pthread_mutex_unlock(&mut);
    }
}

int main(void)
{
    pthread_t thread1,thread2;
    pthread_mutex_init(&mut,NULL);//初始化
    pthread_create(&thread1,NULL,(void *)&producer,NULL);
    consumer(&buf);//为什么传入buf地址？buf不是全局吗？
    pthread_mutex_destroy(&mut);
    
    return 0;
}
//生产者消费者模型演示代码