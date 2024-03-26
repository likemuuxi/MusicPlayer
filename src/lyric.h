#ifndef __LRC_H__
#define __LRC_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dlist.h"


//时间相关结构体类型
typedef struct _lrc_time_t{
    //分钟
    unsigned short min;
    //秒
    unsigned short sec;
    //毫秒
    unsigned short msec;
}lrc_time_t;

//歌词节点信息
typedef struct _lrc_node_t{
    int time;
    char lrcbuf[200];
}lrc_node_t;



//加载歌词文件
char *lrc_read_file(const char *filename);

int get_song_lines(const char *filename);

//使用\r\n切割歌词
int lrc_strtok_buf(char *lrc_text[], char *ptext);

//解析lrc文件前4行 歌手 歌名 专辑等等
int lrc_parse_head(char *lrc_line_text[], char *song_msg[]);

//获取歌曲信息 前4行
int lrc_song_head_print(char *song_msg[]);

//释放song_head内存
int lrc_song_head_free(char *song_msg[], int n);


//显示歌词的时间
int lrc_time_print(lrc_time_t *t);

//解析歌词信息 歌词和时间
int lrc_lyric_parse(dlist_t *d, int lrc_lines, char *lrc_text[]);


//将秒数转化为分和秒 lrc_time_t类型
int lrc_make_time(lrc_time_t *t);

#endif /*__LRC_H__*/
