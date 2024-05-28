#ifndef CUTTER_H
#define CUTTER_H
#include "device.h"

class cutter : public device
{
public:
    cutter();
    cutter(int dir);
};

#endif // CUTTER_H
