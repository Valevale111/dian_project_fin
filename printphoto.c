//level 2-1
//example
// export const getAscii = (data: Uint8ClampedArray, index: number) => {
//   const r = data[index];
//   const g = data[index + 1];
//   const b = data[index + 2];
//   const a = data[index + 3];

//   const i = (0.2126 * r + 0.7152 * g + 0.0722 * b);

//   return a !== 0 ? pixel[Math.floor(i / weight)] : pixel[0];
// };
//video_decoder.h
//double get_fps();  //传入什么参数？？
// int get_frame_index();
// int get_total_frames();

#include <stdio.h>
#include <malloc.h>
#include "video_decoder.h"//优先在当前目录搜索

int main(int argc,char *argv[])
{
    decoder_init(NULL);
    char *path=NULL;
    path = argv[1]; //获得视频地址
    double current_fps;
    int current_index,all_frames;
    Frame photo;
    photo.linesize = 0;
    photo.data = (char*)malloc(sizeof(char)*100);//初始化一维指针
    if(decoder_init(path)==-1)//如果初始化成功
    {
        photo = decoder_get_frame(path); 
        if(photo.linesize)
        {
            printf("current line size = %d.\n",photo.linesize);
        }
    }
    else 
        printf("Initialize failed.\n");
    decoder_close();

    free(photo.data);
    return 0;
}
