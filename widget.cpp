#include "widget.h"

int Widget::flags = 2; //标志位，0开始 1暂停 2初始化
int Widget::currentPosition = 0; //进度条初始值
int repeat_singel = 0; //重复和单曲切换标志 0重复 1单曲 默认列表播完结束
int change_mute = 0; //点击音量图标切换静音
int change_playlist = 0; //切换播放列表
int change_random = 0; //切换随机播放状态

int song_num; //歌曲的个数
int num = 0; //歌曲的位置
int lry_node = 0; //歌词节点数

char **song_list; //歌曲列表

int *arr;  //保存随机后的数组
int random_i = 0; //标记随机数组序号

int mplayerVolume = 100; //音量
int last_volume_value;

float total_time = 0.0f; //歌曲总时间
float real_time = 0.0f; //歌曲播放当前时间

struct RECV_ARG{
    int a;
    Widget *b;
};

int my_cmp(void *data, void *key)
{
    lrc_node_t *t = (lrc_node_t *)data;

    return t->time - *(int*)key;
}

//Fisher-Yates洗牌
void fisherYates(int n) {
    srand(time(NULL));
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

//打印链表中5个节点的信息到屏幕上
void Widget::lrc_lyric_print(node_t *start, node_t *cur)
{
    listwidget_lrc->clear();

    lrc_node_t *lrc_node = NULL;

    for (int i = 0; i < 5; i++)
    {
        lrc_node = (lrc_node_t*)start->data;

        if(start == cur)
        {

            listwidget_lrc->addItem(lrc_node->lrcbuf);
            // 设置播放行高亮
            listwidget_lrc->item(i)->setTextColor(QColor(255, 76, 0));

        }
        else
        {
            listwidget_lrc->addItem(lrc_node->lrcbuf);
            // 设置每行的内容
            //listwidget_lrc->item(i)->setText(QString("%1").arg(lrc_node->lrcbuf));
        }

        //指向链表下一个节点
        start = start->next;
    }

}


//歌词信息
void Widget::set_song_msg(int num)
{
    int lines = 0;
    char buf[256];
    char lyric_dir[128];
    char song_name[32];
    strcpy(song_name, strtok(song_list[num], "."));
    sprintf(lyric_dir, "%s%s%s", LYRIC_DIR, song_name, ".lrc");

    dlist_t *d = NULL;
    d = dlist_create(sizeof(lrc_node_t)); //链表

    char *lyric_text = NULL; //歌词
    char** song_msg = (char **)malloc(4 * sizeof(char*));//歌曲头信息
    char *lrc_line_text[200] = {NULL}; //单行歌词

    lyric_text = lrc_read_file(lyric_dir);


    //切割歌词 \r\n分隔
    lines = lrc_strtok_buf(lrc_line_text, lyric_text);

    //解析歌词信息 前4行
    lrc_parse_head(lrc_line_text, song_msg);

    sprintf(buf, "歌曲：%s", song_msg[0]);
    label_songname->setText(buf);
    label_songname->setAlignment(Qt::AlignCenter);
    label_songname->setFixedSize(600, 20);

    sprintf(buf, "歌手：%s", song_msg[1]);
    label_singer->setText(buf);
    label_singer->setAlignment(Qt::AlignCenter);
    label_singer->setFixedSize(600, 20);

    sprintf(buf, "专辑：%s", song_msg[2]);
    label_album->setText(buf);
    label_album->setAlignment(Qt::AlignCenter);
    label_album->setFixedSize(600, 20);

    //释放歌词头空间
    lrc_song_head_free(song_msg, 4);


    //-----------设置歌词----------------
    lrc_lyric_parse(d, lines, lrc_line_text); //歌词解析
    dlist_select_sort(d, my_cmp); //排序歌词

    node_t *start = NULL;
    node_t *cur = NULL;

    //qDebug() << lines << d->count << endl;

    if(lry_node == 0)
    {
        //显示歌词前五行
        start = d->p;
        lrc_lyric_print(start, cur);
    }

    //通过时间查找链表节点
    start = dlist_find_by_key(d, &currentPosition, my_cmp);
    //qDebug() << lry_node << endl;

    //更新歌词节点序号信息
    lry_node = node_find_by_key(d, &currentPosition, my_cmp);
    //qDebug() << lry_node << endl;

    if (NULL != start)
    {
        lry_node++;

        //前面3行 不滚动显示
        if (lry_node <= 3)
        {
            lrc_lyric_print(d->p, start);
        }
        else if (lry_node >= d->count - 5) //最后5行 不滚动显示
        {
            listwidget_lrc->clear();
            //d->p->prev->prev->prev->prev->prev 表示倒数第五个节点
            lrc_lyric_print(d->p->prev->prev->prev->prev->prev, start);
        }
        else
        {
            listwidget_lrc->clear();
            //start->prev->prev表示当前节点前面两个节点
            lrc_lyric_print(start->prev->prev, start);
        }

        //结束循环
        //qDebug() << lry_node << " " << d->count << endl;

        if (lry_node == d->count)
        {
            qDebug() << "歌词显示完毕" << endl;
        }
    }

    int count = listwidget_lrc->count(); // 获取列表项的数量
    for(int i = 0; i < count; i++) {
        QListWidgetItem *item = listwidget_lrc->item(i); // 获取每一项
        item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter); // 设置文本对齐方式
    }


    //释放链表
    dlist_destroy(d);
}


