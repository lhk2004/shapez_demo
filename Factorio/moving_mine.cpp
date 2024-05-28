#include "moving_mine.h"
#include <QDebug>

moving_mine::moving_mine(float pos_x, float pos_y, int mov_dir, int mine_type)
{
    type = mine_type;
    if (mine_type == 1) pic.load(MOVING_MINE_1_PATH);
    else if (mine_type == 2) pic.load(MOVING_MINE_2_PATH);
    else if (mine_type == 3) pic.load(MOVING_MINE_3_PATH);
    else if (mine_type == 4) pic.load(MOVING_MINE_4_PATH);

    x = pos_x + 2.5;
    y = pos_y + 2;
    is_cut = 0;
    if (mine_type == 1 || mine_type == 2)
    {
        can_be_cut = 1;
    }
    else if (mine_type == 3 || mine_type == 4)
    {
        can_be_cut = 0;
    }
    direction = UP;
    moving_direction = mov_dir;
    control_disabled = 0;
}

void moving_mine::cut()
{
    is_cut = 1;
    if (type == 1)
    {
        pic.load(CUTTED_MOVING_MINE_1_PATH);
    }
    else if (type == 2)
    {
        pic.load(CUTTED_MOVING_MINE_2_PATH);
    }
    direction = UP;
}

void moving_mine::rotate()
{
    pic = pic.transformed(QTransform().rotate(90));
    direction++;
    if (direction > LEFT) direction = UP;
}

void moving_mine::paint(QPainter &painter, float pos_x, float pos_y, float scaleFactor)
{

    QSize paint_size(MAP_CELL_WIDTH, MAP_CELL_HEIGHT);
    paint_size *= 0.8;
    paint_size *= scaleFactor;
    QPixmap paint_pic = pic.scaled(paint_size);

    painter.drawPixmap(pos_x, pos_y, paint_pic);
}
