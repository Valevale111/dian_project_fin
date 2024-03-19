//最大池化和平均池化函数
#include <stdio.h>
#include "video_decoder.h"
#include "resize.h"

int resize_max(Frame *Frame,int x,int y);//最大池化的分辨率调整函数
int resize_aver(Frame *Frame,int x,int y);//平均池化的分辨率调整函数

//要求可以传入窗口大小和步长，自由调整降采样帧的大小（分辨度）
//注意：如果x!+y,可能出现重叠区域
//另外：如果当前输入参数不能充分采样，如何处理

//最大池化的分辨率调整函数
int resize_max(Frame *Frame,int x,int y){
    

    return 0;
}   


//平均池化的分辨率调整函数
int resize_aver(Frame *Frame,int x,int y){

    return 0;
}