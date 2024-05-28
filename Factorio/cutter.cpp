#include "cutter.h"


cutter::cutter(int dir)
{
    direction = dir;
    pic.load(CUTTER_PATH);
    width = 2 * MAP_CELL_WIDTH;
    height = MAP_CELL_HEIGHT;
    if (dir == RIGHT)
    {
        width = MAP_CELL_WIDTH;
        height = 2 * MAP_CELL_HEIGHT;
        pic = pic.transformed(QTransform().rotate(90));
    }
    else if (dir == DOWN)
    {
        width = 2 * MAP_CELL_WIDTH;
        height = MAP_CELL_HEIGHT;
        pic = pic.transformed(QTransform().rotate(180));
    }
    else if (dir == LEFT)
    {
        width = MAP_CELL_WIDTH;
        height = 2 * MAP_CELL_HEIGHT;
        pic = pic.transformed(QTransform().rotate(270));
    }
}


