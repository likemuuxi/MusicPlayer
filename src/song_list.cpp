#include "song_list.h"

int get_song_num(void)
{
    int song_num = 0;
    DIR *dir = NULL;
    struct dirent *d = NULL;

    //1. 打开一个目录
    dir = opendir(SONGDIR);
    if (NULL == dir)
    {
        perror("opendir");
        return 0;
    }

    // 2. 计算歌曲数量
    while ((d = readdir(dir)) != NULL)
    {
        // 查找mp3文件
        if(strstr(d->d_name, ".mp3") != NULL)
        {
            song_num++;
        }
    }

    //关闭目录
    closedir(dir);

    return song_num;
}




char **get_song_list(void)
{
    int i = 0;
    int song_num = 0;
    char **song_list = NULL;
    DIR *dir = NULL;
    struct dirent *d = NULL;

    //1. 打开一个目录
    dir = opendir(SONGDIR);
    if (NULL == dir)
    {
        perror("opendir");
        return NULL;
    }

    // 2. 计算歌曲数量
    song_num = get_song_num();

    // 3. 分配足够的内存来存储歌曲名
    song_list = (char **)malloc(sizeof(char *) * song_num);

    if (song_list == NULL)
    {
        perror("malloc");
        return NULL;
    }

    // 4. 读取歌曲名称到分配的内存中
    while ((d = readdir(dir)) != NULL)
    {
        if(strstr(d->d_name, ".mp3") != NULL)
        {
            song_list[i] = (char *)malloc(strlen(d->d_name) + 1);
            if(NULL == song_list[i])
            {
                perror("malloc");
                return NULL;
            }
            memcpy(song_list[i], d->d_name, strlen(d->d_name) + 1);
            i++;
        }
    }
// 调试
//    printf("song_num: %d\n", song_num);
//    for(int j = 0; j < i; j++)
//    {
//        printf("song: %s\n", song_list[j]);
//    }

    //关闭目录
    closedir(dir);

    return song_list;
}


//释放song_list空间
void free_song_list(char **song_list)
{
    int i;
    int song_num = 0;
    song_num = get_song_num();

    for(i = 0; i < song_num; i++)
    {
        free(song_list[i]);
    }
    free(song_list);
}



