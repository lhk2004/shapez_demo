#ifndef CONFIG_H
#define CONFIG_H

/********** Main Menu Loading Paths **********/
#define NEW_GAME_PATH ":/main_menu/new_game.png"
#define LOAD_PATH ":/main_menu/load.png"
#define SHOP_PATH ":/main_menu/shop.png"
#define IN_SHOP_BUYING_PATH ":/pic/shop.png"
#define QUIT_PATH ":/main_menu/quit.png"
#define BACKGROUND_PATH ":/main_menu/background.png"

/********** MainScene Settings **********/
#define GAME_WIDTH 785
#define GAME_HEIGHT 711
#define GAME_TITLE "Factorio"
#define GAME_RES_PATH "./res.qrc"
#define GAME_ICON_PATH ":/pic/Factorio.ico"
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3
#define DEFAULT_SMALLER_MAP_NUM_ROWS 41
#define DEFAULT_SMALLER_MAP_NUM_COLS 41
#define DEFAULT_LARGER_MAP_NUM_ROWS 46
#define DEFAULT_LARGER_MAP_NUM_COLS 45

/********** Map Loading Paths **********/
#define SMALLER_MAP_PATH ":/pic/Map.png"
#define LARGER_MAP_PATH ":/pic/larger_map.png"
#define SMALLER_DELIVERY_CENTER_PATH ":/pic/smaller_delivery_center.png"
#define LARGER_DELIVERY_CENTER_PATH ":/pic/larger_delivery_center.png"
#define COIN_PATH ":/pic/coin.png"
#define SAVE_ICON_PATH ":/pic/save_icon.png"
#define HOME_ICON_PATH ":/pic/home_icon.png"
#define HANDLE_BAR_PATH ":/pic/handle_bar.png"
#define MINE_1_PATH ":/pic/mine_1.png"
#define MINE_2_PATH ":/pic/mine_2.png"
#define MINE_3_PATH ":/pic/mine_3.png"
#define MINE_4_PATH ":/pic/mine_4.png"
#define DIGGER_PATH ":/pic/digger.png"
#define CORNER_TRACK_PATH ":/pic/corner_track.png"
#define VERTICAL_TRACK_PATH ":/pic/vertical_track.png"
#define ROTATOR_PATH ":/pic/rotator.png"
#define CUTTER_PATH ":/pic/cutter.png"
#define TRASH_CAN_PATH ":/pic/trash_can.png"

/********** Map Settings **********/
#define HANDLE_BAR_LENGTH 381
#define DELIVERY_CENTER_INIT_X GAME_WIDTH / 2 - 49
#define DELIVERY_CENTER_INIT_Y GAME_HEIGHT / 2 - 31
#define DELIVERY_CENTER_WIDTH 80
#define DELIVERY_CENTER_HEIGHT 78
#define DEFAULT_MINE_BLOCK_NUM 6

/********** Cell Settings **********/
#define MAP_CELL_WIDTH 19.2
#define MAP_CELL_HEIGHT 17.3
#define EMPTY_CELL 0
#define DELIVERY_CENTER_CELL -1
#define DELIVERY_CENTER_OUTSKIRT_CELL -2
#define CUTTER_IN_CELL -3
#define CUTTER_TRASH_CELL -4

/********** Delivery Center Settings **********/
#define SMALLER_MODE 0
#define LARGER_MODE 1
#define SMALLER_DELIVERY_CENTER_X_LEFTBOUND 18
#define SMALLER_DELIVERY_CENTER_X_RIGHTBOUND 21
#define SMALLER_DELIVERY_CENTER_Y_UPPERBOUND 19
#define SMALLER_DELIVERY_CENTER_Y_LOWERBOUND 22
#define LARGER_DELIVERY_CENTER_X_LEFTBOUND 18
#define LARGER_DELIVERY_CENTER_X_RIGHTBOUND 22
#define LARGER_DELIVERY_CENTER_Y_UPPERBOUND 19
#define LARGER_DELIVERY_CENTER_Y_LOWERBOUND 23

/********** Moving Mines Settings **********/
#define RIGHT2DOWN 5
#define DOWN2LEFT 6
#define LEFT2UP 7
#define UP2RIGHT 8
#define UP2LEFT 9
#define LEFT2DOWN 10
#define DOWN2RIGHT 11
#define RIGHT2UP 12
#define MOVING_MINE_1_PATH ":/pic/moving_mine_1.png"
#define CUTTED_MOVING_MINE_1_PATH ":/pic/cutted_moving_mine_1.png"
#define MOVING_MINE_2_PATH ":/pic/moving_mine_2.png"
#define CUTTED_MOVING_MINE_2_PATH ":/pic/cutted_moving_mine_2.png"
#define MOVING_MINE_3_PATH ":/pic/moving_mine_3.png"
#define MOVING_MINE_4_PATH ":/pic/moving_mine_4.png"


/********** Game Settings **********/
#define MINE_TYPE_NUM 4
#define GAME_RATE 10
#define DEFAULT_DIGGING_SPEED 6000
#define DEFAULT_PROCESSING_SPEED 800
#define DEFALUT_MOVING_SPEED 0.25
#define MISSION_STAGE_NUM 4


/********** Font Settings **********/
#define MISSION_STAGE_1_PATH ":/font/mission_stage_1.png"
#define MISSION_STAGE_2_PATH ":/font/mission_stage_2.png"
#define MISSION_STAGE_3_PATH ":/font/mission_stage_3.png"
#define MISSION_STAGE_4_PATH ":/font/mission_stage_4.png"
#define NUM_0_PATH ":/font/num_0.png"
#define NUM_1_PATH ":/font/num_1.png"
#define NUM_2_PATH ":/font/num_2.png"
#define NUM_3_PATH ":/font/num_3.png"
#define NUM_4_PATH ":/font/num_4.png"
#define NUM_5_PATH ":/font/num_5.png"
#define NUM_6_PATH ":/font/num_6.png"
#define NUM_7_PATH ":/font/num_7.png"
#define NUM_8_PATH ":/font/num_8.png"
#define NUM_9_PATH ":/font/num_9.png"
#define DELIVER_PATH ":/font/deliver.png"
#define GOAL_1_PATH ":/font/goal_1.png"
#define GOAL_2_AND_4_PATH ":/font/goal_2_and_4.png"
#define GOAL_3_PATH ":/font/goal_3.png"
#define IN_GAME_UPGRADE_PATH ":/font/in_game_upgrade.png"

#endif // CONFIG_H
