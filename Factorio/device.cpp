#include "device.h"

device::device()
{

}

void device::paint(QPainter &painter, int x, int y, float scaleFactor)
{
    QSize paint_size(width, height);
    paint_size *= scaleFactor;
    QPixmap paint_pic = pic.scaled(paint_size);
    painter.drawPixmap(x, y, paint_pic);
}
