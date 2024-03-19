#include <stdio.h>
// #include <malloc.h>
#include "video_decoder.h"
#include <malloc.h>
#include "main.h"

int rgb2photo(Frame *frame);

int main(){
    int curren_index,all_fps;
    double curren_fps;
    Frame frame;
    if(decoder_init("./dragon.mp4")==0){
        printf("hello\n");
        all_fps = get_total_frames();
        frame = decoder_get_frame();

        curren_index = get_frame_index();
        curren_fps = get_fps();
        printf("all_fps=%d,curren_index=%d,curren_fps=%f\n",all_fps,curren_index,curren_fps);
        printf("width=%d,height=%d,linesize=%d\n",frame.width,frame.height,frame.linesize);
    //    printf("%ld\n",sizeof(frame.data)/sizeof(unsigned char));
        
     //   printf("\e[=17h");
        rgb2photo(&frame);

        decoder_close();
    }
    else if(decoder_init("./dragon.mp4")==-1)
        printf("initialize failed.\n");

    return 0;
}

//采用：(0.2126 * red + 0.7152 * green + 0.0722 * blue)
//r * 0.299 + g * 0.587 + b * 0.114--另一种灰度算法（未采用）
//灰度等级：9（0-8）
//grays = "@%#*+=-:. "   #由于控制台是白色背景，所以先密后疏/黑色背景要转置一下
int rgb2photo(Frame *frame){
    int i,j,width,height,tmp,r1,g1,b1;
    double r,g,b;
    width = frame->width,height = frame->height;
    int line_gray[height][width];
    unsigned char *loc = frame->data;
    char grays[10] = " .:-=+*#@";

 //   width=height=50; 
    for(j=0;j<height;j++){
        for(i=0;i<width;i++){
        r = loc[j*(i-1)+3*i];//red
        g = loc[j*(i-1)+3*i+1];//green
        b = loc[j*(i-1)+3*+2];//blue
        r1 = (int)r;
        g1 = (int)g;
        b1 = (int)b;
        tmp = (int)(0.2126*r+0.7152*g+0.0722*b);//rgb转灰度值
        line_gray[j][i]= tmp%9;//0-8
    //    printf("%c",grays[line_gray[i]]);
        printf("\e[38;2;%d;%d;%dm\e[48;2m%c\e[0m",r1,g1,b1,grays[line_gray[j][i]]);       
    //   printf("%d",line_gray[i]);
        }
    //    for(i=0;i<a;i++) printf("%c",grays[line_gray[i]]);
    //    for(i=0;i<a;i++) printf("%d ",line_gray[i]);//
        printf("\n");
    }
    
    return 0;
}

 //   2*2步进
    // int resized_gray[height / 2][width / 2];
    // for (int i = 0; 2 * i < height; i++)
    // {
    //     for (int j = 0; 2 * j < width; j++)
    //     {
    //         int sum = line_gray[2 * i][2 * j] + line_gray[2 * i][2 * j + 1] + line_gray[2 * i + 1][2 * j] + line_gray[2 * i + 1][2 * j + 1];
    //         resized_gray[i][j] = (int)((double)sum / 4.0 * 8 / 255.0);
    //         printf("%c", grays[resized_gray[i][j]]);
    //     }
    //     printf("\n");
    // }

    
// 互斥锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// 视频帧缓冲区
#define BUFFER_SIZE 100
int buffer1[BUFFER_SIZE];
int buffer2[BUFFER_SIZE];
int buffer1_count = 0;
int buffer2_count = 0;

// Resize 方法1:gray-average
void resize_method1(Frame *frame) {
    printf("Method gray-average: Resizing frame %d\n", frame);
    // 实现 Resize 方法1 的代码
}

// Resize 方法2:gray-max
void resize_method2(Frame *frame) {
    printf("Method gray-color: Resizing frame %d\n", frame);
    // 实现 Resize 方法2 的代码
}

