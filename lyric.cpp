#include "lyric.h"



// 加载歌词文件
char *lrc_read_file(const char *filename)
{
    int ret = 0;
    FILE *fp = NULL;
    char *ptext = NULL;
    unsigned long FILE_SIZE = 0;

    // 参数检查
    if (NULL == filename)
    {
        printf("lrc_read_file NULL..\n");
        goto err0;
    }

    // 打开文件
    fp = fopen(filename, "r");
    if(NULL == fp)
    {
        perror("fopen");
        goto err0;
    }
    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    FILE_SIZE = ftell(fp);

    // 分配空间
    ptext = (char *)malloc(FILE_SIZE + 1);
    if (NULL == ptext)
    {
        printf("lrc_read_file malloc failed..\n");
        goto err1;
    }
    memset(ptext, 0, FILE_SIZE + 1);
    // 定位到文件开头
    rewind(fp);
    // 读取文件
    ret = fread(ptext, FILE_SIZE, 1, fp);
    if (ret == 0)
    {
        printf("fread failed..\n");
        goto err1;
    }

    fclose(fp);
    return ptext;

err1:
    fclose(fp);
err0:
    return NULL;
}



// 使用\r\n切割歌词 返回歌词的行数
int lrc_strtok_buf(char *lrc_text[], char *ptext)
{
    int i = 0;
    //第一次切
    lrc_text[i] = strtok(ptext, "\r\n");
    //后续切
    while (NULL != lrc_text[i])
    {
        i++;
        lrc_text[i] = strtok(NULL, "\r\n");
    }

    return i;
}

int get_song_lines(const char *filename)
{

    int lines = 0;
    char *lyric_text = NULL;
    char *lrc_line_text[200] = {NULL};

    lyric_text = lrc_read_file(filename);
    lines = lrc_strtok_buf(lrc_line_text, lyric_text);

    return lines;
}


// 解析lrc文件前3行 歌手 歌名 专辑
//[ti:简单爱]
//[ar:周杰伦]
//[al:范特西]
//考虑有的字段没有内容，空间动态分配
int lrc_parse_head(char *lrc_line_text[], char *song_msg[])
{
    int i = 0;
    //分隔符
    char *tok[] = {"[ti", "[ar", "[al", ""};

    for (i = 0; i < 4; i++)
    {
        if (strncmp(tok[i], lrc_line_text[i], 3) == 0)
        {
            song_msg[i] = (char *)malloc(strlen(lrc_line_text[i]) - 4);
            if (NULL == song_msg[i])
            {
                printf("lrc_parse_head malloc failed...\n");
                break;
            }
            memset(song_msg[i], 0, strlen(lrc_line_text[i]) - 4);
            sscanf(lrc_line_text[i], "%*4s%[^]]", song_msg[i]);
        }
        else
        {
            //不存在该字段
            song_msg[i] = NULL;
        }
    }

    return 0;
}


// 释放song_head内存
int lrc_song_head_free(char *song_msg[], int n)
{

    for (int i = 0; i < n; i++)
    {
        if (NULL != song_msg[i])
        {
            free(song_msg[i]);
            //习惯性指空，可防止野指针  预防一些问题
            song_msg[i] = NULL;
        }

    }

    return 0;
}


//歌词时间
int lrc_time_print(lrc_time_t *t)
{
    char str[32];

    memset(str, 0, 32);
    sprintf(str, "%02d:%02d.%02d", t->min, t->sec, t->msec);
    printf("%s\n", str);

    return 0;
}

// 解析歌词信息 歌词和时间
int lrc_lyric_parse(dlist_t *d, int lrc_lines, char *lrc_text[])
{
    //参数检查

    lrc_node_t node;
    int min = 0;
    int sec = 0;
    int msec = 0;

    int time_array[10] = {0};
    //同一句歌词重复次数
    int time_counts = 0;

    //跳过前 3 行
    for (int i = 3; i < lrc_lines; i++)
    {
        time_counts = 0;
        //不知道时间戳数量
        while ('[' == *lrc_text[i])
        {
            sscanf(lrc_text[i], "[%2d:%2d.%2d", &min, &sec, &msec);
            //转换为秒进行存储
            time_array[time_counts++] = min * 60 + sec + msec / 50;
            //每个时间戳相隔10个字符
            lrc_text[i] = lrc_text[i] + 10;
        }
        //解析完一行所有的时间后，src就为歌词的首地址
        //根据歌词的时间个数, 将歌词和时间保存到链表中
        for (int j = 0; j < time_counts; j++)
        {
            memset(&node, 0, sizeof(node));
            node.time = time_array[j];
            strcpy(node.lrcbuf, lrc_text[i]);

            dlist_append(d, &node);
        }
    }

    return 0;
}


// 时间更新规则   将秒数转化为分和秒 lrc_time_t类型
int lrc_make_time(lrc_time_t *t)
{
    //参数检查
    if (NULL == t)
    {
        printf("lrc_make_time NULL..\n");
        goto err0;
    }
    //转化
    t->sec++;
    if (60 == t->sec)
    {
        t->sec = 0;
        t->min++;

        if (60 == t->min)
        {
            t->min = 0;
        }
    }

    return 0;
err0:
    return 1;
}
