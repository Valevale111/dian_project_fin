#include <stdio.h>
#include "video_decoder.h"
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h> // 添加头文件以使用布尔类型

//可以输入选项，没有要求默认灰度打印，有要求彩色打印
//可以输入步进窗口大小和步长
//看哪个方法效果好就默认哪个方法
int rgb2gray_aver(Frame *frame,int r1,int r2);//average pooling
int rgb2gray_max(Frame *frame,int r1,int r2);//max pooling
void rgb2color_aver(Frame *frame, int window, int step);//average pooling
void rgb2color_max(Frame *frame, int window, int step);//max pooling
int example(int my_method,int r1,int r2,char *fvalue);//主题文件

int do_name, do_gf_name;
char *l_opt_arg;
static const char *shortopts = "hvcf:r:";

struct option longopts[] = {
  {"help",    no_argument,       NULL, 'h'},
  {"version", no_argument,       NULL, 'v'},
  {"color",  no_argument  , NULL, 'c'},
  {NULL,    required_argument, NULL, 'f'},
  {NULL,    required_argument, NULL, 'r'},
  {0,         0,                 0,      0},
};

//命令行
int main (int argc, char *argv[])
{
    int c;
    int r1=0,r2=0,my_method=0;
    char *fvalue=NULL;

    while ((c = getopt_long (argc, argv, shortopts, longopts, NULL)) != -1)
    {
        switch (c)
        {
        case 'h'://帮助文档
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -h, --help       Display this help message\n");
            printf("  -v, --version    Display the version of the program\n");
            printf("  -c, --color      Print the video as RGB,else print as gray\n");
            printf("  -f,              Input your video's address,else play the default video\n");
            printf("  -r,              Input parameters for pooling(window>=2,step>=2),else use default parameters:window=2,step=2\n");
        //    printf("  -v, --version    Display the version of the program\n");
            exit(0);
            break;
        case 'v'://版本信息
            printf("dian-player v1.0\n");
            exit(0);
            break;
        case 'c'://如果有-c，彩色打印；如果没有，默认灰度打印
            my_method = 3;   //打印RGB视频默认使用average pooling
            break;
        case 'f': //视频文档路径
            fvalue = optarg;
            break;
        case 'r': //降采样窗口和步长
        //默认opind=1,多个参数的话需要-1
            r1 = atoi(argv[optind-1]);
            r2 = atoi(argv[optind]);
            break;
        default:
            printf("wrong input!\n");
            exit(1);
        }
    }
    
    //向视频操作函数传入得到的所有参数
    example(my_method,r1,r2,fvalue);

    return 0;
}

//主要功能实现函数
int example(int my_method,int r1,int r2,char *fvalue)//
{
    int curren_index, all_fps,time;
    int window=2,step=2;
    int method;
    if(my_method==0) method=1;//如果用户未要求彩色打印，默认使用gray-average pooling池化打印
    else{
        method = my_method;
    } 
    char *fname = NULL;
    if(!fvalue) fname =  "./videos/dragon.mp4";
    else{
        fname = fvalue;
        printf("fname=%s\n",fname);
    }//如果传入地址，则使用传入视频地址；否则播放默认视频
    if(r1>=2&&r2>=2){
        window = r1;
        step = r2;
    }
    bool paused = false; // 添加变量来跟踪暂停状态
    bool accelerated = false; // 添加变量来跟踪加速状态
    
    double curren_fps,my_fps;
    Frame frame;

    if (decoder_init(fname) == 0)
    {
        all_fps = get_total_frames();
        for (int i = 0; i < all_fps; i++)
        {
            frame = decoder_get_frame();
            frame = decoder_get_frame();
            frame = decoder_get_frame();
            frame = decoder_get_frame();
            frame = decoder_get_frame();
            frame = decoder_get_frame();
                switch(method){
                   case 1:
                       rgb2gray_aver(&frame,window,step);
                       break;
                   case 2:
                       rgb2gray_max(&frame,window,step);
                       break;
                   case 3:
                       rgb2color_aver(&frame,window,step);
                       break;
                   case 4:
                       rgb2color_max(&frame,window,step);
                       break;
                   default:
                       printf("wrong method!\n");
                }
            //printf("width=%d,height=%d,linesize=%d\n",frame.width,frame.height,frame.linesize);
            // curren_fps = get_fps();
            // curren_index = get_frame_index();
            // printf("fps:%f,index:%d\n",curren_fps,curren_index);
            // 休眠约33毫秒,fps=30帧
            my_fps = 30;
            time =(int) (1000000/my_fps * 4);
            usleep(time);
        //    usleep(33033);
            // 清空控制台
            printf ("\033c"); 
        }

        decoder_close();
    }
    else if (decoder_init("./videos/dragon.mp4") == -1)
        printf("initialize failed.\n");

    return 0;
}

// 采用： r * 0.299 + g * 0.587 + b * 0.114
// 灰度等级：9（0-8）
// grays = "@%#*+=-:. "   #由于控制台是白色背景，所以先密后疏/黑色背景要转置一下

