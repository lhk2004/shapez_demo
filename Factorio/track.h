#ifndef TRACK_H
#define TRACK_H
#include "device.h"

class track : public device
{
public:
    bool is_corner;

public:
    track();
    track(bool is_corner, int dir);
    void switch_to_corner_direction(int dir);
};
#endif // TRACK_H
