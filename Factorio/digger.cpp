#include "digger.h"

digger::digger(int dir)
{
    direction = dir;
    pic.load(DIGGER_PATH);

    if (dir == RIGHT) pic = pic.transformed(QTransform().rotate(90));
    else if (dir == DOWN) pic = pic.transformed(QTransform().rotate(180));
    else if (dir == LEFT) pic = pic.transformed(QTransform().rotate(270));

    width = MAP_CELL_WIDTH;
    height = MAP_CELL_HEIGHT;
}