void Widget::set_window(void)
{
    //控件主布局
    layout = new QVBoxLayout();
    this->setLayout(layout);
    //水平布局 播放列表 + 歌词
    layout_player = new QHBoxLayout();

    //-------------播放列表区域------------------
    //layout_list 标题 + 歌曲
    layout_list = new QVBoxLayout();
    label_list = new QLabel;
    label_list->setText("歌曲列表");
    label_list->setAlignment(Qt::AlignCenter);
    layout_list->addWidget(label_list);
    //歌曲列表
    list_song = new QListWidget();
    list_song->setMaximumWidth(220);
    list_song->setMinimumSize(200, 100);
    list_song->setStyleSheet("QListWidget {"
                             "background-color: lightgray;"
                             "border-radius: 10px;"
                             "border-width: 0px;"
                             "padding: 10px;"
                             "}"
                             "QListWidget::item {"
                             "color: white;"  // 设置字体颜色
                             "background-color: darkgray;"  // 设置条目背景色
                             "border-radius: 10px;"
                             "padding: 5px;"  // 设置条目内边距
                             "margin: 3px;"  // 设置条目之间的间距
                             "border:none;"
                             "border-width: 0px;"
                             "}"
                             "QListWidget::item:selected {"
                             "background-color: #ee8869;" // 设置选中条目的背景色
                             "}"
                             "QListWidget:focus{"
                                 "outline: 0px;"
                             "}");

    layout_list->addWidget(list_song);

    // 添加歌曲
    for (int i = 0; i < song_num; i++)
    {
        list_song->addItem(song_list[i]);
    }
    //统一表格条目高度
    for(int i = 0; i < list_song->count(); ++i)
    {
        QListWidgetItem* item = list_song->item(i);
        item->setSizeHint(QSize(item->sizeHint().width(), 40));  // 50 是你想要设置的高度
    }


    //------------歌词显示区域------------------
    layout_song = new QVBoxLayout();

    // 添加一个弹簧控件
    QSpacerItem* verticalSpacer1 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout_song->addSpacerItem(verticalSpacer1);

    // 创建字体对象，并设置字体大小
    QFont font;
    font.setPointSize(13);  // 设置字体大小

    label_songname = new QLabel;
    label_songname->setFont(font);
    label_singer = new QLabel;
    label_singer->setFont(font);
    label_album = new QLabel;
    label_album->setFont(font);

    main_image = new QLabel;
    QPixmap main_icon(MAIN_ICON);
    main_image->setPixmap(main_icon);
    main_image->setAlignment(Qt::AlignCenter);


    layout_msg = new QVBoxLayout();
    layout_msg->addWidget(label_songname);
    layout_msg->addWidget(label_singer);
    layout_msg->addWidget(label_album);
    layout_msg->setAlignment(Qt::AlignCenter);

    layout_song->addLayout(layout_msg);
    spacer = new QSpacerItem(80, 80);
    layout_song->addSpacerItem(spacer);

    layout_song->addWidget(main_image);

    // 添加一个弹簧控件
    QSpacerItem* verticalSpacer2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout_song->addSpacerItem(verticalSpacer2);

    //歌词列表
    listwidget_lrc = new QListWidget();

    listwidget_lrc->setFrameShape(QListWidget::NoFrame); //设置边框透明
    listwidget_lrc->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //隐藏滚动条
    listwidget_lrc->setSpacing(5); //增大间距
    listwidget_lrc->setFont(font); //设置字体
    listwidget_lrc->setStyleSheet("QListWidget {"
                                 "background-color: transparent;"
                                 "}"
                                 "QListWidget::item {"
                                 "background-color: transparent;"
                                 "margin: 2px;"  // 设置条目之间的间距
                                 "border:none;"
                                 "border-width: 0px;"
                                 "}"
                                 "QListWidget::item:selected {"
                                 "color: black;"
                                 "background-color: transparent;" // 设置选中条目的背景色
                                 "}"
                                 "QListWidget:focus{"
                                     "outline: 0px;"
                                 "}");



    layout_song->addWidget(listwidget_lrc);
    // 添加一个弹簧控件
    QSpacerItem* verticalSpacer3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout_song->addSpacerItem(verticalSpacer3);

    //------------控制按钮------------------
    QHBoxLayout *layout_control = new QHBoxLayout();
    //上一首
    last_song = new QPushButton();
    last_song->setStyleSheet("QPushButton { color: white; border-radius: 5px; } QPushButton:hover { background-color: #b5ded2; color: #000000; }");
    last_song->setFixedWidth(80);
    QPixmap back_icon(BACK_ICON);
    last_song->setIconSize(QSize(48,48));
    last_song->setIcon(back_icon);
    //播放暂停
    pause_continue = new QPushButton();
    pause_continue->setStyleSheet("QPushButton { color: white; border-radius: 5px; } QPushButton:hover { background-color: #b5ded2; color: #000000; }");
    pause_continue->setFixedWidth(80);
    QPixmap pause_icon(PAUSE_ICON);
    pause_continue->setIconSize(QSize(48,48));
    pause_continue->setIcon(pause_icon);
    //下一首
    next_song = new QPushButton();
    next_song->setStyleSheet("QPushButton { color: white; border-radius: 5px; } QPushButton:hover { background-color: #b5ded2; color: #000000; }");
    next_song->setFixedWidth(80);
    QPixmap next_icon(NEXT_ICON);
    next_song->setIconSize(QSize(48,48));
    next_song->setIcon(next_icon);


    layout_control->addWidget(last_song, 0, Qt::AlignCenter);
    layout_control->addWidget(pause_continue, 0, Qt::AlignCenter);
    layout_control->addWidget(next_song, 0, Qt::AlignCenter);

    layout_song->addLayout(layout_control);

    layout_player->addLayout(layout_list);
    layout_player->addLayout(layout_song);
    layout->addLayout(layout_player);


    //------------进度条----------------
    layout_progress = new QVBoxLayout();
    layout_time = new QHBoxLayout();
    label_real_time = new QLabel; //实时时间
    label_real_time->setAlignment(Qt::AlignLeft);
    label_total_time = new QLabel; //总时间
    label_total_time->setAlignment(Qt::AlignRight);

    progressBar = new QSlider(Qt::Horizontal);//进度条

    layout_time->addWidget(label_real_time);
    layout_time->addWidget(label_total_time);
    layout_progress->addLayout(layout_time);
    layout_progress->addWidget(progressBar);

    layout->addLayout(layout_progress);

    //播放列表图标
    playlist = new QPushButton();
    playlist->setStyleSheet("QPushButton { color: white; border-radius: 5px; } QPushButton:hover { background-color: #b5ded2; color: #000000; }");
    playlist->resize(24,24);
    QPixmap list_icon(PLATLIST_ICON);
    playlist->setIconSize(QSize(24,24));
    playlist->setIcon(list_icon);

    //随机图标
    random = new QPushButton();
    random->setStyleSheet("QPushButton { color: white; border-radius: 5px; } QPushButton:hover { background-color: #b5ded2; color: #000000; }");
    random->resize(24,24);
    QPixmap random_icon(RANDOM_ICON);
    random->setIconSize(QSize(24,24));
    random->setIcon(random_icon);

    //重复播放
    repeat = new QPushButton();
    repeat->setStyleSheet("QPushButton { color: white; border-radius: 5px; } QPushButton:hover { background-color: #b5ded2; color: #000000; }");
    repeat->resize(24,24);
    QPixmap repeat_icon(REPEAT_ICON);
    repeat->setIconSize(QSize(24,24));
    repeat->setIcon(repeat_icon);

    //------------音量条----------------
    layout_volume = new QHBoxLayout();
    volume_slide = new QSlider(Qt::Horizontal); //音量条
    volume_slide->setMaximumWidth(140);
    volume_slide->setMaximum(100); //设置最大值
    volume_slide->setMinimum(0); //设置最小值
    volume_slide->setValue(100); //设置初始值

    //音量图标
    volume = new QPushButton();
    volume->setStyleSheet("QPushButton { color: white; border-radius: 5px; } QPushButton:hover { background-color: #b5ded2; color: #000000; }");
    volume->resize(24,24);
    QPixmap vloume_icon(VOLUME_MAX_ICON);
    volume->setIconSize(QSize(24,24));
    volume->setIcon(vloume_icon);


    layout_volume->addWidget(playlist);
    layout_volume->setSpacing(20);
    layout_volume->addWidget(random);
    layout_volume->setSpacing(20);
    layout_volume->addWidget(repeat);
    layout_volume->addStretch(0);
    layout_volume->addWidget(volume);
    layout_volume->addWidget(volume_slide);

    layout->addLayout(layout_volume);

    //定时器
    timer = new QTimer(this);

}


