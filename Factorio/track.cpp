#include "track.h"

track::track(bool is_cor, int dir)
{
    is_corner = is_cor;
    direction = dir;
    if (is_corner == 1)
    {
        pic.load(CORNER_TRACK_PATH);
        if (dir == RIGHT)  //The direction here represents from where the belt on the track should flow out clockwise
        {
            pic = pic.transformed(QTransform().rotate(90));
        }
        else if (dir == DOWN)
        {
            pic = pic.transformed(QTransform().rotate(180));
        }
        else if (dir == LEFT)
        {
            pic = pic.transformed(QTransform().rotate(270));
        }
    }
    else
    {
        pic.load(VERTICAL_TRACK_PATH);
        if (dir % 2 == 1)
        {
            pic = pic.transformed(QTransform().rotate(90));
        }
    }

    width = MAP_CELL_WIDTH;
    height = MAP_CELL_HEIGHT;
}

void track::switch_to_corner_direction(int dir)
{
    is_corner = 1;
    direction = dir;
    pic.load(CORNER_TRACK_PATH);
    if (dir == RIGHT)  //The direction here represents from where the belt on the track should flow out clockwise
    {
        pic = pic.transformed(QTransform().rotate(90));
    }
    else if (dir == DOWN)
    {
        pic = pic.transformed(QTransform().rotate(180));
    }
    else if (dir == LEFT)
    {
        pic = pic.transformed(QTransform().rotate(270));
    }
}
