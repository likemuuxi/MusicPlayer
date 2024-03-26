#ifndef SONG_LIST_H
#define SONG_LIST_H

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <fcntl.h>



#define SONGDIR "/home/muuxi/project/music_player/my_music_player/song"


int get_song_num(void);
char **get_song_list(void);
void free_song_list(char **song_list);

#endif // SONG_LIST_H