/*******************歌曲进度调整******************/
void Widget::set_progress(int value)
{
    int mplayerProgress = value;

    currentPosition = value;

    // 打开有名管道
    int fd = open(FIFO_NAME, O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        return;
    }

    // 构建 mplayer 命令
    char command[32];
    snprintf(command, sizeof(command), "seek %d 2\n", mplayerProgress);

    // 将命令写入管道
    if (write(fd, command, strlen(command)) < 0)
    {
        perror("write");
        return;
    }

    set_song_msg(num);

    QPixmap pause_icon(PLAY_ICON);
    pause_continue->setIcon(pause_icon);

    Widget::flags = 0;

    ::close(fd);
}


/*******************音量调整******************/
void Widget::set_volume(int value)
{
    mplayerVolume = value;

    // 打开有名管道
    int fd = open(FIFO_NAME, O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        return;
    }

    // 构建mplayer命令
    char command[32];
    snprintf(command, sizeof(command), "volume %d 1\n", mplayerVolume);

    // 将命令写入管道
    if (write(fd, command, strlen(command)) < 0)
    {
        perror("write");
        return;
    }
    ::close(fd);

    if((mplayerVolume > 50) && (mplayerVolume <= 100))
    {
        QPixmap volume_icon(VOLUME_MAX_ICON);
        volume->setIconSize(QSize(24,24));
        volume->setIcon(volume_icon);
    }
    else if((mplayerVolume > 0) && (mplayerVolume <= 50))
    {
        QPixmap volume_icon(VOLUME_MID_ICON);
        volume->setIconSize(QSize(24,24));
        volume->setIcon(volume_icon);
    }
    else if(mplayerVolume == 0)
    {
        QPixmap volume_icon(VOLUME_MUTE_ICON);
        volume->setIconSize(QSize(24,24));
        volume->setIcon(volume_icon);
    }


}

