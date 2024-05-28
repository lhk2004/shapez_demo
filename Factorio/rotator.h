#ifndef ROTATOR_H
#define ROTATOR_H
#include "device.h"

class rotator : public device
{
public:
    rotator();
    rotator(int dir);
};

#endif // ROTATOR_H
