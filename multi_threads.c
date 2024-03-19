//level2：多线程函数（2），互斥锁；
//每次打印完成后记得检查是否还有下一帧，并且对全局变量总帧数进行自减
//队列：一个就可以，入队是解码后的入队，出队也是解码后的出队
//先实现平均池化的灰度图吧
#include "video_decoder.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// 定义队列结构
//上锁的是这个队列，队列保存帧数据
//重点：队列的内容物是什么数据

typedef struct QueueNode{
    Frame frameData;
    struct QueueNode *next;
}QueueNode;

typedef QueueNode *LinkQueue;
LinkQueue *Q;

#define QUEUE_SIZE 100
void* queue[QUEUE_SIZE];
int front = 0;
int rear = 0;

// 互斥锁和条件变量
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// 入队操作
void enqueue(void* frame) {
    pthread_mutex_lock(&mutex);

    // 将帧指针存入队列
    queue[rear] = frame;
    rear = (rear + 1) % QUEUE_SIZE;

    // 发送条件变量信号，通知出队线程有新的数据可以处理
    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&mutex);
}

// 出队操作
void* dequeue() {
    pthread_mutex_lock(&mutex);

    // 当队列为空时，等待条件变量
    while (front == rear) {
        pthread_cond_wait(&cond, &mutex);
    }

    // 从队列中取出帧指针
    void* frame = queue[front];
    front = (front + 1) % QUEUE_SIZE;

    pthread_mutex_unlock(&mutex);

    return frame;
}

void* rgb2gray_aver(Frame *frame,int window1,int step1)//假设传入参数窗口window，步长step
{
    int i, j, r, g, b;
    int width = frame->width, height = frame->height;
    int line_gray[height][width];
    unsigned char *loc = frame->data;

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            r = loc[(3 * i) + j * frame->linesize];              // red
            g = loc[3 * i + 1 + j * frame->linesize];            // green
            b = loc[3 * i + 2 + j * frame->linesize];            // blue
            line_gray[j][i] = 0.299 * r + 0.587 * g + 0.114 * b; // 0-8
        }
    }//RGB转灰度

    int m,n,sum;//k用于内层循环,sum用于求和取平均
    int window=window1,step=step1;
    int width1,height1;
    float num = (float)(window*window);
    width1 = (width-window+step)/step;
    height1 = (height-window+step)/step;
    int resized_gray[height1][width1];
    for(i=0;i<height1;i++){
        for(j=0;j<width1;j++){
            sum = 0;
            for(n=0;n<window;n++){
                for(m=0;m<window;m++){
                sum += line_gray[step*i+n][step*j+m];
                }
            }
            resized_gray[i][j] = (int)((double)sum / num * 8 / 255.0);
        }
    }
    //return 0;
}

// 视频处理线程函数
void* video_process_thread(void* arg) {
    // 假设视频帧大小为 1024 字节
    int frame_size = 1024;
    int total_frames = *(int*)arg; // 获取视频总帧数

    for (int i = 0; i < total_frames; i++) {
        // 模拟解码操作，分配内存并存储帧数据
        void* frame = malloc(frame_size);
        // 模拟处理帧数据
        usleep(100000); // 模拟解码时间
        enqueue(frame);
    }

    return NULL;
}

// 视频打印线程函数
void* video_print_thread(void* arg) {
    int total_frames = *(int*)arg; // 获取视频总帧数

    for (int i = 0; i < total_frames; i++) {
        // 从队列中取出帧指针并处理
        void* frame = dequeue();
        // 模拟打印帧数据
        printf("Printing frame: %p\n", frame);
        // 释放帧数据内存
        free(frame);
    }

    return NULL;
}

int main() {
    pthread_t process_thread, print_thread;
    if(decoder_init("./videos/dragon.mp4")!=0){
        printf("Initialize failed!\n");
        exit(1);
    }//初始化解码器，打开视频
    int total_frames = get_total_frames();

    // 创建视频处理线程
    pthread_create(&process_thread, NULL, video_process_thread, &total_frames);
    // 创建视频打印线程
    pthread_create(&print_thread, NULL, video_print_thread, &total_frames);
   
    // 等待视频处理线程结束
    pthread_join(process_thread, NULL);
    // 等待视频打印线程结束
    pthread_join(print_thread, NULL);

    decoder_close();//关闭解码器
    return 0;
}