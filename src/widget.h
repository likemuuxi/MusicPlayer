#ifndef WIDGET_H
#define WIDGET_H

#include <QDebug>
#include <QWidget>
#include <QLabel>
#include <QString>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QListWidget>
#include <QPushButton>
#include <QPixmap>
#include <QPalette>
#include <stdio.h>
#include <unistd.h>
#include "song_list.h"
#include "lyric.h"
#include "dlist.h"
#include <QCloseEvent>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <QTimer>
#include <pthread.h>
#include <QColor>
#include <QIcon>
#include <QMouseEvent>
#include <QSpacerItem>
#include <time.h>

#define FIFO_NAME "myfifo"
#define SONG_DIR "/home/muuxi/project/music_player/my_music_player/song/"
#define LYRIC_DIR "/home/muuxi/project/music_player/my_music_player/lyrics/"

//图标区域
#define WALLPAPER "/home/muuxi/project/music_player/my_music_player/image/6.jpg"

#define MUSIC_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--music-2-fill.svg"
#define MAIN_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--folder-music-fill.svg"

#define PLAY_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--player-pause-fill.svg"
#define PAUSE_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--play-circle-fill.svg"
#define BACK_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--player-start-fill.svg"
#define NEXT_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--player-end-fill.svg"

#define VOLUME_MAX_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--volume-up-fill.svg"
#define VOLUME_MID_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--volume-down-fill.svg"
#define VOLUME_MUTE_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--volume-off-fill.svg"


#define PLATLIST_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--playlist-fill.svg"
#define RANDOM_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--playlist-shuffle-fill.svg"
#define REPEAT_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--playlist-repeat-list-fill.svg"
#define SINGLE_ICON "/home/muuxi/project/music_player/my_music_player/image/iconamoon--playlist-repeat-song-fill.svg"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(int pipe_fd_0, QWidget *parent = nullptr);
    ~Widget();

    void set_window(void); //设置布局
    void set_song_msg(int num); //歌词区域
    void song_player(int num); //歌曲播放
    void song_stop(void); //停止播放
    void lrc_lyric_print(node_t *start, node_t *cur);//打印链表中5个节点的信息到屏幕上
    void handleCellClicked(QListWidgetItem *item); //播放列表点击事件

    void start_pause(void); //开始暂停，切歌
    void to_next_song(void); //点击下一首的槽函数
    void to_last_song(void); //点击上一首的槽函数
    void set_volume(int value); //音量调整
    void volumeClicked(void); //点击音量图标
    void set_progress(int value); //歌曲进度调整
    void mouse_progress(QMouseEvent *ev); //鼠标点击进度条
    void set_time(void); //设置时间标签
    void updateSlider(void); //进度条自走
    void playlistClicked(void); //显隐藏播放列表
    void randomClicked(void); //随机播放
    void repeatClicked(void); //重复播放/单曲循环

    //静态成员变量
    static int flags;
    static int currentPosition;

private:
    QVBoxLayout *layout; //主要区域布局
    QHBoxLayout *layout_player; //播放区域布局
    QVBoxLayout *layout_list; //歌曲列表布局
    QLabel *label_list; //歌曲列表标题
    QListWidget *list_song; //歌曲表格
    QVBoxLayout *layout_song; //歌词显示布局
    QVBoxLayout *layout_msg; //歌曲信息布局
    QListWidget *listwidget_lrc; //歌词显示表格


    QHBoxLayout *layout_control; //控制区域布局
    QPushButton *last_song; //上一首
    QPushButton *pause_continue; //暂停继续
    QPushButton *next_song; //下一首
    QPushButton *playlist; //播放列表
    QPushButton *random; //随机播放
    QPushButton *repeat; //重复/单曲播放

    QHBoxLayout *layout_volume; //音量条布局
    QPushButton *volume; //重复/单曲播放
    QSlider *volume_slide; //音量条

    QVBoxLayout *layout_progress; //进度条布局
    QHBoxLayout *layout_time; //时间标签布局
    QSlider *progressBar; //进度条

    QLabel *label_songname; //歌名
    QLabel *label_singer; //歌手
    QLabel *label_album; //专辑

    QLabel *main_image; //歌词界面显示图案

    QTimer *timer; //定时器

    QLabel *label_real_time; //歌曲实时时间
    QLabel *label_total_time; //歌曲总时间

    QSpacerItem *spacer; //添加间隔


signals:
    void get_time(void);
};
#endif // WIDGET_H