/*******************开始暂停处理******************/
void Widget::start_pause(void)
{
    if(Widget::flags == 2)
    {
        //初始化点击，播放列表中第一个歌曲，并切换图标

        QPixmap pause_icon(PLAY_ICON);
        pause_continue->setIcon(pause_icon);

        song_player(num);

        Widget::flags = 0;
    }
    else if(Widget::flags == 0)
    {
        QPixmap pause_icon(PAUSE_ICON);
        pause_continue->setIcon(pause_icon);
        Widget::flags = 1;

        timer->stop();
    }
    else
    {
        QPixmap pause_icon(PLAY_ICON);
        pause_continue->setIcon(pause_icon);
        Widget::flags = 0;

        timer->start(1000);
    }

    //歌曲继续
    int fd = open(FIFO_NAME, O_WRONLY);
    if (fd < 0) {
        perror("open");
        return;
    }
    if(write(fd, "pause\n", strlen("pause\n")) < 0)
    {
        printf("write error");
        return ;
    }
    ::close(fd);
}

void Widget::song_stop(void)
{
    num = 0;
    song_player(num);
    start_pause();
    listwidget_lrc->clear();
    label_songname->clear();
    label_singer->clear();
    label_album->clear();
    Widget::flags = 2;
}

/*******************切到下一首******************/
void Widget::to_next_song(void)
{
    if(Widget::flags == 2)
    {
        num = 0; //初始化第一首歌
        return;
    }
    else
    {
        if(repeat_singel == 0) //列表播放模式
        {
            if(change_random == 1) //随机模式
            {
                ++random_i;
                //qDebug() << num << " " << random_i  << " " << song_num << endl;
                if(random_i <= song_num) //列表所有歌随机遍历完成
                {
                    num = arr[random_i - 1];
                    //qDebug() << "下一首：" << num << endl;
                }
                else
                {
                    random_i = 0;
                    song_stop();
                    return ;
                }
                song_player(num);
            }
            else
            {
                num++;
                if(num < song_num) //正常切歌
                {
                    song_player(num);
                }
                else //到达列表末尾，停止播放
                {
                    song_stop();
                    return ;
                }
            }
        }
        else if(repeat_singel == 1) //重复模式
        {
            if(change_random == 1) //随机模式
            {
                ++random_i;
                qDebug() << num << " " << random_i  << " " << song_num << endl;
                if(random_i == song_num)
                {
                    random_i = 0;
                }

                num = arr[random_i];
                qDebug() << "下一首：" << num << endl;

                song_player(num);
            }
            else
            {
                num++;
                //到末尾重置到开头
                if(num == song_num)
                {
                    num = 0;
                }

                song_player(num);
            }
        }
        else if(repeat_singel == 2) //单曲循环模式
        {
            if(change_random == 1) //随机模式
            {
                num = arr[random_i];

                if(random_i == song_num) //遍历随机列表所有歌
                {
                    random_i = 0; //从头再次遍历
                }
                else
                {
                    random_i++;
                }

                song_player(num);
            }
            else
            {
                song_player(num);
            }
        }
    }
}

