#ifndef DIGGER_H
#define DIGGER_H
#include "device.h"

class digger : public device
{
public:
    digger();
    digger(int dir);
};

#endif // DIGGER_H