// Resize 方法3:color-average
void resize_method3(Frame *frame) {
    printf("Method color-average: Resizing frame %d\n", frame);
    // 实现 Resize 方法3 的代码
}

// Resize 方法4:color-max
void resize_method4(Frame *frame) {
    printf("Method color-max: Resizing frame %d\n", frame);
    // 实现 Resize 方法4 的代码
}

// 视频处理线程函数
void* video_process_thread(void* arg) {
    int selected_method = *(int*)arg; // 获取用户选择的 resize 方法
    int total_frames = get_total_frames(); // 获取视频总帧数
    printf("Total frames: %d\n", total_frames);

    for (int i = 0; i < total_frames; i++) {
        pthread_mutex_lock(&mutex);
        Frame frame =  decoder_get_frame();
        memcpy();
        
    //    usleep(100000); // 模拟解码时间
        // 根据用户选择的方法调用相应的 resize 函数
        switch (selected_method) {
            case 1:
                resize_method1(&frame);
                break;
            case 2:
                resize_method2(&frame);
                break;
            case 3:
                resize_method3(&frame);
                break;
            case 4:
                resize_method4(&frame);
                break;
            default:
                printf("Invalid resize method.\n");
                break;
        }
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

// 视频打印线程函数
void* video_print_thread(void* arg) {
    int total_frames = get_total_frames(); // 获取视频总帧数

    while (1) {
        pthread_mutex_lock(&mutex);
        if (buffer2_count > 0) {
            // 从缓冲区2读取数据并打印
            int frame = buffer2[--buffer2_count];
            printf("Printing frame: %d\n", frame);
        }
        pthread_mutex_unlock(&mutex);

        // 检查是否打印完所有帧
        if (buffer2_count == total_frames) {
            printf("All frames printed.\n");
            break;
        }
    }

    return NULL;
}

int main() {
    pthread_t process_thread, print_thread;
    int selected_method = 1; //默认使用gray_average池化图像
    if(decoder_init("./videos/dragon.mp4")!=0){
        printf("Initialize failed!\n");
        exit(1);
    }//初始化解码器，打开视频

    pthread_create(&process_thread, NULL, video_process_thread, (void*)&selected_method);//视频处理线程，传递 resize 方法作为参数
    pthread_create(&print_thread, NULL, video_print_thread, NULL);//视频打印线程
    
    pthread_join(process_thread, NULL);// 等待视频处理线程结束
    pthread_join(print_thread, NULL);// 等待视频打印线程结束

    decoder_close();//关闭解码器
    return 0;
}

// int main(int argc, char *argv[])
// {                                          
//   int opt,flags;
//   char *fvalue,*rvalue;

//   flags = 0;
//   char *optstring = "hvc::f:r:"; 
//   while ((opt = getopt(argc, argv, optstring)) != -1) {
//     switch(opt) {
//       case 'h': //帮助文档
//         flags = 1;
//         printf("Usage: %s [options]\n", argv[0]);
//         printf("Options:\n");
//         printf("  -h    Display this help message\n");
//         printf("  -v    Display the version of the program\n");
//         printf("  -c,   Display this help message\n");
//         printf("  -f,   Display the version of the program\n");
//         printf("  -r,   Display this help message\n");
//         break;
//       case 'v': //版本信息
//         flags = 2;
//         printf("dian-player v1.0\n");
//         exit(0);
//         break;
//       case 'c': //如果有-c，彩色打印；如果没有，默认灰度打印
//         flags = 3;
//         example(3);
//         break;
//       case 'f': //视频文档路径
//         flags = 4;
//         fvalue = optarg;
//         break;
//       case 'r': //降采样窗口和步长
//         flags = 5;
//         break;
//       default:
//         flags = 7;
//         printf("wrong input!\n");
//     }
//   }
// //  printf("flags=%d; optind=%d; avalue=%s; bvalue=%s\n", flags, optind, avalue,bvalue);

//   exit(EXIT_SUCCESS);
// }