/*******************切到上一首******************/
void Widget::to_last_song(void)
{
    if(Widget::flags == 2)
    {
        num = 0; //初始化第一首歌
        return;
    }
    else
    {
        if(repeat_singel == 0) //列表播放模式
        {
            if(change_random == 1) //随机模式
            {
                if(random_i == 0)
                {
                    random_i = song_num + 1;
                }
                --random_i;
                //qDebug() << num << " " << random_i  << " " << song_num << endl;
                if(random_i > 0) //遍历随机列表所有歌
                {
                    num = arr[random_i - 1];
                    //qDebug() << "上一首：" << num << endl;
                }
                else
                {
                    random_i = 0;
                    song_stop();
                    return ;
                }
                song_player(num);
            }
            else
            {
                num--;
                if(num >= 0) //正常切歌
                {
                    song_player(num);
                }
                else if(num == -1) //到达列表首部，停止播放
                {
                    song_stop();
                }
            }
        }
        else if(repeat_singel == 1) //重复模式
        {
            if(change_random == 1) //随机模式
            {
                if(random_i == 0)
                {
                    random_i = song_num;
                }
                --random_i;
                qDebug() << num << " " << random_i  << " " << song_num << endl;

                num = arr[random_i];
                qDebug() << "上一首：" << num << endl;

                song_player(num);
            }
            else
            {
                num--;
                //到首部重置到末尾
                if(num == -1)
                {
                    num = song_num - 1;
                }
                song_player(num);
            }
        }
        else if(repeat_singel == 2) //单曲循环模式
        {
            if(change_random == 1) //随机模式
            {
                num = arr[random_i];

                if(random_i == song_num) //列表所有歌随机遍历完成
                {
                    random_i = 0; //从头再次遍历
                }
                else
                {
                    random_i++;
                }

                song_player(num);
            }
            else
            {
                song_player(num);
            }
        }
    }
}


