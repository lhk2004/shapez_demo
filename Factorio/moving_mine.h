#ifndef MOVING_MINE_H
#define MOVING_MINE_H
#include "config.h"
#include <QPixmap>
#include <QPainter>
#include <QSize>

class moving_mine
{
public:
    moving_mine(float pos_x, float pos_y, int mov_dir, int mine_type);

    void cut();
    void rotate();

    void paint(QPainter& painter, float x, float y, float scaleFactor);

public:
    int type;
    QPixmap pic;
    bool can_be_cut;
    bool control_disabled;   //Used when the mine is crossing the cutter or the rotator
    bool is_cut;
    float x;
    float y;
    int direction;
    int moving_direction;
};

#endif // MOVING_MINE_H
