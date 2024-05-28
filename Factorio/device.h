#ifndef DEVICE_H
#define DEVICE_H
#include <QPixmap>
#include <QPainter>
#include <QSize>
#include "config.h"

class device
{
public:
    device();
    void paint(QPainter& painter, int x, int y, float scaleFactor);

public:
    int direction;
    QPixmap pic;
    double width, height;
};

#endif // DEVICE_H