/*******************双击播放列表******************/
void Widget::handleCellClicked(QListWidgetItem *item)
{
    //改变歌曲编号
    num = list_song->row(item);
    song_player(num);
}

/*******************歌曲播放******************/
void Widget::song_player(int num)
{
    char lyric_dir[128];
    char song_dir[128];
    char song_name[32];
    strcpy(song_name, strtok(song_list[num], "."));
    sprintf(song_dir, "%s%s%s", SONG_DIR, song_name, ".mp3");
    sprintf(lyric_dir, "%s%s%s", LYRIC_DIR, song_name, ".lrc");

    int fd;
    char command[128];
    lry_node = 0;

    //切换播放图标
    QPixmap pause_icon(PLAY_ICON);
    pause_continue->setIcon(pause_icon);

    //取消显示歌词显示界面的图标
    if(Widget::flags == 2)
    {
        if(main_image != NULL)
        {
            delete main_image;
            main_image = NULL;

            layout_song->removeItem(spacer);
            delete spacer;
        }
    }

    //进度条复位
    currentPosition = 0;
    progressBar->setValue(currentPosition);

    //歌曲时间复位
    label_real_time->setText(QString("00:00"));

    mkfifo(FIFO_NAME, 0664);

    if((fd = open(FIFO_NAME, O_WRONLY)) == -1)
    {
        perror("fail to open");
    }

    //加载歌曲
    sprintf(command, "loadfile %s\n", song_dir);
    if(write(fd, command, strlen(command)) == -1)
    {
        perror("write");
    }

    if(Widget::flags == 2)
    {
        //设置音量条对应音量， 防止维持上次程序运行的音量
        mplayerVolume = volume_slide->value();
        snprintf(command, sizeof(command), "volume %d 1\n", mplayerVolume);

        if (write(fd, command, strlen(command)) < 0)
        {
            perror("write");
            return;
        }
    }

    set_song_msg(num);

    list_song->setItemSelected(list_song->item(num), true);  // 将该行设置为选中

    timer->start(1000); // 设置定时器间隔，单位为毫秒

    Widget::flags = 0;

    ::close(fd);
}


void *send_cmd(void *arg)
{
    mkfifo(FIFO_NAME,0664);
    int fd;
    if((fd = open(FIFO_NAME, O_WRONLY)) == -1)
    {
        perror("fail to open");
    }
    while(1)
    {
        usleep(500000);
        if(Widget::flags == 0)
        {
            if(write(fd, "get_time_length\n", strlen("get_time_length\n")) == -1)
            {
                perror("write");
            }
        }

    }
    ::close(fd);
}


