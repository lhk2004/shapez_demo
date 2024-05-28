#include "trash_can.h"

trash_can::trash_can()
{
    pic.load(TRASH_CAN_PATH);

    width = MAP_CELL_WIDTH;
    height = MAP_CELL_HEIGHT;
}
