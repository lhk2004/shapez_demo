#ifndef MAINSCENE_H
#define MAINSCENE_H

#include <cmath>
#include <QWidget>
#include <QTimer>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <ctime>
#include <QCursor>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QTextStream>
#include "track.h"
#include "digger.h"
#include "moving_mine.h"
#include "config.h"
#include "cutter.h"
#include "rotator.h"
#include "trash_can.h"

enum
{
    FREE,
    PUTTING_TRACK,
    DRAWING_TRACK,
    PUTTING_DIGGER,
    PUTTING_ROTATOR,
    PUTTING_CUTTER,
    PUTTING_TRASH_CAN
};

enum
{
    MAIN_MENU,
    START,
    IN_GAME_MENU,
    IN_GAME_UPGRADE,
    SHOP
};

struct Node  //Linklist Node for moving mines
{
    moving_mine content;
    Node* next;

    Node() = delete;

    Node(moving_mine a, Node* n = nullptr) : content(a), next(n) {}
};

struct Map
{
    QPixmap map;
    QPixmap delivery_center;
    QPixmap handle_bar;
    QPixmap mine_1;
    QPixmap mine_2;
    QPixmap mine_3;
    QPixmap mine_4;
};

struct map_cell
{

    int state = 0;  //-2: outskirt of delivery center -1: delivery center   0: empty cell  1 - MINE_TYPE_NUM: mines
    bool has_digger = 0;
    digger* attached_digger = NULL;
    bool has_track = 0;
    track* attached_track = NULL;
    bool has_cutter = 0;
    cutter* attached_cutter = NULL;
    bool has_trash_can = 0;
    trash_can* attached_trash_can = NULL;
    bool has_rotator = 0;
    rotator* attached_rotator = NULL;
};


QT_BEGIN_NAMESPACE
namespace Ui {
class MainScene;
}
QT_END_NAMESPACE

class MainScene : public QWidget
{
    Q_OBJECT

public:
    MainScene(QWidget *parent = nullptr);
    ~MainScene();

    void main_menu();

    void init_cells();
    bool check_start_pos(int pos_x, int pos_y);
    QPoint find_start_pos();
    void generate_mine(QPoint pos, int cur_mine_type);
    void random_init_mines(int block_num);
    void init_paths();
    void init_map();
    void init_scene();
    void start_game();
    bool check_cell_has_moving_mines(int row, int col);
    void dig();
    bool check_moving_up(int up_egde_row, int cur_row, int cur_col);
    bool check_moving_down(int down_edge_row, int cur_row, int cur_col);
    bool check_moving_left(int left_edge_col, int cur_row, int cur_col);
    bool check_moving_right(int right_edge_col, int cur_row, int cur_col);
    void update_position();
    void check_finished_mines();
    void check_cut_mines();
    void check_rotate_mines();
    void draw_track();
    void paint_map();
    void paintEvent(QPaintEvent*);
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void load(QString input_request);
    void save(QString output_request);

    Map game_map;

    QTimer update_timer;
    QTimer overall_timer;
    QTimer dig_timer;



private:
    Ui::MainScene *ui;

    int coins = 0;

    float scaleFactor = 1.0;                        //used in painting when scrolling the mouse wheel, initialized as 1.0
    QPoint last_mouse_position;
    int print_position_x = 0, print_position_y = 0; //used in painting when dragging the mouse, initialized as 0
    map_cell cells[DEFAULT_LARGER_MAP_NUM_ROWS][DEFAULT_LARGER_MAP_NUM_COLS];
    int delivery_center_mode = SMALLER_MODE;
    int map_mode = SMALLER_MODE;
    int mouse_state = FREE;
    int to_be_put_direction = UP;
    int last_paint_track_row;
    int last_paint_track_col;
    Node* head[MINE_TYPE_NUM];
    int count_mines[MINE_TYPE_NUM] = { 0 };          //used in saving the game

    int game_state = MAIN_MENU;
    int digging_speed = DEFAULT_DIGGING_SPEED;
    double mine_moving_speed = DEFALUT_MOVING_SPEED;
    int processing_speed = DEFAULT_PROCESSING_SPEED;

    int get_coins_per_collection = 1;
    int mine_block_num = DEFAULT_MINE_BLOCK_NUM;

    int cur_mission_stage = 1;
    int has_collect_mines = 0;

    int NUM_ROWS;
    int NUM_COLS;
};
#endif // MAINSCENE_H