void *recv_msg(void *arg)
{
    RECV_ARG *recv = (RECV_ARG *)arg;
    Widget *p = recv->b;
    int pipe_fd_0 = recv->a;
    while(1)
    {
        char buf[64];
        if(read(pipe_fd_0, buf, sizeof(buf)) == -1)
        {
            perror("fail to read");
        }
        if(strncmp(buf, "ANS_LENGTH", strlen("ANS_LENGTH")) == 0)
        {
            float ttime = 0.0f;
            sscanf(buf, "ANS_LENGTH=%f", &ttime);
            total_time = ttime;
            emit p->get_time();
        }
    }
}


void Widget::set_time(void)
{
    progressBar->setMaximum(total_time);

    char time_2[8];
    sprintf(time_2, "%02d:%02d", (int)total_time / 60, (int)total_time % 60);
    label_total_time->setText(QString(time_2));

    progressBar->setMaximum(total_time);
}

/*******************更新进度条******************/
void Widget::updateSlider(void)
{
    if (currentPosition < total_time)
    {
        currentPosition++;

        char time[8];
        sprintf(time, "%02d:%02d", currentPosition / 60, currentPosition % 60);

        label_real_time->setText(QString(time));

        progressBar->setValue(currentPosition);
    }
    else
    {
        if(repeat_singel == 2) //单曲循环
        {
            song_player(num);
        }
        else
        {
            to_next_song();
        }
    }
}

/*******************播放列表图标******************/
void Widget::playlistClicked(void)
{

    if(change_playlist == 0)
    {
        for (int i = 0; i < layout_list->count(); ++i) {
            QWidget *widget = layout_list->itemAt(i)->widget();
            if (widget)
                widget->hide();
        }

        change_playlist = 1;
    }
    else
    {
        for (int i = 0; i < layout_list->count(); ++i) {
            QWidget *widget = layout_list->itemAt(i)->widget();
            if (widget)
                widget->show();
        }

        change_playlist = 0;
    }
}

/*******************随机播放图标******************/
void Widget::randomClicked(void)
{
    if(change_random == 0)
    {

        random->setStyleSheet("QPushButton { background-color: #b5ded2; color: #000000; border-radius: 5px;}");
        change_random = 1;
    }
    else
    {
        random->setStyleSheet("QPushButton { color: white; border-radius: 5px; } QPushButton:hover { background-color: #b5ded2; color: #000000; }");
        change_random = 0;
    }

}

/*******************重复播放列表图标******************/
void Widget::repeatClicked(void)
{
    if(repeat_singel == 0) //重复模式选取
    {
        repeat->setStyleSheet("QPushButton { background-color: #b5ded2; color: #000000; border-radius: 5px;}");

        repeat_singel = 1;

    }
    else if(repeat_singel == 1) //单曲循环模式
    {
        repeat->setStyleSheet("QPushButton { background-color: #b5ded2; color: #000000; border-radius: 5px;}");

        QPixmap repeat_icon(SINGLE_ICON);
        repeat->setIconSize(QSize(24,24));
        repeat->setIcon(repeat_icon);

        repeat_singel = 2;

    }
    else //取消选中
    {
        repeat->setStyleSheet("QPushButton { color: #000000; border-radius: 5px;}");

        QPixmap repeat_icon(REPEAT_ICON);
        repeat->setIconSize(QSize(24,24));
        repeat->setIcon(repeat_icon);

        repeat_singel = 0;
    }

}

/*******************音量图标******************/
void Widget::volumeClicked(void)
{

    if(change_mute == 0)
    {
        last_volume_value = mplayerVolume;

        QPixmap volume_icon(VOLUME_MUTE_ICON);
        volume->setIconSize(QSize(24,24));
        volume->setIcon(volume_icon);

        volume_slide->setValue(0);
        change_mute = 1;
    }
    else
    {
        if((last_volume_value > 50) && (last_volume_value <= 100))
        {
            QPixmap volume_icon(VOLUME_MAX_ICON);
            volume->setIconSize(QSize(24,24));
            volume->setIcon(volume_icon);
        }
        else if((last_volume_value > 0) && (last_volume_value <= 50))
        {
            QPixmap volume_icon(VOLUME_MID_ICON);
            volume->setIconSize(QSize(24,24));
            volume->setIcon(volume_icon);
        }
        else if(last_volume_value == 0)
        {
            QPixmap volume_icon(VOLUME_MUTE_ICON);
            volume->setIconSize(QSize(24,24));
            volume->setIcon(volume_icon);
        }

        volume_slide->setValue(last_volume_value);
        change_mute = 0;
    }

}