//灰度图打印
//average pooling
int rgb2gray_aver(Frame *frame,int r1,int r2)
{
    int i, j, a, c;
    int r, g, b;
    int window=r1,step=r2;
    int width = frame->width, height = frame->height;
    int line_gray[height][width];
    unsigned char *loc = frame->data;
    char grays[9] = " .:-=+*#@";

    // 如果直接按照width*height二位打印，因为太大会溢出，报错segmentation fault
    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            r = loc[(3 * i) + j * frame->linesize];              // red
            g = loc[3 * i + 1 + j * frame->linesize];            // green
            b = loc[3 * i + 2 + j * frame->linesize];            // blue
            line_gray[j][i] = 0.299 * r + 0.587 * g + 0.114 * b; // 0-8
        }
    }

    //假设传入参数窗口window，步长step
    //已知原像素width*hight
    int m,n,sum;//k用于内层循环,sum用于求和取平均
    int width1,height1;
    float num = (float)(window*window);
    //是否能充分扫描
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
            printf("%c", grays[resized_gray[i][j]]); //灰度打印
        }
        printf("\n");
    }

    return 0;
}

//灰度图打印
//max pooling
//因为二维数组传参参数不固定不好传参，干脆创建了两个开头部分相同的函数，跳过了传参...
int rgb2gray_max(Frame *frame,int r1,int r2){
    int i, j, a, c;
    int r, g, b;
    int window=r1,step=r2;
    int width = frame->width, height = frame->height;
    int line_gray[height][width];
    unsigned char *loc = frame->data;
    char grays[9] = " .:-=+*#@";

    // 如果直接按照width*height二位打印，因为太大会溢出，报错segmentation fault
    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            r = loc[(3 * i) + j * frame->linesize];              // red
            g = loc[3 * i + 1 + j * frame->linesize];            // green
            b = loc[3 * i + 2 + j * frame->linesize];            // blue
            line_gray[j][i] = 0.299 * r + 0.587 * g + 0.114 * b; // 0-8
        }
    }

    //假设传入参数窗口window，步长step
    //已知原像素width*hight
    int m,n,max;//用于内层循环
    int width1,height1;
    float num = (float)(window*window);
    //是否能充分扫描
    width1 = (width-window+step)/step;
    height1 = (height-window+step)/step;
    int resized_gray[height1][width1];
    for(i=0;i<height1;i++){
        for(j=0;j<width1;j++){
            max = 0;
            for(n=0;n<window;n++){
                for(m=0;m<window;m++){
                if(line_gray[step*i+n][step*j+m]>max){
                    max = line_gray[step*i+n][step*j+m];
                    } 
                }
            }
            resized_gray[i][j] = (int)((double)max * 8 / 255.0);//0-8
            printf("%c", grays[resized_gray[i][j]]); 
        }
        printf("\n");
    }

    return 0;
}

//彩色图打印
//average pooling
// RGB 图像的池化操作,与灰度图不同，需要保留原始图像的RGB数据
void rgb2color_aver(Frame *frame, int window, int step) {
    unsigned char *data = frame->data;
    int width = frame->width,height = frame->height;
    int pooled_width = (width - window + step) / step;
    int pooled_height = (height - window + step) / step;
    int x,y,idx;
    int i,j,m,n,count;
    int sum_r,sum_g,sum_b,avr_r,avr_g,avr_b;

    // 遍历池化后的每个像素
    for (i = 0; i < pooled_height; i++) {
        for (j = 0; j < pooled_width; j++) {
            sum_r = 0, sum_g = 0, sum_b = 0;
            count = 0;
            // 遍历窗口内的像素并求和
            for (m = 0; m < window; m++) {
                for (n = 0; n < window; n++) {
                    x = i * step + m;
                    y = j * step + n;
                    // 边界检查
                    if (x < height && y < width) {//条件：当前元素在原始数据中
                        idx = x * frame->linesize + y * 3; // 计算RGB值在一维数组中的索引
                        sum_r += data[idx];
                        sum_g += data[idx + 1];
                        sum_b += data[idx + 2];
                        count++;
                    }
                }
            }
            // 计算平均值并打印对应的彩色 ASCII 字符
            avr_r = sum_r / count;
            avr_g = sum_g / count;
            avr_b = sum_b / count;
            printf("\x1b[38;2;%d;%d;%dm\e[48;2m█\x1b[0m", avr_r, avr_g, avr_b);
        }
        printf("\n");
    }
}

//彩色图打印
//max pooling
void rgb2color_max(Frame *frame, int window, int step){
    unsigned char *data = frame->data;
    int width = frame->width,height = frame->height;
    int pooled_width = (width - window + step) / step;
    int pooled_height = (height - window + step) / step;
 //   char color_ascii_chars[] = " .:-=+*#@";

    // 遍历池化后的每个像素
    for (int i = 0; i < pooled_height; i++) {
        for (int j = 0; j < pooled_width; j++) {
            int count = 0;
            int sum = 0,max_num=0;
            // 遍历窗口内的像素,通过比较窗口内元素的灰度值
            //找到灰度值最大的像素，作为当前窗口内最具代表性的颜色值，保存索引
            //根据其打印彩色字符
            for (int m = 0; m < window; m++) {
                for (int n = 0; n < window; n++) {
                    int x = i * step + m;
                    int y = j * step + n;
                    // 边界检查
                    if (x < height && y < width) {//条件：当前元素在原始数据中
                        int idx = x * frame->linesize + y * 3; // 计算RGB值在一维数组中的索引
                        if(data[idx]+data[idx + 1]+data[idx + 2]>sum){
                            max_num = idx;
                        }
                        count++;
                    }
                }
            }
            // 打印对应的彩色 ASCII 字符
            printf("\x1b[38;2;%d;%d;%dm\e[48;2m█\x1b[0m", data[max_num],data[max_num+1],data[max_num+2]);
        }
        printf("\n");
    }
}