Widget::Widget(int pipe_fd_0, QWidget *parent)
    : QWidget(parent)
{
    //设置主窗口的信息
    this->resize(860, 640); // 设置窗口大小
    this->setWindowIcon(QIcon(MUSIC_ICON)); // 设置窗口图标
    this->setWindowTitle("Music Player"); // 设置窗口标题

    //设置桌面背景图
//    QPixmap pix;
//    QPalette pal = this->palette();
//    pix.load(WALLPAPER);
//    pix = pix.scaled(800, 600, Qt::KeepAspectRatio);
//    pal.setBrush(QPalette::Background, QBrush(pix));
//    setPalette(pal);

    RECV_ARG *w = (RECV_ARG *)malloc(sizeof(RECV_ARG));
    pthread_t tid1;
    pthread_create(&tid1, NULL, send_cmd, NULL);
    pthread_detach(tid1);

    w->b=(Widget *)malloc(sizeof(Widget));
    w->a = pipe_fd_0;
    w->b = this;

    pthread_t tid2;
    pthread_create(&tid2, NULL, recv_msg, (void *)w);
    pthread_detach(tid2);

    //获取歌曲数目
    song_num = get_song_num();
    if (song_num <= 0)
    {
        printf("No songs found.\n");
        exit(1);
    }

    //获取随机歌曲数组
    arr = (int *)malloc(song_num * sizeof(int));  // 分配内存
    for (int i = 0; i < song_num; i++)
    {
        arr[i] = i;
    }
    fisherYates(song_num);

    //获取歌曲列表
    song_list = get_song_list();
    if (song_list == NULL)
    {
        printf("Song list is empty.\n");
        exit(1);
    }

    //添加控件
    set_window();

    connect(this->list_song, &QListWidget::itemDoubleClicked, this, &Widget::handleCellClicked); //双击歌曲列表
    connect(this->pause_continue, &QPushButton::clicked, this, &Widget::start_pause); //开始暂停
    connect(this->next_song, &QPushButton::clicked, this, &Widget::to_next_song); //下一首
    connect(this->last_song, &QPushButton::clicked, this, &Widget::to_last_song); //下一首
    connect(this->volume_slide, &QSlider::valueChanged, this, &Widget::set_volume); //音量控制
    connect(this->volume, &QPushButton::clicked, this, &Widget::volumeClicked); //音量控制
    connect(this->progressBar, &QSlider::sliderMoved, this, &Widget::set_progress); //进度条拖动控制
    //connect(this->progressBar, &QSlider::mouseReleaseEvent, this, &Widget::mouse_progress); //重写进度条点击事件
    connect(this, &Widget::get_time, this, &Widget::set_time); //设置时间标签信号
    connect(this->timer, &QTimer::timeout, this, &Widget::updateSlider); //进度条自走，歌曲播放结束切歌
    connect(this->playlist, &QPushButton::clicked, this, &Widget::playlistClicked); //显隐藏播放列表
    connect(this->random, &QPushButton::clicked, this, &Widget::randomClicked); //随机播放
    connect(this->repeat, &QPushButton::clicked, this, &Widget::repeatClicked); //循环播放

    //定时器关联刷新歌词
    connect(this->timer, &QTimer::timeout, this, [this]() {
        set_song_msg(num);  // 更新歌词区信息
    });

}


Widget::~Widget()
{
    free(arr);
    //释放歌曲列表空间
    free_song_list(song_list);
}
