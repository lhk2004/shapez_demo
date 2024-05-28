#include "mainscene.h"
#include "ui_mainscene.h"
#include <iostream>


MainScene::MainScene(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainScene)
{

    ui->setupUi(this);
    update_timer.setInterval(GAME_RATE);
    update_timer.start();
    connect(&update_timer, &QTimer::timeout, [=](){
        update();
    });
    main_menu();
}

MainScene::~MainScene()
{
    if (game_state == START)
    {
        for (int i = 0; i < MINE_TYPE_NUM; i++)
        {
            while (head[i] != NULL)
            {
                Node* tmp = head[i];
                head[i] = head[i]->next;
                delete tmp;
            }
        }
        for (int i = 0; i < NUM_ROWS; i++)
        {
            for (int j = 0; j < NUM_COLS; j++)
            {
                delete cells[i][j].attached_digger;
                delete cells[i][j].attached_cutter;
                delete cells[i][j].attached_rotator;
                delete cells[i][j].attached_track;
                delete cells[i][j].attached_trash_can;
            }
        }
    }
    delete ui;
}

void MainScene::main_menu()
{
    //Read the config.txt
    QDir* dir = new QDir(QDir::currentPath());
    dir->cdUp();
    QString directory = dir->absolutePath() + "/Factorio/config.txt";
    delete dir;

    QFile file(directory);
    file.open(QIODevice::ReadOnly);
    QTextStream in_file(&file);
    in_file.setIntegerBase(10);
    in_file >> coins >> delivery_center_mode >> get_coins_per_collection >> mine_block_num >> map_mode;
    file.close();

    setFixedSize(GAME_WIDTH, GAME_HEIGHT);
    setWindowTitle(GAME_TITLE);
    setWindowIcon(QIcon(GAME_ICON_PATH));
    setMouseTracking(true);
}

void MainScene::init_cells()
{
    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 0; j < NUM_COLS; j++)
        {
            cells[i][j].state = EMPTY_CELL;
        }
    }
    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 0; j < NUM_COLS; j++)
        {
            if (delivery_center_mode == SMALLER_MODE)
            {
                if (j >= SMALLER_DELIVERY_CENTER_X_LEFTBOUND && j <= SMALLER_DELIVERY_CENTER_X_RIGHTBOUND
                    && i <= SMALLER_DELIVERY_CENTER_Y_LOWERBOUND && i >= SMALLER_DELIVERY_CENTER_Y_UPPERBOUND)
                {
                    cells[i][j].state = DELIVERY_CENTER_CELL;
                }
                if ((j == SMALLER_DELIVERY_CENTER_X_LEFTBOUND - 1 && i <= SMALLER_DELIVERY_CENTER_Y_LOWERBOUND + 1 && i >= SMALLER_DELIVERY_CENTER_Y_UPPERBOUND - 1)
                    || (j == SMALLER_DELIVERY_CENTER_X_RIGHTBOUND + 1 && i <= SMALLER_DELIVERY_CENTER_Y_LOWERBOUND + 1 && i >= SMALLER_DELIVERY_CENTER_Y_UPPERBOUND - 1)
                    || (i == SMALLER_DELIVERY_CENTER_Y_LOWERBOUND + 1 && j >= SMALLER_DELIVERY_CENTER_X_LEFTBOUND - 1 && j <= SMALLER_DELIVERY_CENTER_X_RIGHTBOUND + 1)
                    || (i == SMALLER_DELIVERY_CENTER_Y_UPPERBOUND - 1 && j >= SMALLER_DELIVERY_CENTER_X_LEFTBOUND - 1 && j <= SMALLER_DELIVERY_CENTER_X_RIGHTBOUND + 1))
                {
                    cells[i][j].state = DELIVERY_CENTER_OUTSKIRT_CELL;
                }
            }
            else
            {
                if (j >= LARGER_DELIVERY_CENTER_X_LEFTBOUND && j <= LARGER_DELIVERY_CENTER_X_RIGHTBOUND
                    && i <= LARGER_DELIVERY_CENTER_Y_LOWERBOUND && i >= LARGER_DELIVERY_CENTER_Y_UPPERBOUND)
                {
                    cells[i][j].state = DELIVERY_CENTER_CELL;
                }
                if ((j == LARGER_DELIVERY_CENTER_X_LEFTBOUND - 1 && i <= LARGER_DELIVERY_CENTER_Y_LOWERBOUND + 1 && i >= LARGER_DELIVERY_CENTER_Y_UPPERBOUND - 1)
                    || (j == LARGER_DELIVERY_CENTER_X_RIGHTBOUND + 1 && i <= LARGER_DELIVERY_CENTER_Y_LOWERBOUND + 1 && i >= LARGER_DELIVERY_CENTER_Y_UPPERBOUND - 1)
                    || (i == LARGER_DELIVERY_CENTER_Y_LOWERBOUND + 1 && j >= LARGER_DELIVERY_CENTER_X_LEFTBOUND - 1 && j <= LARGER_DELIVERY_CENTER_X_RIGHTBOUND + 1)
                    || (i == LARGER_DELIVERY_CENTER_Y_UPPERBOUND - 1 && j >= LARGER_DELIVERY_CENTER_X_LEFTBOUND - 1 && j <= LARGER_DELIVERY_CENTER_X_RIGHTBOUND + 1))
                {
                    cells[i][j].state = DELIVERY_CENTER_OUTSKIRT_CELL;
                }
            }

        }
    }
}

bool MainScene::check_start_pos(int pos_x, int pos_y)
{
    if (pos_x + 6 > NUM_ROWS || pos_y + 7 > NUM_COLS)return 0;
    for (int i = pos_x; i < pos_x + 6; i++)
    {
        for (int j = pos_y; j < pos_y + 7; j++)
        {
            if (cells[i][j].state != 0)return 0;
        }
    }
    return 1;
}

QPoint MainScene::find_start_pos()
{
    int pos_x = rand() % NUM_ROWS, pos_y = rand() % NUM_COLS;
    while (!check_start_pos(pos_x, pos_y))
    {
        pos_x =rand() % NUM_ROWS;
        pos_y = rand() % NUM_COLS;
    }
    return QPoint(pos_x, pos_y);
}

void MainScene::generate_mine(QPoint pos, int cur_mine_type)
{
    //each time randomly generate 4 or 5 or 6 rows of mines
    int rows = rand() % 3 + 4;
    int last_start_col = -1;
    int last_end_col = -1;
    for (int j = pos.x(); j < pos.x() + rows; j++)
    {
        int start_col;
        if (j == pos.x())
        {
            start_col = pos.y() + 1 + rand() % 2;
        }
        else
        {
            start_col = pos.y() + rand() % 2;
        }
        //(1)if the current row belongs to the first half of the rows and is not the first row -> make sure start_col is less than or equal to last_start_col
        //(2)if the current row belongs to the second half of the rows -> make sure start_col is greater than or equal to last_start_col or is less than or equal to the last_end_col
        while ( ((j != pos.x() && j - pos.x() < rows / 2) && (start_col > last_start_col))
               || ((j - pos.x() >= rows / 2) && ((start_col < last_start_col) || (start_col > last_end_col))) )
        {
            start_col = pos.y() + rand() % 3;
        }
        last_start_col = start_col;
        int cols;
        if (j == pos.x())
        {
            cols = rand() % 2 + 1;
        }
        else
        {
            cols = rand() % 4 + 2;
        }
        last_end_col = start_col + cols - 1;
        for (int k = start_col; k < start_col + cols; k++)
        {
            cells[j][k].state = cur_mine_type + 1;
        }
    }
}

void MainScene::random_init_mines(int block_num)
{

    bool has_initialized_mine_type[MINE_TYPE_NUM] = { 0 };
    //First make sure that every type of mine can be generated on the map
    for (int i = 0; i < std::min(MINE_TYPE_NUM, block_num); i++)
    {
        int cur_mine_type = rand() % MINE_TYPE_NUM;
        while (has_initialized_mine_type[cur_mine_type] == 1)
        {
            cur_mine_type = rand() % MINE_TYPE_NUM;
        }
        has_initialized_mine_type[cur_mine_type] = 1;
        generate_mine(find_start_pos(), cur_mine_type);
    }
    //Next generate the mine blocks left
    for (int i = std::min(MINE_TYPE_NUM, block_num); i < block_num; i++)
    {
        int cur_mine_type = rand() % MINE_TYPE_NUM;
        generate_mine(find_start_pos(), cur_mine_type);
    }
}

void MainScene::init_paths()
{
    if (map_mode == SMALLER_MODE) game_map.map.load(SMALLER_MAP_PATH);
    else game_map.map.load(LARGER_MAP_PATH);
    if (delivery_center_mode == SMALLER_MODE) game_map.delivery_center.load(SMALLER_DELIVERY_CENTER_PATH);
    else game_map.delivery_center.load(LARGER_DELIVERY_CENTER_PATH);
    game_map.handle_bar.load(HANDLE_BAR_PATH);
    game_map.mine_1.load(MINE_1_PATH);
    game_map.mine_2.load(MINE_2_PATH);
    game_map.mine_3.load(MINE_3_PATH);
    game_map.mine_4.load(MINE_4_PATH);
}

void MainScene::init_map()
{

    init_paths();
    init_cells();
    random_init_mines(mine_block_num);
}

void MainScene::init_scene()
{

    for (int i = 0; i < MINE_TYPE_NUM; i++)
    {
        head[i] = NULL;
    }

    srand((unsigned)time(NULL));


    init_map();


    overall_timer.setInterval(GAME_RATE);
    dig_timer.setInterval(digging_speed);
}

void MainScene::start_game()
{
    overall_timer.start();
    dig_timer.start();

    static int has_called;
    if (has_called == 0)
    {
        connect(&overall_timer, &QTimer::timeout, [=](){
            update_position();
            check_finished_mines();
            check_cut_mines();
            check_rotate_mines();
        });
        connect(&dig_timer, &QTimer::timeout, [=](){
            dig();
        });
        has_called++;
    }


}

bool MainScene::check_cell_has_moving_mines(int row, int col)
{
    for (int i = 0; i < MINE_TYPE_NUM; i++)
    {
        Node* tmp = head[i];
        while (tmp != NULL)
        {
            int cur_row = (tmp->content.y + MAP_CELL_HEIGHT * 0.4) / MAP_CELL_HEIGHT;
            int cur_col = (tmp->content.x + MAP_CELL_WIDTH * 0.4) / MAP_CELL_WIDTH;
            if (cur_row == row && cur_col == col)return 1;
            tmp = tmp->next;
        }
    }
    return 0;
}

void MainScene::dig()
{
    if (game_state == START)
    {
        for (int i = 0; i < NUM_ROWS; i++)
        {
            for (int j = 0; j < NUM_COLS; j++)
            {
                if (cells[i][j].has_digger && cells[i][j].state > 0)
                {
                    for (int mine_type = 1; mine_type <= MINE_TYPE_NUM; mine_type++)
                    {
                        if (cells[i][j].state == mine_type)
                        {
                            if (j - 1 >= 0 && cells[i][j].attached_digger->direction == LEFT && cells[i][j - 1].has_track &&
                                ( (cells[i][j - 1].attached_track->is_corner == 0
                                    && (cells[i][j - 1].attached_track->direction == RIGHT || cells[i][j - 1].attached_track->direction == LEFT) ) ) )
                            {
                                if (check_cell_has_moving_mines(i, j - 1) == 0)
                                {
                                    Node* tmp = new Node(moving_mine((j - 1) * MAP_CELL_WIDTH, i * MAP_CELL_HEIGHT, LEFT, mine_type), head[mine_type - 1]);
                                    head[mine_type - 1] = tmp;
                                    count_mines[mine_type - 1]++;
                                }
                            }
                            if (j + 1 < NUM_COLS && cells[i][j].attached_digger->direction == RIGHT && cells[i][j + 1].has_track &&
                                ( (cells[i][j + 1].attached_track->is_corner == 0
                                    && (cells[i][j + 1].attached_track->direction == RIGHT || cells[i][j + 1].attached_track->direction == LEFT) ) ) )
                            {
                                if (check_cell_has_moving_mines(i, j + 1) == 0)
                                {
                                    Node* tmp = new Node(moving_mine((j + 1) * MAP_CELL_WIDTH, i * MAP_CELL_HEIGHT, RIGHT, mine_type), head[mine_type - 1]);
                                    head[mine_type - 1] = tmp;
                                    count_mines[mine_type - 1]++;
                                }
                            }
                            if (i - 1 >= 0 && cells[i][j].attached_digger->direction == UP && cells[i - 1][j].has_track &&
                                ( (cells[i - 1][j].attached_track->is_corner == 0
                                    && (cells[i - 1][j].attached_track->direction == UP || cells[i - 1][j].attached_track->direction == DOWN) ) ) )
                            {
                                if (check_cell_has_moving_mines(i - 1, j) == 0)
                                {
                                    Node* tmp = new Node(moving_mine(j * MAP_CELL_WIDTH, (i - 1) * MAP_CELL_HEIGHT, UP, mine_type), head[mine_type - 1]);
                                    head[mine_type - 1] = tmp;
                                    count_mines[mine_type - 1]++;
                                }
                            }
                            if (i + 1 < NUM_ROWS && cells[i][j].attached_digger->direction == DOWN && cells[i + 1][j].has_track &&
                                ( (cells[i + 1][j].attached_track->is_corner == 0
                                    && (cells[i + 1][j].attached_track->direction == UP || cells[i + 1][j].attached_track->direction == DOWN) ) ) )
                            {
                                if (check_cell_has_moving_mines(i + 1, j) == 0)
                                {
                                    Node* tmp = new Node(moving_mine(j * MAP_CELL_WIDTH, (i + 1) * MAP_CELL_HEIGHT, DOWN, mine_type), head[mine_type - 1]);
                                    head[mine_type - 1] = tmp;
                                    count_mines[mine_type - 1]++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool MainScene::check_moving_up(int up_edge_row, int cur_row, int cur_col)
{
    if (cells[up_edge_row][cur_col].has_track == 1)
    {
        if (cur_row == up_edge_row)
        {
            if (cur_row - 1 >= 0 && check_cell_has_moving_mines(cur_row - 1, cur_col) == 0)
            {
                return 1;
            }
        }
        if ((cells[up_edge_row][cur_col].attached_track->is_corner == 0 && cells[up_edge_row][cur_col].attached_track->direction % 2 == 0)
            || (cells[up_edge_row][cur_col].attached_track->is_corner == 1 && cells[up_edge_row][cur_col].attached_track->direction == DOWN)
            || (cells[up_edge_row][cur_col].attached_track->is_corner == 1 && cells[up_edge_row][cur_col].attached_track->direction == RIGHT))
        {
            if (cur_row - 1 >= 0 && check_cell_has_moving_mines(cur_row - 1, cur_col) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

bool MainScene::check_moving_down(int down_edge_row, int cur_row, int cur_col)
{
    if (cells[down_edge_row][cur_col].has_track == 1)
    {
        if (cur_row == down_edge_row)
        {
            if (cur_row + 1 < NUM_ROWS && check_cell_has_moving_mines(cur_row + 1, cur_col) == 0)
            {
                return 1;
            }
        }
        else if ((cells[down_edge_row][cur_col].attached_track->is_corner == 0 && cells[down_edge_row][cur_col].attached_track->direction % 2 == 0)
                 || (cells[down_edge_row][cur_col].attached_track->is_corner == 1 && cells[down_edge_row][cur_col].attached_track->direction == LEFT)
                 || (cells[down_edge_row][cur_col].attached_track->is_corner == 1 && cells[down_edge_row][cur_col].attached_track->direction == UP))
        {
            if (cur_row + 1 < NUM_ROWS && check_cell_has_moving_mines(cur_row + 1, cur_col) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

bool MainScene::check_moving_left(int left_edge_col, int cur_row, int cur_col)
{
    if (cells[cur_row][left_edge_col].has_track == 1)
    {
        if (cur_col == left_edge_col)
        {
            if (cur_col - 1 >= 0 && check_cell_has_moving_mines(cur_row, cur_col - 1) == 0)
            {
                return 1;
            }
        }
        else if ((cells[cur_row][left_edge_col].attached_track->is_corner == 0 && cells[cur_row][left_edge_col].attached_track->direction % 2 == 1)
                 || (cells[cur_row][left_edge_col].attached_track->is_corner == 1 && cells[cur_row][left_edge_col].attached_track->direction == UP)
                 || (cells[cur_row][left_edge_col].attached_track->is_corner == 1 && cells[cur_row][left_edge_col].attached_track->direction == RIGHT))
        {
            if (cur_col - 1 >= 0 && check_cell_has_moving_mines(cur_row, cur_col - 1) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

bool MainScene::check_moving_right(int right_edge_col, int cur_row, int cur_col)
{
    if (cells[cur_row][right_edge_col].has_track == 1)
    {
        if (cur_col == right_edge_col)
        {
            if (cur_col + 1 < NUM_COLS && check_cell_has_moving_mines(cur_row, cur_col + 1) == 0)
            {
                return 1;
            }
        }
        else if ((cells[cur_row][right_edge_col].attached_track->is_corner == 0 && cells[cur_row][right_edge_col].attached_track->direction % 2 == 1)
                 || (cells[cur_row][right_edge_col].attached_track->is_corner == 1 && cells[cur_row][right_edge_col].attached_track->direction == LEFT)
                 || (cells[cur_row][right_edge_col].attached_track->is_corner == 1 && cells[cur_row][right_edge_col].attached_track->direction == DOWN))
        {
            if (cur_col + 1 < NUM_COLS && check_cell_has_moving_mines(cur_row, cur_col + 1) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

void MainScene::update_position()
{
    if (game_state == START)
    {
        for (int i = 0; i < MINE_TYPE_NUM; i++)
        {
            Node* tmp = head[i];
            while (tmp != NULL)
            {
                if (tmp->content.control_disabled == 1)
                {
                    tmp = tmp->next;
                    continue;
                }
                int up_edge_row = tmp->content.y / MAP_CELL_HEIGHT;
                int down_edge_row = (tmp->content.y + MAP_CELL_HEIGHT * 0.8) / MAP_CELL_HEIGHT;
                int left_edge_col = tmp->content.x / MAP_CELL_WIDTH;
                int right_edge_col = (tmp->content.x + MAP_CELL_WIDTH * 0.8) / MAP_CELL_WIDTH;
                int cur_row = (tmp->content.y + MAP_CELL_HEIGHT * 0.4) / MAP_CELL_HEIGHT;
                int cur_col = (tmp->content.x + MAP_CELL_WIDTH * 0.4) / MAP_CELL_WIDTH;

                if (cells[cur_row][cur_col].has_track == 1)
                {
                    //The track direction is vertical
                    if (cells[cur_row][cur_col].attached_track->is_corner == 0 && cells[cur_row][cur_col].attached_track->direction % 2 == 0)
                    {
                        if (tmp->content.moving_direction == RIGHT2DOWN || tmp->content.moving_direction == LEFT2DOWN) tmp->content.moving_direction = DOWN;
                        if (tmp->content.moving_direction == LEFT2UP || tmp->content.moving_direction == RIGHT2UP) tmp->content.moving_direction = UP;
                        if (tmp->content.moving_direction == UP)
                        {
                            if (check_moving_up(up_edge_row, cur_row, cur_col) == 1)
                            {
                                tmp->content.y -= mine_moving_speed;
                            }
                        }
                        else if (tmp->content.moving_direction == DOWN)
                        {
                            if (check_moving_down(down_edge_row, cur_row, cur_col) == 1)
                            {
                                tmp->content.y += mine_moving_speed;
                            }
                        }
                    }
                    //The track direction is horizontal
                    else if (cells[cur_row][cur_col].attached_track->is_corner == 0 && cells[cur_row][cur_col].attached_track->direction % 2 == 1)
                    {
                        if (tmp->content.moving_direction == DOWN2RIGHT || tmp->content.moving_direction == UP2RIGHT) tmp->content.moving_direction = RIGHT;
                        if (tmp->content.moving_direction == DOWN2LEFT || tmp->content.moving_direction == UP2LEFT) tmp->content.moving_direction = LEFT;
                        if (tmp->content.moving_direction == LEFT)
                        {
                            if (check_moving_left(left_edge_col, cur_row, cur_col) == 1)
                            {
                                tmp->content.x -= mine_moving_speed;
                            }
                        }
                        else if (tmp->content.moving_direction == RIGHT)
                        {
                            if (check_moving_right(right_edge_col, cur_row, cur_col) == 1)
                            {
                                tmp->content.x += mine_moving_speed;
                            }
                        }
                    }
                    //The track direction is corner
                    else if (cells[cur_row][cur_col].attached_track->is_corner == 1)
                    {
                        if (cells[cur_row][cur_col].attached_track->direction == DOWN)
                        {
                            if (tmp->content.moving_direction == RIGHT || tmp->content.moving_direction == DOWN2RIGHT || tmp->content.moving_direction == UP2RIGHT) tmp->content.moving_direction = RIGHT2DOWN;
                            if (tmp->content.moving_direction == UP || tmp->content.moving_direction == RIGHT2UP || tmp->content.moving_direction == LEFT2UP) tmp->content.moving_direction = UP2LEFT;
                            if (tmp->content.moving_direction == RIGHT2DOWN)
                            {
                                if (check_moving_down(down_edge_row, cur_row, cur_col) == 1)
                                {
                                    float x_speed = mine_moving_speed * (MAP_CELL_WIDTH / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    float y_speed = mine_moving_speed * (MAP_CELL_HEIGHT / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    tmp->content.x += x_speed;
                                    tmp->content.y += y_speed;
                                }
                            }
                            else if (tmp->content.moving_direction == UP2LEFT)
                            {
                                if (check_moving_left(left_edge_col, cur_row, cur_col) == 1)
                                {
                                    float x_speed = mine_moving_speed * (MAP_CELL_WIDTH / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    float y_speed = mine_moving_speed * (MAP_CELL_HEIGHT / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    tmp->content.x -= x_speed;
                                    tmp->content.y -= y_speed;
                                }
                            }
                        }
                        else if (cells[cur_row][cur_col].attached_track->direction == LEFT)
                        {
                            if (tmp->content.moving_direction == DOWN || tmp->content.moving_direction == LEFT2DOWN || tmp->content.moving_direction == RIGHT2DOWN) tmp->content.moving_direction = DOWN2LEFT;
                            if (tmp->content.moving_direction == RIGHT || tmp->content.moving_direction == DOWN2RIGHT || tmp->content.moving_direction == UP2RIGHT) tmp->content.moving_direction = RIGHT2UP;
                            if (tmp->content.moving_direction == RIGHT2UP)
                            {
                                if (check_moving_up(up_edge_row, cur_row, cur_col) == 1)
                                {
                                    float x_speed = mine_moving_speed * (MAP_CELL_WIDTH / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    float y_speed = mine_moving_speed * (MAP_CELL_HEIGHT / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    tmp->content.x += x_speed;
                                    tmp->content.y -= y_speed;
                                }
                            }
                            else if (tmp->content.moving_direction == DOWN2LEFT)
                            {
                                if (check_moving_left(left_edge_col, cur_row, cur_col) == 1)
                                {
                                    float x_speed = mine_moving_speed * (MAP_CELL_WIDTH / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    float y_speed = mine_moving_speed * (MAP_CELL_HEIGHT / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    tmp->content.x -= x_speed;
                                    tmp->content.y += y_speed;
                                }
                            }
                        }
                        else if (cells[cur_row][cur_col].attached_track->direction == UP)
                        {
                            if (tmp->content.moving_direction == LEFT || tmp->content.moving_direction == DOWN2LEFT || tmp->content.moving_direction == UP2LEFT) tmp->content.moving_direction = LEFT2UP;
                            if (tmp->content.moving_direction == DOWN || tmp->content.moving_direction == LEFT2DOWN || tmp->content.moving_direction == RIGHT2DOWN) tmp->content.moving_direction = DOWN2RIGHT;
                            if (tmp->content.moving_direction == LEFT2UP)
                            {
                                if (check_moving_up(up_edge_row, cur_row, cur_col) == 1)
                                {
                                    float x_speed = mine_moving_speed * (MAP_CELL_WIDTH / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    float y_speed = mine_moving_speed * (MAP_CELL_HEIGHT / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    tmp->content.x -= x_speed;
                                    tmp->content.y -= y_speed;
                                }
                            }
                            else if (tmp->content.moving_direction == DOWN2RIGHT)
                            {
                                if (check_moving_right(right_edge_col, cur_row, cur_col) == 1)
                                {
                                    float x_speed = mine_moving_speed * (MAP_CELL_WIDTH / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    float y_speed = mine_moving_speed * (MAP_CELL_HEIGHT / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    tmp->content.x += x_speed;
                                    tmp->content.y += y_speed;
                                }
                            }
                        }
                        else if (cells[cur_row][cur_col].attached_track->direction == RIGHT)
                        {
                            if (tmp->content.moving_direction == LEFT || tmp->content.moving_direction == DOWN2LEFT || tmp->content.moving_direction == UP2LEFT) tmp->content.moving_direction = LEFT2DOWN;
                            if (tmp->content.moving_direction == UP || tmp->content.moving_direction == RIGHT2UP || tmp->content.moving_direction == LEFT2UP) tmp->content.moving_direction = UP2RIGHT;
                            if (tmp->content.moving_direction == LEFT2DOWN)
                            {
                                if (check_moving_down(down_edge_row, cur_row, cur_col) == 1)
                                {
                                    float x_speed = mine_moving_speed * (MAP_CELL_WIDTH / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    float y_speed = mine_moving_speed * (MAP_CELL_HEIGHT / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    tmp->content.x -= x_speed;
                                    tmp->content.y += y_speed;
                                }
                            }
                            else if (tmp->content.moving_direction == UP2RIGHT)
                            {
                                if (check_moving_right(right_edge_col, cur_row, cur_col) == 1)
                                {
                                    float x_speed = mine_moving_speed * (MAP_CELL_WIDTH / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    float y_speed = mine_moving_speed * (MAP_CELL_HEIGHT / sqrt(MAP_CELL_WIDTH * MAP_CELL_WIDTH + MAP_CELL_HEIGHT * MAP_CELL_HEIGHT));
                                    tmp->content.x += x_speed;
                                    tmp->content.y -= y_speed;
                                }
                            }
                        }
                    }
                }
                tmp = tmp->next;
            }
        }
    }
}

void MainScene::check_finished_mines()
{
    if (game_state == START)
    {
        for (int i = 0; i < MINE_TYPE_NUM; i++)
        {
            Node* tmp = head[i];
            Node* last;
            while (tmp != NULL)
            {
                int up_edge_row = tmp->content.y / MAP_CELL_HEIGHT;
                int down_edge_row = (tmp->content.y + MAP_CELL_HEIGHT * 0.8) / MAP_CELL_HEIGHT;
                int left_edge_col = tmp->content.x / MAP_CELL_WIDTH;
                int right_edge_col = (tmp->content.x + MAP_CELL_WIDTH * 0.8) / MAP_CELL_WIDTH;
                int cur_row = (tmp->content.y + MAP_CELL_HEIGHT * 0.4) / MAP_CELL_HEIGHT;
                int cur_col = (tmp->content.x + MAP_CELL_WIDTH * 0.4) / MAP_CELL_WIDTH;
                if (cells[up_edge_row][cur_col].state == DELIVERY_CENTER_CELL
                    || cells[down_edge_row][cur_col].state == DELIVERY_CENTER_CELL
                    || cells[cur_row][left_edge_col].state == DELIVERY_CENTER_CELL
                    || cells[cur_row][right_edge_col].state == DELIVERY_CENTER_CELL)
                {
                    count_mines[tmp->content.type - 1]--;
                    if (cur_mission_stage == 1 && tmp->content.type == 3)
                    {
                        has_collect_mines++;
                        coins += get_coins_per_collection;
                        if (has_collect_mines == 20)
                        {
                            cur_mission_stage = 2;
                            has_collect_mines = 0;
                            game_state = IN_GAME_UPGRADE;
                        }
                    }
                    else if (cur_mission_stage == 2 && tmp->content.type == 4)
                    {
                        has_collect_mines++;
                        coins += get_coins_per_collection;
                        if (has_collect_mines == 30)
                        {
                            cur_mission_stage = 3;
                            has_collect_mines = 0;
                            game_state = IN_GAME_UPGRADE;
                        }
                    }
                    else if (cur_mission_stage == 3 && tmp->content.type == 1 && tmp->content.is_cut == 1 && tmp->content.direction == UP)
                    {
                        has_collect_mines++;
                        coins += get_coins_per_collection;
                        if (has_collect_mines == 50)
                        {
                            cur_mission_stage = 4;
                            has_collect_mines = 0;
                            game_state = IN_GAME_UPGRADE;
                        }
                    }
                    else if (cur_mission_stage == 4 && tmp->content.type == 2 && tmp->content.is_cut == 1 && tmp->content.direction == RIGHT)
                    {
                        has_collect_mines++;
                        coins += get_coins_per_collection;
                        if (has_collect_mines == 30)
                        {
                            std::cout << "You have won the game!" << std::endl;

                        }
                    }
                    if (tmp == head[i])
                    {
                        Node* del = tmp;
                        head[i] = tmp->next;
                        tmp = tmp->next;
                        delete del;
                        del = NULL;
                    }
                    else
                    {
                        Node* del = tmp;
                        tmp = tmp->next;
                        last->next = tmp;
                        delete del;
                        del = NULL;
                    }
                }
                else
                {
                    last = tmp;
                    tmp = tmp->next;
                }
            }
        }
    }
}

void MainScene::check_cut_mines()
{
    if (game_state == START)
    {
        for (int i = 0; i < MINE_TYPE_NUM; i++)
        {
            if (head[i] != NULL && head[i]->content.can_be_cut == 0) continue;
            Node* tmp = head[i];
            while (tmp != NULL)
            {
                int up_edge_row = tmp->content.y / MAP_CELL_HEIGHT;
                int down_edge_row = (tmp->content.y + MAP_CELL_HEIGHT * 0.8) / MAP_CELL_HEIGHT;
                int left_edge_col = tmp->content.x / MAP_CELL_WIDTH;
                int right_edge_col = (tmp->content.x + MAP_CELL_WIDTH * 0.8) / MAP_CELL_WIDTH;
                int cur_row = (tmp->content.y + MAP_CELL_HEIGHT * 0.4) / MAP_CELL_HEIGHT;
                int cur_col = (tmp->content.x + MAP_CELL_WIDTH * 0.4) / MAP_CELL_WIDTH;
                if (cells[up_edge_row][cur_col].state == CUTTER_IN_CELL)
                {
                    if (up_edge_row - 1 >= 0 && cells[up_edge_row - 1][cur_col + 1].has_trash_can == 1 && cells[up_edge_row - 1][cur_col].has_track == 1 &&
                        ((cells[up_edge_row - 1][cur_col].attached_track->is_corner == 0 && cells[up_edge_row - 1][cur_col].attached_track->direction % 2 == 0)
                            || (cells[up_edge_row - 1][cur_col].attached_track->is_corner == 1 && cells[up_edge_row - 1][cur_col].attached_track->direction == DOWN)
                            || (cells[up_edge_row - 1][cur_col].attached_track->is_corner == 1 && cells[up_edge_row - 1][cur_col].attached_track->direction == RIGHT)))
                    {
                        if (check_cell_has_moving_mines(up_edge_row - 1, cur_col) == 0)
                        {
                            tmp->content.x = cur_col * MAP_CELL_WIDTH + 2.5;
                            tmp->content.y = (up_edge_row - 1) * MAP_CELL_HEIGHT + 2;
                            tmp->content.cut();

                            //Make the mine invisible and immobolized for 2 seconds
                            tmp->content.control_disabled = 1;
                            QTimer* enableControlTimer = new QTimer(this);
                            enableControlTimer->setSingleShot(true);
                            connect(enableControlTimer, &QTimer::timeout, [=]() {
                                tmp->content.control_disabled = 0;
                                enableControlTimer->deleteLater();
                            });
                            enableControlTimer->start(processing_speed);
                        }
                    }
                }
                else if (cells[down_edge_row][cur_col].state == CUTTER_IN_CELL)
                {
                    if (down_edge_row + 1 < NUM_ROWS && cells[down_edge_row + 1][cur_col - 1].has_trash_can == 1 && cells[down_edge_row + 1][cur_col].has_track == 1 &&
                        ((cells[down_edge_row + 1][cur_col].attached_track->is_corner == 0 && cells[down_edge_row + 1][cur_col].attached_track->direction % 2 == 0)
                         || (cells[down_edge_row + 1][cur_col].attached_track->is_corner == 1 && cells[down_edge_row + 1][cur_col].attached_track->direction == LEFT)
                         || (cells[down_edge_row + 1][cur_col].attached_track->is_corner == 1 && cells[down_edge_row + 1][cur_col].attached_track->direction == UP)))
                    {
                        if (check_cell_has_moving_mines(down_edge_row + 1, cur_col) == 0)
                        {
                            tmp->content.x = cur_col * MAP_CELL_WIDTH + 2.5;
                            tmp->content.y = (down_edge_row + 1) * MAP_CELL_HEIGHT + 2;
                            tmp->content.cut();

                            //Make the mine invisible and immobolized for 2 seconds
                            tmp->content.control_disabled = 1;
                            QTimer* enableControlTimer = new QTimer(this);
                            enableControlTimer->setSingleShot(true);
                            connect(enableControlTimer, &QTimer::timeout, [=]() {
                                tmp->content.control_disabled = 0;
                                enableControlTimer->deleteLater();
                            });
                            enableControlTimer->start(processing_speed);
                        }
                    }
                }
                else if (cells[cur_row][left_edge_col].state == CUTTER_IN_CELL)
                {
                    if (left_edge_col - 1 >= 0 && cells[cur_row - 1][left_edge_col - 1].has_trash_can == 1 && cells[cur_row][left_edge_col - 1].has_track == 1 &&
                        ((cells[cur_row][left_edge_col - 1].attached_track->is_corner == 0 && cells[cur_row][left_edge_col - 1].attached_track->direction % 2 == 1)
                         || (cells[cur_row][left_edge_col - 1].attached_track->is_corner == 1 && cells[cur_row][left_edge_col - 1].attached_track->direction == RIGHT)
                         || (cells[cur_row][left_edge_col - 1].attached_track->is_corner == 1 && cells[cur_row][left_edge_col - 1].attached_track->direction == UP)))
                    {
                        if (check_cell_has_moving_mines(cur_row, left_edge_col - 1) == 0)
                        {
                            tmp->content.x = (left_edge_col - 1) * MAP_CELL_WIDTH + 2.5;
                            tmp->content.y = cur_row * MAP_CELL_HEIGHT + 2;
                            tmp->content.cut();

                            //Make the mine invisible and immobolized for 2 seconds
                            tmp->content.control_disabled = 1;
                            QTimer* enableControlTimer = new QTimer(this);
                            enableControlTimer->setSingleShot(true);
                            connect(enableControlTimer, &QTimer::timeout, [=]() {
                                tmp->content.control_disabled = 0;
                                enableControlTimer->deleteLater();
                            });
                            enableControlTimer->start(processing_speed);
                        }
                    }
                }
                else if (cells[cur_row][right_edge_col].state == CUTTER_IN_CELL)
                {
                    if (right_edge_col + 1 < NUM_COLS && cells[cur_row + 1][right_edge_col + 1].has_trash_can == 1 && cells[cur_row][right_edge_col + 1].has_track == 1 &&
                        ((cells[cur_row][right_edge_col + 1].attached_track->is_corner == 0 && cells[cur_row][right_edge_col + 1].attached_track->direction % 2 == 1)
                         || (cells[cur_row][right_edge_col + 1].attached_track->is_corner == 1 && cells[cur_row][right_edge_col + 1].attached_track->direction == LEFT)
                         || (cells[cur_row][right_edge_col + 1].attached_track->is_corner == 1 && cells[cur_row][right_edge_col + 1].attached_track->direction == DOWN)))
                    {
                        if (check_cell_has_moving_mines(cur_row, right_edge_col + 1) == 0)
                        {
                            tmp->content.x = (right_edge_col + 1) * MAP_CELL_WIDTH + 2.5;
                            tmp->content.y = cur_row * MAP_CELL_HEIGHT + 2;
                            tmp->content.cut();

                            //Make the mine invisible and immobolized for 2 seconds
                            tmp->content.control_disabled = 1;
                            QTimer* enableControlTimer = new QTimer(this);
                            enableControlTimer->setSingleShot(true);
                            connect(enableControlTimer, &QTimer::timeout, [=]() {
                                tmp->content.control_disabled = 0;
                                enableControlTimer->deleteLater();
                            });
                            enableControlTimer->start(processing_speed);
                        }
                    }
                }
                tmp = tmp->next;
            }
        }
    }
}

void MainScene::check_rotate_mines()
{
    if (game_state == START)
    {
        for (int i = 0; i < MINE_TYPE_NUM; i++)
        {
            if (head[i] != NULL && head[i]->content.can_be_cut == 0) continue;
            Node* tmp = head[i];
            while (tmp != NULL)
            {
                int up_edge_row = tmp->content.y / MAP_CELL_HEIGHT;
                int down_edge_row = (tmp->content.y + MAP_CELL_HEIGHT * 0.8) / MAP_CELL_HEIGHT;
                int left_edge_col = tmp->content.x / MAP_CELL_WIDTH;
                int right_edge_col = (tmp->content.x + MAP_CELL_WIDTH * 0.8) / MAP_CELL_WIDTH;
                int cur_row = (tmp->content.y + MAP_CELL_HEIGHT * 0.4) / MAP_CELL_HEIGHT;
                int cur_col = (tmp->content.x + MAP_CELL_WIDTH * 0.4) / MAP_CELL_WIDTH;
                if (cells[up_edge_row][cur_col].has_rotator == 1 && cells[up_edge_row][cur_col].attached_rotator->direction == UP)
                {
                    if (cells[up_edge_row - 1][cur_col].has_track == 1 &&
                        ((cells[up_edge_row - 1][cur_col].attached_track->is_corner == 0 && cells[up_edge_row - 1][cur_col].attached_track->direction % 2 == 0)
                         || (cells[up_edge_row - 1][cur_col].attached_track->is_corner == 1 && cells[up_edge_row - 1][cur_col].attached_track->direction == DOWN)
                         || (cells[up_edge_row - 1][cur_col].attached_track->is_corner == 1 && cells[up_edge_row - 1][cur_col].attached_track->direction == RIGHT)))
                    {
                        if (check_cell_has_moving_mines(up_edge_row - 1, cur_col) == 0)
                        {
                            tmp->content.x = cur_col * MAP_CELL_WIDTH + 2.5;
                            tmp->content.y = (up_edge_row - 1) * MAP_CELL_HEIGHT + 2;
                            tmp->content.rotate();

                            //Make the mine invisible and immobolized for 2 seconds
                            tmp->content.control_disabled = 1;
                            QTimer* enableControlTimer = new QTimer(this);
                            enableControlTimer->setSingleShot(true);
                            connect(enableControlTimer, &QTimer::timeout, [=]() {
                                tmp->content.control_disabled = 0;
                                enableControlTimer->deleteLater();
                            });
                            enableControlTimer->start(processing_speed);
                        }
                    }
                }
                else if (cells[down_edge_row][cur_col].has_rotator == 1 && cells[down_edge_row][cur_col].attached_rotator->direction == DOWN)
                {
                    if (cells[down_edge_row + 1][cur_col].has_track == 1 &&
                        ((cells[down_edge_row + 1][cur_col].attached_track->is_corner == 0 && cells[down_edge_row + 1][cur_col].attached_track->direction % 2 == 0)
                         || (cells[down_edge_row + 1][cur_col].attached_track->is_corner == 1 && cells[down_edge_row + 1][cur_col].attached_track->direction == LEFT)
                         || (cells[down_edge_row + 1][cur_col].attached_track->is_corner == 1 && cells[down_edge_row + 1][cur_col].attached_track->direction == UP)))
                    {
                        if (check_cell_has_moving_mines(down_edge_row + 1, cur_col) == 0)
                        {
                            tmp->content.x = cur_col * MAP_CELL_WIDTH + 2.5;
                            tmp->content.y = (down_edge_row + 1) * MAP_CELL_HEIGHT + 2;
                            tmp->content.rotate();

                            //Make the mine invisible and immobolized for 2 seconds
                            tmp->content.control_disabled = 1;
                            QTimer* enableControlTimer = new QTimer(this);
                            enableControlTimer->setSingleShot(true);
                            connect(enableControlTimer, &QTimer::timeout, [=]() {
                                tmp->content.control_disabled = 0;
                                enableControlTimer->deleteLater();
                            });
                            enableControlTimer->start(processing_speed);
                        }
                    }
                }
                else if (cells[cur_row][left_edge_col].has_rotator == 1 && cells[cur_row][left_edge_col].attached_rotator->direction == LEFT)
                {
                    if (cells[cur_row][left_edge_col - 1].has_track == 1 &&
                        ((cells[cur_row][left_edge_col - 1].attached_track->is_corner == 0 && cells[cur_row][left_edge_col - 1].attached_track->direction % 2 == 1)
                         || (cells[cur_row][left_edge_col - 1].attached_track->is_corner == 1 && cells[cur_row][left_edge_col - 1].attached_track->direction == RIGHT)
                         || (cells[cur_row][left_edge_col - 1].attached_track->is_corner == 1 && cells[cur_row][left_edge_col - 1].attached_track->direction == UP)))
                    {
                        if (check_cell_has_moving_mines(cur_row, left_edge_col - 1) == 0)
                        {
                            tmp->content.x = (left_edge_col - 1) * MAP_CELL_WIDTH + 2.5;
                            tmp->content.y = cur_row * MAP_CELL_HEIGHT + 2;
                            tmp->content.rotate();

                            //Make the mine invisible and immobolized for 2 seconds
                            tmp->content.control_disabled = 1;
                            QTimer* enableControlTimer = new QTimer(this);
                            enableControlTimer->setSingleShot(true);
                            connect(enableControlTimer, &QTimer::timeout, [=]() {
                                tmp->content.control_disabled = 0;
                                enableControlTimer->deleteLater();
                            });
                            enableControlTimer->start(processing_speed);
                        }
                    }
                }
                else if (cells[cur_row][right_edge_col].has_rotator == 1 && cells[cur_row][right_edge_col].attached_rotator->direction == RIGHT)
                {
                    if (cells[cur_row][right_edge_col + 1].has_track == 1 &&
                        ((cells[cur_row][right_edge_col + 1].attached_track->is_corner == 0 && cells[cur_row][right_edge_col + 1].attached_track->direction % 2 == 1)
                         || (cells[cur_row][right_edge_col + 1].attached_track->is_corner == 1 && cells[cur_row][right_edge_col + 1].attached_track->direction == LEFT)
                         || (cells[cur_row][right_edge_col + 1].attached_track->is_corner == 1 && cells[cur_row][right_edge_col + 1].attached_track->direction == DOWN)))
                    {
                        if (check_cell_has_moving_mines(cur_row, right_edge_col + 1) == 0)
                        {
                            tmp->content.x = (right_edge_col + 1) * MAP_CELL_WIDTH + 2.5;
                            tmp->content.y = cur_row * MAP_CELL_HEIGHT + 2;
                            tmp->content.rotate();

                            //Make the mine invisible and immobolized for 2 seconds
                            tmp->content.control_disabled = 1;
                            QTimer* enableControlTimer = new QTimer(this);
                            enableControlTimer->setSingleShot(true);
                            connect(enableControlTimer, &QTimer::timeout, [=]() {
                                tmp->content.control_disabled = 0;
                                enableControlTimer->deleteLater();
                            });
                            enableControlTimer->start(processing_speed);
                        }
                    }
                }
                tmp = tmp->next;
            }
        }
    }
}

void MainScene::draw_track()
{
    int cur_row = (mapFromGlobal(QCursor::pos()).y() - print_position_y) / (MAP_CELL_HEIGHT * scaleFactor);
    int cur_col = (mapFromGlobal(QCursor::pos()).x() - print_position_x) / (MAP_CELL_WIDTH * scaleFactor);

    if (cells[cur_row][cur_col].state == EMPTY_CELL || cells[cur_row][cur_col].state == DELIVERY_CENTER_OUTSKIRT_CELL)
    {
        //Note that the previous track MUST be vertical or horizontal

        //Current position is right below the previous track
        if (cur_row == last_paint_track_row + 1 && cur_col == last_paint_track_col)
        {
            if (cells[last_paint_track_row][last_paint_track_col].attached_track->direction % 2 == 1) //Previous track is horizontal
            {
                if (last_paint_track_col - 1 >= 0 && cells[last_paint_track_row][last_paint_track_col - 1].has_track == 1)
                {
                    //Turn the previous horizontal track into a downward corner track
                    cells[last_paint_track_row][last_paint_track_col].attached_track->switch_to_corner_direction(DOWN);
                }
                else if (last_paint_track_col + 1 <= NUM_COLS - 1 && cells[last_paint_track_row][last_paint_track_col + 1].has_track == 1)
                {
                    //Turn the previous horizontal track into a rightward corner track
                    cells[last_paint_track_row][last_paint_track_col].attached_track->switch_to_corner_direction(RIGHT);
                }
            }
            else //Previous track is vertical
            {
                //No need to do anything
            }
            cells[cur_row][cur_col].has_track = 1;
            track* new_track = new track(0, UP);
            cells[cur_row][cur_col].attached_track = new_track;
            last_paint_track_row = cur_row;
            last_paint_track_col = cur_col;
        }
        //Current position is right above the previous track
        else if (cur_row == last_paint_track_row - 1 && cur_col == last_paint_track_col)
        {
            if (cells[last_paint_track_row][last_paint_track_col].attached_track->direction % 2 == 1) //Previous track is horizontal
            {
                if (last_paint_track_col - 1 >= 0 && cells[last_paint_track_row][last_paint_track_col - 1].has_track == 1)
                {
                    //Turn the previous horizontal track into a leftward corner track
                    cells[last_paint_track_row][last_paint_track_col].attached_track->switch_to_corner_direction(LEFT);
                }
                else if (last_paint_track_col + 1 <= NUM_COLS - 1 && cells[last_paint_track_row][last_paint_track_col + 1].has_track == 1)
                {
                    //Turn the previous horizontal track into a upward corner track
                    cells[last_paint_track_row][last_paint_track_col].attached_track->switch_to_corner_direction(UP);
                }
            }
            else //Previous track is vertical
            {
                //No need to do anything
            }
            cells[cur_row][cur_col].has_track = 1;
            track* new_track = new track(0, UP);
            cells[cur_row][cur_col].attached_track = new_track;
            last_paint_track_row = cur_row;
            last_paint_track_col = cur_col;
        }
        //Current position is on the left of the previous track
        else if (cur_row == last_paint_track_row && cur_col == last_paint_track_col - 1)
        {
            if (cells[last_paint_track_row][last_paint_track_col].attached_track->direction % 2 == 0) //Previous track is vertical
            {
                if (last_paint_track_row - 1 >= 0 && cells[last_paint_track_row - 1][last_paint_track_col].has_track == 1)
                {
                    //Turn the previous horizontal track into a leftward corner track
                    cells[last_paint_track_row][last_paint_track_col].attached_track->switch_to_corner_direction(LEFT);
                }
                else if (last_paint_track_row + 1 <= NUM_ROWS - 1 && cells[last_paint_track_row + 1][last_paint_track_col].has_track == 1)
                {
                    //Turn the previous horizontal track into a downward corner track
                    cells[last_paint_track_row][last_paint_track_col].attached_track->switch_to_corner_direction(DOWN);
                }
            }
            else //Previous track is horizontal
            {
                //No need to do anything
            }
            cells[cur_row][cur_col].has_track = 1;
            track* new_track = new track(0, RIGHT);
            cells[cur_row][cur_col].attached_track = new_track;
            last_paint_track_row = cur_row;
            last_paint_track_col = cur_col;
        }
        //Current position is on the right of the previous track
        else if (cur_row == last_paint_track_row && cur_col == last_paint_track_col + 1)
        {
            if (cells[last_paint_track_row][last_paint_track_col].attached_track->direction % 2 == 0) //Previous track is vertical
            {
                if (last_paint_track_row - 1 >= 0 && cells[last_paint_track_row - 1][last_paint_track_col].has_track == 1)
                {
                    //Turn the previous horizontal track into a upward corner track
                    cells[last_paint_track_row][last_paint_track_col].attached_track->switch_to_corner_direction(UP);
                }
                else if (last_paint_track_row + 1 <= NUM_ROWS - 1 && cells[last_paint_track_row + 1][last_paint_track_col].has_track == 1)
                {
                    //Turn the previous horizontal track into a rightward corner track
                    cells[last_paint_track_row][last_paint_track_col].attached_track->switch_to_corner_direction(RIGHT);
                }
            }
            else //Previous track is horizontal
            {
                //No need to do anything
            }
            cells[cur_row][cur_col].has_track = 1;
            track* new_track = new track(0, RIGHT);
            cells[cur_row][cur_col].attached_track = new_track;
            last_paint_track_row = cur_row;
            last_paint_track_col = cur_col;
        }
    }
}

void MainScene::paint_map()
{

    QPainter painter(this);

/******************** Scaling ********************/

    //Resize map
    QSize map_size;
    if (map_mode == SMALLER_MODE)
    {
        map_size.setWidth(GAME_WIDTH);
        map_size.setHeight(GAME_HEIGHT);
    }
    else
    {
        map_size.setWidth(GAME_WIDTH + 5 * MAP_CELL_WIDTH);
        map_size.setHeight(GAME_HEIGHT + 4 * MAP_CELL_HEIGHT);
    }
    map_size *= scaleFactor;
    QPixmap print_map = game_map.map.scaled(map_size);

    //Resize delivery center
    QSize delivery_center_size;
    if (delivery_center_mode == SMALLER_MODE)
    {
        delivery_center_size.setWidth(DELIVERY_CENTER_WIDTH);
        delivery_center_size.setHeight(DELIVERY_CENTER_HEIGHT);
    }
    else
    {
        delivery_center_size.setWidth(DELIVERY_CENTER_WIDTH / 4 * 5);
        delivery_center_size.setHeight(DELIVERY_CENTER_HEIGHT / 4 * 5);
    }
    delivery_center_size *= scaleFactor;
    QPixmap print_delivery_center = game_map.delivery_center.scaled(delivery_center_size);
    int print_delivery_center_x = (DELIVERY_CENTER_INIT_X) * scaleFactor;
    int print_delivery_center_y = (DELIVERY_CENTER_INIT_Y) * scaleFactor;

    //Resize mines
    QSize mine_size(MAP_CELL_WIDTH, MAP_CELL_HEIGHT);
    mine_size *= scaleFactor;
    QPixmap print_mine_1 = game_map.mine_1.scaled(mine_size);
    QPixmap print_mine_2 = game_map.mine_2.scaled(mine_size);
    QPixmap print_mine_3 = game_map.mine_3.scaled(mine_size);
    QPixmap print_mine_4 = game_map.mine_4.scaled(mine_size);

/******************** Painting ********************/


    //Print the plain map
    painter.drawPixmap(0 + print_position_x, 0 + print_position_y, print_map);

    //Paint the mines
    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 0; j < NUM_COLS; j++)
        {
            if (cells[i][j].state >= 1 && cells[i][j].state <= MINE_TYPE_NUM)
            {
                switch(cells[i][j].state)
                {
                case 1: painter.drawPixmap(j * MAP_CELL_WIDTH * scaleFactor + print_position_x, i * MAP_CELL_HEIGHT * scaleFactor + print_position_y, print_mine_1); break;
                case 2: painter.drawPixmap(j * MAP_CELL_WIDTH * scaleFactor + print_position_x, i * MAP_CELL_HEIGHT * scaleFactor + print_position_y, print_mine_2); break;
                case 3: painter.drawPixmap(j * MAP_CELL_WIDTH * scaleFactor + print_position_x, i * MAP_CELL_HEIGHT * scaleFactor + print_position_y, print_mine_3); break;
                case 4: painter.drawPixmap(j * MAP_CELL_WIDTH * scaleFactor + print_position_x, i * MAP_CELL_HEIGHT * scaleFactor + print_position_y, print_mine_4); break;
                default: break;
                }
            }
            if (cells[i][j].has_digger == 1)
            {
                cells[i][j].attached_digger->paint(painter, j * MAP_CELL_WIDTH * scaleFactor + print_position_x, i * MAP_CELL_HEIGHT * scaleFactor + print_position_y, scaleFactor);
            }
            else if (cells[i][j].has_track == 1)
            {
                cells[i][j].attached_track->paint(painter, j * MAP_CELL_WIDTH * scaleFactor + print_position_x, i * MAP_CELL_HEIGHT * scaleFactor + print_position_y, scaleFactor);
            }
            else if (cells[i][j].state == CUTTER_IN_CELL && (cells[i][j].attached_cutter->direction == UP || cells[i][j].attached_cutter->direction == RIGHT))
            {
                cells[i][j].attached_cutter->paint(painter, j * MAP_CELL_WIDTH * scaleFactor + print_position_x, i * MAP_CELL_HEIGHT * scaleFactor + print_position_y, scaleFactor);
            }
            else if (cells[i][j].state == CUTTER_TRASH_CELL && (cells[i][j].attached_cutter->direction == DOWN || cells[i][j].attached_cutter->direction == LEFT))
            {
                cells[i][j].attached_cutter->paint(painter, j * MAP_CELL_WIDTH * scaleFactor + print_position_x, i * MAP_CELL_HEIGHT * scaleFactor + print_position_y, scaleFactor);
            }
            else if (cells[i][j].has_trash_can == 1)
            {
                cells[i][j].attached_trash_can->paint(painter, j * MAP_CELL_WIDTH * scaleFactor + print_position_x, i * MAP_CELL_HEIGHT * scaleFactor + print_position_y, scaleFactor);
            }
            else if (cells[i][j].has_rotator == 1)
            {
                cells[i][j].attached_rotator->paint(painter, j * MAP_CELL_WIDTH * scaleFactor + print_position_x, i * MAP_CELL_HEIGHT * scaleFactor + print_position_y, scaleFactor);
            }
        }
    }

    //Paint the moving mines
    for (int i = 0; i < MINE_TYPE_NUM; i++)
    {
        Node* tmp = head[i];
        while (tmp != NULL)
        {
            if (tmp->content.control_disabled == 1)
            {
                tmp = tmp->next;
                continue;
            }
            tmp->content.paint(painter, (tmp->content.x) * scaleFactor + print_position_x, (tmp->content.y) * scaleFactor + print_position_y, scaleFactor);
            tmp = tmp->next;
        }
    }


    if (mouse_state == DRAWING_TRACK)
    {
        draw_track();
    }

    //Paint the delivery center
    painter.drawPixmap(print_delivery_center_x + print_position_x, print_delivery_center_y + print_position_y, print_delivery_center);

    //Paint the mission guide in the delivery center
    QPixmap deliver(DELIVER_PATH);
    QSize deliver_paint_size(2 * MAP_CELL_WIDTH, MAP_CELL_HEIGHT);
    deliver_paint_size *= scaleFactor;
    deliver = deliver.scaled(deliver_paint_size);
    painter.drawPixmap((DELIVERY_CENTER_INIT_X + 1.75 * MAP_CELL_WIDTH) * scaleFactor + print_position_x, (DELIVERY_CENTER_INIT_Y + 0.75 * MAP_CELL_HEIGHT) * scaleFactor + print_position_y, deliver);

    QPixmap mission_stage;
    QPixmap goal_mine;
    QPixmap goal_num;

    QPixmap num1;
    QPixmap num2;
    if (has_collect_mines >= 10)
    {
        int n1 = has_collect_mines / 10;
        if (n1 == 1) num1.load(NUM_1_PATH);
        else if (n1 == 2) num1.load(NUM_2_PATH);
        else if (n1 == 3) num1.load(NUM_3_PATH);
        else if (n1 == 4) num1.load(NUM_4_PATH);
        else if (n1 == 5) num1.load(NUM_5_PATH);
        else if (n1 == 6) num1.load(NUM_6_PATH);
        else if (n1 == 7) num1.load(NUM_7_PATH);
        else if (n1 == 8) num1.load(NUM_8_PATH);
        else if (n1 == 9) num1.load(NUM_9_PATH);
        QSize num_paint_size(0.5 * MAP_CELL_WIDTH, 0.8 * MAP_CELL_HEIGHT);
        num_paint_size *= scaleFactor;
        num1 = num1.scaled(num_paint_size);
        painter.drawPixmap((DELIVERY_CENTER_INIT_X + 2.1 * MAP_CELL_WIDTH) * scaleFactor + print_position_x, (DELIVERY_CENTER_INIT_Y + 2.2 * MAP_CELL_HEIGHT) * scaleFactor + print_position_y, num1);
    }
    int n2 = has_collect_mines % 10;
    if (n2 == 0) num2.load(NUM_0_PATH);
    else if (n2 == 1) num2.load(NUM_1_PATH);
    else if (n2 == 2) num2.load(NUM_2_PATH);
    else if (n2 == 3) num2.load(NUM_3_PATH);
    else if (n2 == 4) num2.load(NUM_4_PATH);
    else if (n2 == 5) num2.load(NUM_5_PATH);
    else if (n2 == 6) num2.load(NUM_6_PATH);
    else if (n2 == 7) num2.load(NUM_7_PATH);
    else if (n2 == 8) num2.load(NUM_8_PATH);
    else if (n2 == 9) num2.load(NUM_9_PATH);
    QSize num_paint_size(0.5 * MAP_CELL_WIDTH, 0.8 * MAP_CELL_HEIGHT);
    num_paint_size *= scaleFactor;
    num2 = num2.scaled(num_paint_size);
    painter.drawPixmap((DELIVERY_CENTER_INIT_X + 2.5 * MAP_CELL_WIDTH) * scaleFactor + print_position_x, (DELIVERY_CENTER_INIT_Y + 2.2 * MAP_CELL_HEIGHT) * scaleFactor + print_position_y, num2);


    if (cur_mission_stage == 1)
    {
        mission_stage.load(MISSION_STAGE_1_PATH);
        goal_mine.load(MOVING_MINE_3_PATH);
        goal_num.load(GOAL_1_PATH);
    }
    else if (cur_mission_stage == 2)
    {
        mission_stage.load(MISSION_STAGE_2_PATH);
        goal_mine.load(MOVING_MINE_4_PATH);
        goal_num.load(GOAL_2_AND_4_PATH);
    }
    else if (cur_mission_stage == 3)
    {
        mission_stage.load(MISSION_STAGE_3_PATH);
        goal_mine.load(CUTTED_MOVING_MINE_1_PATH);
        goal_num.load(GOAL_3_PATH);
    }
    else if (cur_mission_stage == 4)
    {
        mission_stage.load(MISSION_STAGE_4_PATH);
        goal_mine.load(CUTTED_MOVING_MINE_2_PATH);
        goal_mine = goal_mine.transformed(QTransform().rotate(90));
        goal_num.load(GOAL_2_AND_4_PATH);
    }

    QSize goal_mine_paint_size(MAP_CELL_WIDTH, MAP_CELL_HEIGHT);
    goal_mine_paint_size *= scaleFactor;
    goal_mine = goal_mine.scaled(goal_mine_paint_size);
    painter.drawPixmap((DELIVERY_CENTER_INIT_X + 1.0 * MAP_CELL_WIDTH) * scaleFactor + print_position_x, (DELIVERY_CENTER_INIT_Y + 2.2 * MAP_CELL_HEIGHT) * scaleFactor + print_position_y, goal_mine);

    QSize goal_num_paint_size(1.25 * MAP_CELL_WIDTH, 0.625 * MAP_CELL_HEIGHT);
    goal_num_paint_size *= scaleFactor;
    goal_num = goal_num.scaled(goal_num_paint_size);
    painter.drawPixmap((DELIVERY_CENTER_INIT_X + 2.1 * MAP_CELL_WIDTH) * scaleFactor + print_position_x, (DELIVERY_CENTER_INIT_Y + 3.2 * MAP_CELL_HEIGHT) * scaleFactor + print_position_y, goal_num);

    QSize mission_stage_paint_size(1.2 * MAP_CELL_WIDTH, 1.6 * MAP_CELL_HEIGHT);
    mission_stage_paint_size *= scaleFactor;
    mission_stage = mission_stage.scaled(mission_stage_paint_size);
    painter.drawPixmap((DELIVERY_CENTER_INIT_X + 0.5 * MAP_CELL_WIDTH) * scaleFactor + print_position_x, (DELIVERY_CENTER_INIT_Y + 0.41 * MAP_CELL_HEIGHT) * scaleFactor + print_position_y, mission_stage);

    //Paint the save icon
    QPixmap save_icon(SAVE_ICON_PATH);
    QSize save_paint_size(2 * MAP_CELL_WIDTH, 2 * MAP_CELL_HEIGHT);
    save_icon = save_icon.scaled(save_paint_size);
    painter.drawPixmap(10, 10, save_icon);

    //Paint the home icon
    QPixmap home_icon(HOME_ICON_PATH);
    QSize home_paint_size(2 * MAP_CELL_WIDTH, 2 * MAP_CELL_HEIGHT);
    home_icon = home_icon.scaled(home_paint_size);
    painter.drawPixmap(60, 10, home_icon);

    //Paint the handle bar
    painter.drawPixmap((GAME_WIDTH - HANDLE_BAR_LENGTH) / 2, GAME_HEIGHT * 6 / 7, game_map.handle_bar);

    //Paint the coin number on the upper right of the map
    QPixmap coin(COIN_PATH);
    QSize coin_paint_size(1.5 * MAP_CELL_WIDTH, 1.5 * MAP_CELL_HEIGHT);
    coin = coin.scaled(coin_paint_size);
    painter.drawPixmap(GAME_WIDTH - 120, 11, coin);

    int tmp_coin_num = coins;
    if (coins == 0)
    {
        QPixmap num(NUM_0_PATH);
        QSize num_paint_size(1 * MAP_CELL_WIDTH, 1.6 * MAP_CELL_HEIGHT);
        num = num.scaled(num_paint_size);
        painter.drawPixmap(GAME_WIDTH - 30, 10, num);
    }
    else
    {
        int paint_offset = 0;
        while (tmp_coin_num > 0)
        {
            int tmp_num = tmp_coin_num % 10;
            tmp_coin_num /= 10;
            QPixmap num;
            if (tmp_num == 0) num.load(NUM_0_PATH);
            else if (tmp_num == 1) num.load(NUM_1_PATH);
            else if (tmp_num == 2) num.load(NUM_2_PATH);
            else if (tmp_num == 3) num.load(NUM_3_PATH);
            else if (tmp_num == 4) num.load(NUM_4_PATH);
            else if (tmp_num == 5) num.load(NUM_5_PATH);
            else if (tmp_num == 6) num.load(NUM_6_PATH);
            else if (tmp_num == 7) num.load(NUM_7_PATH);
            else if (tmp_num == 8) num.load(NUM_8_PATH);
            else if (tmp_num == 9) num.load(NUM_9_PATH);
            QSize num_paint_size(1 * MAP_CELL_WIDTH, 1.6 * MAP_CELL_HEIGHT);
            num = num.scaled(num_paint_size);
            painter.drawPixmap(GAME_WIDTH - 30 - paint_offset, 10, num);
            paint_offset += 20;
        }
    }


    //If the mouse state is PUTTING something, then paint the thing around the mouse
    if (mouse_state != FREE)
    {
        int mouse_x = mapFromGlobal(QCursor::pos()).x();
        int mouse_y = mapFromGlobal(QCursor::pos()).y();
        if (mouse_state == PUTTING_DIGGER)
        {
            digger digger_to_be_put(to_be_put_direction);
            digger_to_be_put.paint(painter, mouse_x - MAP_CELL_WIDTH * scaleFactor / 2, mouse_y - MAP_CELL_HEIGHT * scaleFactor / 2, scaleFactor);
        }
        else if (mouse_state == PUTTING_TRACK)
        {
            track track_to_be_put(0, to_be_put_direction);
            track_to_be_put.paint(painter, mouse_x - MAP_CELL_WIDTH * scaleFactor / 2, mouse_y - MAP_CELL_HEIGHT * scaleFactor / 2, scaleFactor);
        }
        else if (mouse_state == PUTTING_CUTTER)
        {
            cutter cutter_to_be_put(to_be_put_direction);
            cutter_to_be_put.paint(painter, mouse_x - MAP_CELL_WIDTH * scaleFactor / 2, mouse_y - MAP_CELL_HEIGHT * scaleFactor / 2, scaleFactor);
        }
        else if (mouse_state == PUTTING_TRASH_CAN)
        {
            trash_can trash_can_to_be_put;
            trash_can_to_be_put.paint(painter, mouse_x - MAP_CELL_WIDTH * scaleFactor / 2, mouse_y - MAP_CELL_HEIGHT * scaleFactor / 2, scaleFactor);
        }
        else if (mouse_state == PUTTING_ROTATOR)
        {
            rotator rotator_to_be_put(to_be_put_direction);
            rotator_to_be_put.paint(painter, mouse_x - MAP_CELL_WIDTH * scaleFactor / 2, mouse_y - MAP_CELL_HEIGHT * scaleFactor / 2, scaleFactor);
        }
    }

}

void MainScene::paintEvent(QPaintEvent*)
{
    if (game_state == MAIN_MENU || game_state == SHOP)
    {
        QPainter main_menu_painter(this);

        QPixmap background(BACKGROUND_PATH);
        main_menu_painter.drawPixmap(0, 0, background);

        QPixmap new_game(NEW_GAME_PATH);
        new_game = new_game.scaled(QSize(16 * MAP_CELL_WIDTH, 6 * MAP_CELL_HEIGHT));
        main_menu_painter.drawPixmap(0.5 * (GAME_WIDTH - 16 * MAP_CELL_WIDTH), 0.1 * GAME_HEIGHT, new_game);

        QPixmap load(LOAD_PATH);
        load = load.scaled(QSize(16 * MAP_CELL_WIDTH, 7.0 * MAP_CELL_HEIGHT));
        main_menu_painter.drawPixmap(0.5 * (GAME_WIDTH - 16 * MAP_CELL_WIDTH), 0.3 * GAME_HEIGHT, load);

        QPixmap shop(SHOP_PATH);
        shop = shop.scaled(QSize(16 * MAP_CELL_WIDTH, 6 * MAP_CELL_HEIGHT));
        main_menu_painter.drawPixmap(0.5 * (GAME_WIDTH - 16 * MAP_CELL_WIDTH), 0.5 * GAME_HEIGHT, shop);

        QPixmap quit(QUIT_PATH);
        quit = quit.scaled(QSize(16 * MAP_CELL_WIDTH, 6.5 * MAP_CELL_HEIGHT));
        main_menu_painter.drawPixmap(0.5 * (GAME_WIDTH - 16 * MAP_CELL_WIDTH), 0.7 * GAME_HEIGHT, quit);

        //Paint the coin number on the upper right of the map
        QPixmap coin(COIN_PATH);
        QSize coin_paint_size(1.5 * MAP_CELL_WIDTH, 1.5 * MAP_CELL_HEIGHT);
        coin = coin.scaled(coin_paint_size);
        main_menu_painter.drawPixmap(GAME_WIDTH - 120, 11, coin);

        int tmp_coin_num = coins;
        if (coins == 0)
        {
            QPixmap num(NUM_0_PATH);
            QSize num_paint_size(1 * MAP_CELL_WIDTH, 1.6 * MAP_CELL_HEIGHT);
            num = num.scaled(num_paint_size);
            main_menu_painter.drawPixmap(GAME_WIDTH - 30, 10, num);
        }
        else
        {
            int paint_offset = 0;
            while (tmp_coin_num > 0)
            {
                int tmp_num = tmp_coin_num % 10;
                tmp_coin_num /= 10;
                QPixmap num;
                if (tmp_num == 0) num.load(NUM_0_PATH);
                else if (tmp_num == 1) num.load(NUM_1_PATH);
                else if (tmp_num == 2) num.load(NUM_2_PATH);
                else if (tmp_num == 3) num.load(NUM_3_PATH);
                else if (tmp_num == 4) num.load(NUM_4_PATH);
                else if (tmp_num == 5) num.load(NUM_5_PATH);
                else if (tmp_num == 6) num.load(NUM_6_PATH);
                else if (tmp_num == 7) num.load(NUM_7_PATH);
                else if (tmp_num == 8) num.load(NUM_8_PATH);
                else if (tmp_num == 9) num.load(NUM_9_PATH);
                QSize num_paint_size(1 * MAP_CELL_WIDTH, 1.6 * MAP_CELL_HEIGHT);
                num = num.scaled(num_paint_size);
                main_menu_painter.drawPixmap(GAME_WIDTH - 30 - paint_offset, 10, num);
                paint_offset += 20;
            }
        }
    }
    if (game_state == START || game_state == IN_GAME_UPGRADE)
    {
        //Avoid out-of-boundary painting
        if (print_position_x > 0) print_position_x = 0;
        if (print_position_y > 0) print_position_y = 0;
        double upper_x_bound, upper_y_bound;
        if (map_mode == SMALLER_MODE)
        {
            upper_x_bound = -(GAME_WIDTH * scaleFactor - GAME_WIDTH);
            upper_y_bound = -(GAME_HEIGHT * scaleFactor - GAME_HEIGHT);
        }
        else
        {
            upper_x_bound = -((GAME_WIDTH + 5 * MAP_CELL_WIDTH) * scaleFactor - (GAME_WIDTH));
            upper_y_bound = -((GAME_HEIGHT + 4 * MAP_CELL_HEIGHT) * scaleFactor - (GAME_HEIGHT));
        }
        if (print_position_x < 0 && print_position_x < upper_x_bound) print_position_x = upper_x_bound;
        if (print_position_y < 0 && print_position_y < upper_y_bound) print_position_y = upper_y_bound;

        paint_map();
    }
    if (game_state == IN_GAME_UPGRADE)
    {
        QPainter upgrade_painter(this);
        QPixmap in_game_upgrade(IN_GAME_UPGRADE_PATH);
        QSize in_game_upgrade_paint_size(in_game_upgrade.width() / 2, in_game_upgrade.height() / 2);
        in_game_upgrade = in_game_upgrade.scaled(in_game_upgrade_paint_size);
        upgrade_painter.drawPixmap(0.5 * (GAME_WIDTH - in_game_upgrade.width()), 0.5 * (GAME_HEIGHT - in_game_upgrade.height()), in_game_upgrade);
    }
    if (game_state == SHOP)
    {
        QPainter shop_painter(this);
        QPixmap in_shop_buying(IN_SHOP_BUYING_PATH);
        QSize in_shop_buying_paint_size(in_shop_buying.width() * 0.8, in_shop_buying.height() * 0.8);
        in_shop_buying = in_shop_buying.scaled(in_shop_buying_paint_size);
        shop_painter.drawPixmap(0.5 * (GAME_WIDTH - in_shop_buying.width()), 0.5 * (GAME_HEIGHT - in_shop_buying.height()), in_shop_buying);
    }
}

void MainScene::wheelEvent(QWheelEvent* event)
{
    if (game_state == START)
    {
        int delta = event->angleDelta().y();
        if (delta > 0)
        {
            scaleFactor *= 1.1;
        }
        else
        {
            scaleFactor /= 1.1;
            if (scaleFactor < 1) scaleFactor = 1;
        }
        event->accept();
    }
}

void MainScene::mousePressEvent(QMouseEvent* event)
{
    if (game_state == MAIN_MENU)
    {
        if (event->button() == Qt::LeftButton)
        {
            //Mouse pressing the start button
            if (event->x() > 240 && event->x() < 541 && event->y() > 74 && event->y() < 169)
            {
                game_state = START;
                if (map_mode == SMALLER_MODE)
                {
                    NUM_ROWS = DEFAULT_SMALLER_MAP_NUM_ROWS;
                    NUM_COLS = DEFAULT_SMALLER_MAP_NUM_COLS;
                }
                else
                {
                    NUM_ROWS = DEFAULT_LARGER_MAP_NUM_ROWS;
                    NUM_COLS = DEFAULT_LARGER_MAP_NUM_COLS;
                }
                init_scene();
                start_game();
            }
            //Mouse pressing the load button
            else if (event->x() > 240 && event->x() < 541 && event->y() > 215 && event->y() < 314)
            {
                QString directory = ":/save";
                if (!directory.isEmpty())
                {
                    QString file_path = QFileDialog::getOpenFileName(this, "选择要读入的存档文件", directory, "存档文件 (*.txt)");
                    if (!file_path.isEmpty())
                    {
                        for (int i = 0; i < MINE_TYPE_NUM; i++)
                        {
                            head[i] = NULL;
                        }

                        //Read the delivery center mode and map mode first in order to execute init_cells()
                        QFile file(file_path);
                        file.open(QIODevice::ReadOnly);
                        QTextStream in_file(&file);
                        in_file.setIntegerBase(10);
                        int tmp_coin, tmp_get_coins_per_collection, tmp_mine_block_num;
                        in_file >> tmp_coin >> tmp_get_coins_per_collection >> delivery_center_mode >> tmp_mine_block_num >> map_mode;
                        file.close();
                        if (map_mode == SMALLER_MODE)
                        {
                            NUM_ROWS = DEFAULT_SMALLER_MAP_NUM_ROWS;
                            NUM_COLS = DEFAULT_SMALLER_MAP_NUM_COLS;
                        }
                        else
                        {
                            NUM_ROWS = DEFAULT_LARGER_MAP_NUM_ROWS;
                            NUM_COLS = DEFAULT_LARGER_MAP_NUM_COLS;
                        }
                        init_cells();
                        load(file_path);


                        overall_timer.setInterval(GAME_RATE);
                        dig_timer.setInterval(digging_speed);

                        game_state = START;
                        init_paths();
                        start_game();

                    }
                }
            }
            //Mouse pressing the shop button
            else if (event->x() > 240 && event->x() < 541 && event->y() > 359 && event->y() < 455)
            {
                game_state = SHOP;
            }
            //Mouse pressing the quit button
            else if (event->x() > 240 && event->x() < 541 && event->y() > 500 && event->y() < 601)
            {
                this->close();
            }
        }
    }
    else if (game_state == START)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (mouse_state == FREE)
            {
                //Mouse pressing the digger icon
                if (event->x() > 295 && event->x() < 335 && event->y() > 624 && event->y() < 670)
                {
                    mouse_state = PUTTING_DIGGER;
                }
                //Mouse pressing the track icon
                else if (event->x() > 222 && event->x() < 258 && event->y() > 626 && event->y() < 671)
                {
                    mouse_state = PUTTING_TRACK;
                }
                //Mouse pressing the cutter icon
                else if (event->x() > 370 && event->x() < 410 && event->y() > 624 && event->y() < 669)
                {
                    mouse_state = PUTTING_CUTTER;
                }
                //Mouse pressing the trashcan icon
                else if (event->x() > 517 && event->x() < 551 && event->y() > 623 && event->y() < 674)
                {
                    mouse_state = PUTTING_TRASH_CAN;
                }
                //Mouse pressing the rotator icon
                else if (event->x() > 439 && event->x() < 486 && event->y() > 624 && event->y() < 670)
                {
                    mouse_state = PUTTING_ROTATOR;
                }
                //Mouse pressing the save button
                else if (event->x() > 10 && event->x() < 47 && event->y() > 10 && event->y() < 44)
                {
                    game_state = IN_GAME_MENU;

                    QDir *dir = new QDir(QDir::currentPath());
                    dir->cdUp();
                    QString directory = dir->absolutePath() + "/Factorio/save";
                    delete dir;

                    if (!directory.isEmpty())
                    {

                        QString fileName = QInputDialog::getText(this, "输入存档文件名", "请输入存档文件名:");

                        if (!fileName.isEmpty())
                        {
                            QString filePath = QDir(directory).filePath(fileName + ".txt");
                            save(filePath);
                        }
                    }

                    //Rewrite the config.txt
                    dir = new QDir(QDir::currentPath());
                    dir->cdUp();
                    directory = dir->absolutePath() + "/Factorio/config.txt";
                    delete dir;

                    QFile file(directory);
                    file.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream out_file(&file);
                    out_file.setIntegerBase(10);
                    out_file << coins << " " << delivery_center_mode << " " << get_coins_per_collection << " " << mine_block_num << " " << map_mode << " ";
                    file.close();

                    game_state = START;
                }
                //Mouse pressing the home button
                else if (event->x() > 59 && event->x() < 98 && event->y() > 10 && event->y() < 45)
                {

                    for (int i = 0; i < MINE_TYPE_NUM; i++)
                    {
                        while (head[i] != NULL)
                        {
                            Node* tmp = head[i];
                            head[i] = head[i]->next;
                            delete tmp;
                        }
                    }

                    for (int i = 0; i < NUM_ROWS; i++)
                    {
                        for (int j = 0; j < NUM_COLS; j++)
                        {
                            if (cells[i][j].has_digger == 1) delete cells[i][j].attached_digger;
                            if (cells[i][j].has_cutter == 1 && cells[i][j].state == CUTTER_IN_CELL) delete cells[i][j].attached_cutter;
                            if (cells[i][j].has_rotator == 1) delete cells[i][j].attached_rotator;
                            if (cells[i][j].has_track == 1) delete cells[i][j].attached_track;
                            if (cells[i][j].has_trash_can == 1) delete cells[i][j].attached_trash_can;
                            cells[i][j].has_cutter = 0;
                            cells[i][j].has_digger = 0;
                            cells[i][j].has_rotator = 0;
                            cells[i][j].has_track = 0;
                            cells[i][j].has_trash_can = 0;
                            cells[i][j].attached_cutter = NULL;
                            cells[i][j].attached_digger = NULL;
                            cells[i][j].attached_rotator = NULL;
                            cells[i][j].attached_track = NULL;
                            cells[i][j].attached_trash_can = NULL;
                        }
                    }

                    scaleFactor = 1.0;
                    print_position_x = 0, print_position_y = 0;
                    cur_mission_stage = 1;
                    has_collect_mines = 0;
                    game_state = MAIN_MENU;
                    digging_speed = DEFAULT_DIGGING_SPEED;
                    mine_moving_speed = DEFALUT_MOVING_SPEED;
                    processing_speed = DEFAULT_PROCESSING_SPEED;
                    main_menu();
                }
                else
                {
                    last_mouse_position = event->globalPos();
                }
            }
            else
            {
                int cur_row = (event->y() - print_position_y) / (MAP_CELL_HEIGHT * scaleFactor);
                int cur_col = (event->x() - print_position_x) / (MAP_CELL_WIDTH * scaleFactor);
                if (cells[cur_row][cur_col].state != DELIVERY_CENTER_CELL)
                {
                    if (mouse_state == PUTTING_DIGGER)
                    {
                        if (cells[cur_row][cur_col].has_cutter == 0
                            && cells[cur_row][cur_col].has_digger == 0
                            && cells[cur_row][cur_col].has_track == 0
                            && cells[cur_row][cur_col].has_rotator == 0
                            && cells[cur_row][cur_col].has_trash_can == 0)
                        {
                            cells[cur_row][cur_col].has_digger = 1;
                            digger* new_digger = new digger(to_be_put_direction);
                            cells[cur_row][cur_col].attached_digger = new_digger;
                            to_be_put_direction = UP;
                            mouse_state = FREE;
                        }
                    }
                    else if (mouse_state == PUTTING_TRACK)
                    {
                        if (cells[cur_row][cur_col].has_cutter == 0
                            && cells[cur_row][cur_col].has_digger == 0
                            && cells[cur_row][cur_col].has_track == 0
                            && cells[cur_row][cur_col].has_rotator == 0
                            && cells[cur_row][cur_col].has_trash_can == 0
                            && cells[cur_row][cur_col].state < 1)
                        {
                            cells[cur_row][cur_col].has_track = 1;
                            track* new_track = new track(0, to_be_put_direction);
                            cells[cur_row][cur_col].attached_track = new_track;
                            to_be_put_direction = UP;
                            mouse_state = DRAWING_TRACK;
                            last_paint_track_row = cur_row;
                            last_paint_track_col = cur_col;
                        }
                    }
                    else if (mouse_state == PUTTING_CUTTER)
                    {
                        if (to_be_put_direction == UP && cur_col + 1 < NUM_COLS)
                        {
                            if (cells[cur_row][cur_col].has_cutter == 0
                                && cells[cur_row][cur_col].has_digger == 0
                                && cells[cur_row][cur_col].has_track == 0
                                && cells[cur_row][cur_col].has_rotator == 0
                                && cells[cur_row][cur_col].has_trash_can == 0
                                && cells[cur_row][cur_col].state < 1
                                && cells[cur_row][cur_col + 1].has_cutter == 0
                                && cells[cur_row][cur_col + 1].has_digger == 0
                                && cells[cur_row][cur_col + 1].has_track == 0
                                && cells[cur_row][cur_col + 1].has_rotator == 0
                                && cells[cur_row][cur_col + 1].has_trash_can == 0
                                && cells[cur_row][cur_col + 1].state < 1)
                            {
                                cells[cur_row][cur_col].has_cutter = 1;
                                cells[cur_row][cur_col + 1].has_cutter = 1;
                                cutter* new_cutter = new cutter(to_be_put_direction);
                                cells[cur_row][cur_col].attached_cutter = new_cutter;
                                cells[cur_row][cur_col + 1].attached_cutter = new_cutter;
                                to_be_put_direction = UP;
                                mouse_state = FREE;
                                cells[cur_row][cur_col].state = CUTTER_IN_CELL;
                                cells[cur_row][cur_col + 1].state = CUTTER_TRASH_CELL;
                            }
                        }
                        else if (to_be_put_direction == RIGHT && cur_row + 1 < NUM_ROWS)
                        {
                            if (cells[cur_row][cur_col].has_cutter == 0
                                && cells[cur_row][cur_col].has_digger == 0
                                && cells[cur_row][cur_col].has_track == 0
                                && cells[cur_row][cur_col].has_rotator == 0
                                && cells[cur_row][cur_col].has_trash_can == 0
                                && cells[cur_row][cur_col].state < 1
                                && cells[cur_row + 1][cur_col].has_cutter == 0
                                && cells[cur_row + 1][cur_col].has_digger == 0
                                && cells[cur_row + 1][cur_col].has_track == 0
                                && cells[cur_row + 1][cur_col].has_rotator == 0
                                && cells[cur_row + 1][cur_col].has_trash_can == 0
                                && cells[cur_row + 1][cur_col].state < 1)
                            {
                                cells[cur_row][cur_col].has_cutter = 1;
                                cells[cur_row + 1][cur_col].has_cutter = 1;
                                cutter* new_cutter = new cutter(to_be_put_direction);
                                cells[cur_row][cur_col].attached_cutter = new_cutter;
                                cells[cur_row + 1][cur_col].attached_cutter = new_cutter;
                                to_be_put_direction = UP;
                                mouse_state = FREE;
                                cells[cur_row][cur_col].state = CUTTER_IN_CELL;
                                cells[cur_row + 1][cur_col].state = CUTTER_TRASH_CELL;
                            }
                        }
                        else if (to_be_put_direction == DOWN && cur_col + 1 < NUM_COLS)
                        {
                            if (cells[cur_row][cur_col].has_cutter == 0
                                && cells[cur_row][cur_col].has_digger == 0
                                && cells[cur_row][cur_col].has_track == 0
                                && cells[cur_row][cur_col].has_rotator == 0
                                && cells[cur_row][cur_col].has_trash_can == 0
                                && cells[cur_row][cur_col].state < 1
                                && cells[cur_row][cur_col + 1].has_cutter == 0
                                && cells[cur_row][cur_col + 1].has_digger == 0
                                && cells[cur_row][cur_col + 1].has_track == 0
                                && cells[cur_row][cur_col + 1].has_rotator == 0
                                && cells[cur_row][cur_col + 1].has_trash_can == 0
                                && cells[cur_row][cur_col + 1].state < 1)
                            {
                                cells[cur_row][cur_col].has_cutter = 1;
                                cells[cur_row][cur_col + 1].has_cutter = 1;
                                cutter* new_cutter = new cutter(to_be_put_direction);
                                cells[cur_row][cur_col].attached_cutter = new_cutter;
                                cells[cur_row][cur_col + 1].attached_cutter = new_cutter;
                                to_be_put_direction = UP;
                                mouse_state = FREE;
                                cells[cur_row][cur_col].state = CUTTER_TRASH_CELL;
                                cells[cur_row][cur_col + 1].state = CUTTER_IN_CELL;
                            }
                        }
                        else if (to_be_put_direction == LEFT && cur_row + 1 < NUM_ROWS)
                        {
                            if (cells[cur_row][cur_col].has_cutter == 0
                                && cells[cur_row][cur_col].has_digger == 0
                                && cells[cur_row][cur_col].has_track == 0
                                && cells[cur_row][cur_col].has_rotator == 0
                                && cells[cur_row][cur_col].has_trash_can == 0
                                && cells[cur_row][cur_col].state < 1
                                && cells[cur_row + 1][cur_col].has_cutter == 0
                                && cells[cur_row + 1][cur_col].has_digger == 0
                                && cells[cur_row + 1][cur_col].has_track == 0
                                && cells[cur_row + 1][cur_col].has_rotator == 0
                                && cells[cur_row + 1][cur_col].has_trash_can == 0
                                && cells[cur_row + 1][cur_col].state < 1)
                            {
                                cells[cur_row][cur_col].has_cutter = 1;
                                cells[cur_row + 1][cur_col].has_cutter = 1;
                                cutter* new_cutter = new cutter(to_be_put_direction);
                                cells[cur_row][cur_col].attached_cutter = new_cutter;
                                cells[cur_row + 1][cur_col].attached_cutter = new_cutter;
                                to_be_put_direction = UP;
                                mouse_state = FREE;
                                cells[cur_row][cur_col].state = CUTTER_TRASH_CELL;
                                cells[cur_row + 1][cur_col].state = CUTTER_IN_CELL;
                            }
                        }
                    }
                    else if (mouse_state == PUTTING_TRASH_CAN)
                    {
                        if (cells[cur_row][cur_col].has_cutter == 0
                            && cells[cur_row][cur_col].has_digger == 0
                            && cells[cur_row][cur_col].has_track == 0
                            && cells[cur_row][cur_col].has_rotator == 0
                            && cells[cur_row][cur_col].has_trash_can == 0
                            && cells[cur_row][cur_col].state < 1)
                        {
                            cells[cur_row][cur_col].has_trash_can = 1;
                            trash_can* new_trash_can = new trash_can();
                            cells[cur_row][cur_col].attached_trash_can = new_trash_can;
                            to_be_put_direction = UP;
                            mouse_state = FREE;
                        }
                    }
                    else if (mouse_state == PUTTING_ROTATOR)
                    {
                        if (cells[cur_row][cur_col].has_cutter == 0
                            && cells[cur_row][cur_col].has_digger == 0
                            && cells[cur_row][cur_col].has_track == 0
                            && cells[cur_row][cur_col].has_rotator == 0
                            && cells[cur_row][cur_col].has_trash_can == 0
                            && cells[cur_row][cur_col].state < 1)
                        {
                            cells[cur_row][cur_col].has_rotator = 1;
                            rotator* new_rotator = new rotator(to_be_put_direction);
                            cells[cur_row][cur_col].attached_rotator = new_rotator;
                            to_be_put_direction = UP;
                            mouse_state = FREE;
                        }
                    }
                }

            }


        }
        if (event->button() == Qt::RightButton)
        {
            if (mouse_state == FREE)
            {
                int cur_row = (event->y() - print_position_y) / (MAP_CELL_HEIGHT * scaleFactor);
                int cur_col = (event->x() - print_position_x) / (MAP_CELL_WIDTH * scaleFactor);
                for (int i = 0; i < MINE_TYPE_NUM; i++)
                {
                    Node* tmp = head[i];
                    Node* last;
                    while (tmp != NULL)
                    {
                        int tmp_cur_row = (tmp->content.y + MAP_CELL_HEIGHT * 0.4) / MAP_CELL_HEIGHT;
                        int tmp_cur_col = (tmp->content.x + MAP_CELL_WIDTH * 0.4) / MAP_CELL_WIDTH;
                        if (tmp_cur_row == cur_row && tmp_cur_col == cur_col)
                        {
                            count_mines[tmp->content.type - 1]--;
                            if (tmp == head[i])
                            {
                                Node* del = tmp;
                                head[i] = tmp->next;
                                tmp = tmp->next;
                                delete del;
                                del = NULL;
                            }
                            else
                            {
                                Node* del = tmp;
                                tmp = tmp->next;
                                last->next = tmp;
                                delete del;
                                del = NULL;
                            }
                        }
                        else
                        {
                            last = tmp;
                            tmp = tmp->next;
                        }
                    }

                }
                if (cells[cur_row][cur_col].has_cutter == 1)
                {
                    if (cells[cur_row][cur_col].attached_cutter->direction == UP)
                    {
                        if (cells[cur_row][cur_col].state == CUTTER_IN_CELL)
                        {
                            cells[cur_row][cur_col + 1].has_cutter = 0;
                            cells[cur_row][cur_col + 1].state = EMPTY_CELL;
                        }
                        else if (cells[cur_row][cur_col].state == CUTTER_TRASH_CELL)
                        {
                            cells[cur_row][cur_col - 1].has_cutter = 0;
                            cells[cur_row][cur_col - 1].state = EMPTY_CELL;
                        }
                    }
                    else if (cells[cur_row][cur_col].attached_cutter->direction == RIGHT)
                    {
                        if (cells[cur_row][cur_col].state == CUTTER_IN_CELL)
                        {
                            cells[cur_row + 1][cur_col].has_cutter = 0;
                            cells[cur_row + 1][cur_col].state = EMPTY_CELL;
                        }
                        else if (cells[cur_row][cur_col].state == CUTTER_TRASH_CELL)
                        {
                            cells[cur_row - 1][cur_col].has_cutter = 0;
                            cells[cur_row - 1][cur_col].state = EMPTY_CELL;
                        }
                    }
                    else if (cells[cur_row][cur_col].attached_cutter->direction == DOWN)
                    {
                        if (cells[cur_row][cur_col].state == CUTTER_IN_CELL)
                        {
                            cells[cur_row][cur_col - 1].has_cutter = 0;
                            cells[cur_row][cur_col - 1].state = EMPTY_CELL;
                        }
                        else if (cells[cur_row][cur_col].state == CUTTER_TRASH_CELL)
                        {
                            cells[cur_row][cur_col + 1].has_cutter = 0;
                            cells[cur_row][cur_col + 1].state = EMPTY_CELL;
                        }
                    }
                    else if (cells[cur_row][cur_col].attached_cutter->direction == LEFT)
                    {
                        if (cells[cur_row][cur_col].state == CUTTER_IN_CELL)
                        {
                            cells[cur_row - 1][cur_col].has_cutter = 0;
                            cells[cur_row - 1][cur_col].state = EMPTY_CELL;
                        }
                        else if (cells[cur_row][cur_col].state == CUTTER_TRASH_CELL)
                        {
                            cells[cur_row + 1][cur_col].has_cutter = 0;
                            cells[cur_row + 1][cur_col].state = EMPTY_CELL;
                        }
                    }
                    cells[cur_row][cur_col].has_cutter = 0;
                    cells[cur_row][cur_col].state = EMPTY_CELL;
                    delete cells[cur_row][cur_col].attached_cutter;
                    cells[cur_row][cur_col].attached_cutter = NULL;
                }
                if (cells[cur_row][cur_col].has_digger == 1)
                {
                    cells[cur_row][cur_col].has_digger = 0;
                    delete cells[cur_row][cur_col].attached_digger;
                    cells[cur_row][cur_col].attached_digger = NULL;
                }
                if (cells[cur_row][cur_col].has_track == 1)
                {
                    cells[cur_row][cur_col].has_track = 0;
                    delete cells[cur_row][cur_col].attached_track;
                    cells[cur_row][cur_col].attached_track = NULL;
                }
                if (cells[cur_row][cur_col].has_rotator == 1)
                {
                    cells[cur_row][cur_col].has_rotator = 0;
                    delete cells[cur_row][cur_col].attached_rotator;
                    cells[cur_row][cur_col].attached_rotator = NULL;
                }
                if (cells[cur_row][cur_col].has_trash_can == 1)
                {
                    cells[cur_row][cur_col].has_trash_can = 0;
                    delete cells[cur_row][cur_col].attached_trash_can;
                    cells[cur_row][cur_col].attached_trash_can = NULL;
                }

            }
            else
            {
                mouse_state = FREE;
                to_be_put_direction = UP;
            }
        }

    }
    else if (game_state == IN_GAME_UPGRADE)
    {
        //Mouse pressing the digger icon
        if (event->x() > 172 && event->x() < 310 && event->y() > 303 && event->y() < 447)
        {
            digging_speed *= 0.8;
            dig_timer.setInterval(digging_speed);
            game_state = START;
        }
        //Mouse pressing the track icon
        if (event->x() > 324 && event->x() < 462 && event->y() > 303 && event->y() < 447)
        {
            mine_moving_speed *= 1.2;
            game_state = START;
        }
        //Mouse pressing the cutter/rotator icon
        if (event->x() > 475 && event->x() < 615 && event->y() > 303 && event->y() < 447)
        {
            processing_speed *= 0.8;
            game_state = START;
        }
    }
    else if (game_state == SHOP)
    {
        bool has_changes = 0;
        //Mouse pressing the close icon
        if (event->x() > 706 && event->x() < 762 && event->y() > 182 && event->y() < 238)
        {
            game_state = MAIN_MENU;
        }
        //Mouse pressing the "buying more mine blocks" icon
        else if (event->x() > 199 && event->x() < 322 && event->y() > 363 && event->y() < 392)
        {
            if (coins >= 500)
            {
                coins -= 500;
                mine_block_num++;
                QMessageBox::information(nullptr, "购买成功", "购买成功！");
                has_changes = 1;
            }
            else
            {
                QMessageBox::information(nullptr, "购买失败", "金币数量不足，无法购买！");
            }
        }
        //Mouse pressing the "buying larger delivery center" icon
        else if (event->x() > 464 && event->x() < 587 && event->y() > 363 && event->y() < 392)
        {
            if (coins >= 1000)
            {
                if (delivery_center_mode == LARGER_MODE)
                {
                    QMessageBox::information(nullptr, "购买失败", "您的交付中心大小已经强化过了，无法继续强化！");
                }
                else
                {
                    coins -= 1000;
                    delivery_center_mode = LARGER_MODE;
                    QMessageBox::information(nullptr, "购买成功", "购买成功！");
                    has_changes = 1;
                }
            }
            else
            {
                QMessageBox::information(nullptr, "购买失败", "金币数量不足，无法购买！");
            }
        }
        //Mouse pressing the "buying more coins per collection" icon
        else if (event->x() > 199 && event->x() < 322 && event->y() > 554 && event->y() < 585)
        {
            if (coins >= 500)
            {
                coins -= 500;
                get_coins_per_collection++;
                QMessageBox::information(nullptr, "购买成功", "购买成功！");
                has_changes = 1;
            }
            else
            {
                QMessageBox::information(nullptr, "购买失败", "金币数量不足，无法购买！");
            }
        }
        //Mouse pressing the "buying larger map" icon
        else if (event->x() > 464 && event->x() < 587 && event->y() > 554 && event->y() < 585)
        {
            if (coins >= 2000)
            {
                if (map_mode == LARGER_MODE)
                {
                    QMessageBox::information(nullptr, "购买失败", "您的地图大小已经强化过了，无法继续强化！");
                }
                else
                {
                    coins -= 2000;
                    map_mode = LARGER_MODE;
                    QMessageBox::information(nullptr, "购买成功", "购买成功！");
                    has_changes = 1;
                }
            }
            else
            {
                QMessageBox::information(nullptr, "购买失败", "金币数量不足，无法购买！");
            }
        }
        if (has_changes == 1)
        {
            QDir* dir = new QDir(QDir::currentPath());
            dir->cdUp();
            QString directory = dir->absolutePath() + "/Factorio/config.txt";
            delete dir;

            QFile file(directory);
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            QTextStream out_file(&file);
            out_file.setIntegerBase(10);
            out_file << coins << " " << delivery_center_mode << " " << get_coins_per_collection << " " << mine_block_num << " " << map_mode << " ";
            file.close();
        }

    }
    event->accept();
}

void MainScene::mouseReleaseEvent(QMouseEvent *event)
{
    if (game_state == START)
    {
        if (mouse_state == DRAWING_TRACK)
        {
            if (event->button() == Qt::LeftButton)
            {
                mouse_state = FREE;
            }
        }
        event->accept();
    }
}

void MainScene::mouseMoveEvent(QMouseEvent* event)
{
    if (game_state == START)
    {
        if (event->buttons() & Qt::LeftButton)
        {
            if (mouse_state == FREE)
            {
                QPoint delta = event->globalPos() - last_mouse_position;
                last_mouse_position = event->globalPos();

                print_position_x += delta.x();
                print_position_y += delta.y();
            }

        }
        event->accept();
    }
}

void MainScene::keyPressEvent(QKeyEvent* event)
{
    if (game_state == START)
    {
        if (event->key() == Qt::Key_R && mouse_state != FREE && mouse_state != DRAWING_TRACK)
        {
            to_be_put_direction = (to_be_put_direction + 1) % 4;
        }
        event->accept();
    }
}

void MainScene::load(QString input_request)
{

    QFile file(input_request);
    file.open(QIODevice::ReadOnly);
    QTextStream in_file(&file);
    in_file.setIntegerBase(10);

    //Input order is the same as output order
    int tmp_coin;
    in_file >> tmp_coin >> get_coins_per_collection >> delivery_center_mode >> mine_block_num >> map_mode >> digging_speed >> mine_moving_speed >> processing_speed >> cur_mission_stage >> has_collect_mines;


    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 0; j < NUM_COLS; j++)
        {
            in_file >> cells[i][j].state;
            int has_digger;
            in_file >> has_digger;
            cells[i][j].has_digger = (has_digger != 0);
            if (cells[i][j].has_digger == 1)
            {
                int dir;
                in_file >> dir;
                digger* new_digger = new digger(dir);
                cells[i][j].attached_digger = new_digger;
            }
            int has_track;
            in_file >> has_track;
            cells[i][j].has_track = (has_track != 0);
            if (cells[i][j].has_track == 1)
            {
                int is_cor, dir;
                in_file >> is_cor >> dir;
                track* new_track = new track(is_cor, dir);
                cells[i][j].attached_track = new_track;
            }
            int has_cutter;
            in_file >> has_cutter;
            cells[i][j].has_cutter = (has_cutter != 0);
            if (cells[i][j].has_cutter == 1 && cells[i][j].state == CUTTER_IN_CELL)
            {
                int dir;
                in_file >> dir;
                cutter* new_cutter = new cutter(dir);
                cells[i][j].attached_cutter = new_cutter;
                if (dir == UP) cells[i][j + 1].attached_cutter = new_cutter;
                else if (dir == RIGHT) cells[i + 1][j].attached_cutter = new_cutter;
                else if (dir == DOWN) cells[i][j - 1].attached_cutter = new_cutter;
                else if (dir == LEFT) cells[i - 1][j].attached_cutter = new_cutter;
            }
            int has_trash_can;
            in_file >> has_trash_can;
            cells[i][j].has_trash_can = (has_trash_can != 0);
            if (cells[i][j].has_trash_can == 1)
            {
                trash_can* new_trash_can = new trash_can;
                cells[i][j].attached_trash_can = new_trash_can;
            }
            int has_rotator;
            in_file >> has_rotator;
            cells[i][j].has_rotator = (has_rotator != 0);
            if (cells[i][j].has_rotator == 1)
            {
                int dir;
                in_file >> dir;
                rotator* new_rotator = new rotator(dir);
                cells[i][j].attached_rotator = new_rotator;
            }
        }
    }

    for (int i = 0; i < MINE_TYPE_NUM; i++)
    {
        in_file >> count_mines[i];
        for (int j = 0; j < count_mines[i]; j++)
        {
            int tmp_type, dir, mov_dir;
            float tmp_x, tmp_y;
            int cutted;
            in_file >> tmp_type >> tmp_x >> tmp_y >> dir >> mov_dir >> cutted;
            Node* tmp = new Node(moving_mine(tmp_x - 2.5, tmp_y - 2, mov_dir, tmp_type), head[i]);

            for (int k = UP; k < dir; k++) tmp->content.rotate();
            if (cutted != 0) tmp->content.cut();
            tmp->content.direction = dir;
            tmp->content.is_cut = (cutted != 0);

            head[i] = tmp;
        }
    }

    file.close();
}

void MainScene::save(QString output_request)
{
    QFile file(output_request);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out_file(&file);
    out_file.setIntegerBase(10);

    /*
     * Output Order :
     *     1. One row : coins / delivery_center_mode / digging_speed / mine_moving_speed / processing_speed / cur_mission_stage / has_collected_mines
     *     2. NUM_ROWS * NUM_COLS rows, each row : cells[i][j].state / cells[i][j].has_digger / (if has_digger == 1) cells[i][j].attached_digger's info / ... / cells[i][j].has_rotator / (if has_rotator == 1) cells[i][j].attached_rotator's infor
     *     3. MINE_TYPE_NUM rows, each row : count_mines[i] / (for each mine element : type / x / y / direction / moving_direction / is_cut)
     */

    out_file << coins << " " << get_coins_per_collection << " " << delivery_center_mode << " " << mine_block_num << " " << map_mode << " " << digging_speed << " " << mine_moving_speed << " " << processing_speed << " " << cur_mission_stage << " " << has_collect_mines << " ";
    for (int i = 0; i < NUM_ROWS; i++)
    {
        for (int j = 0; j < NUM_COLS; j++)
        {
            out_file << cells[i][j].state << " " << cells[i][j].has_digger << " ";
            if (cells[i][j].has_digger == 1) out_file << cells[i][j].attached_digger->direction << " ";
            out_file << cells[i][j].has_track << " ";
            if (cells[i][j].has_track == 1) out_file << cells[i][j].attached_track->is_corner << " " << cells[i][j].attached_track->direction << " ";
            out_file << cells[i][j].has_cutter << " ";
            if (cells[i][j].has_cutter == 1 && cells[i][j].state == CUTTER_IN_CELL)
            {
                out_file << cells[i][j].attached_cutter->direction << " ";
            }
            out_file << cells[i][j].has_trash_can << " ";
            //No need to input anything because a trash can doesn't have any attribute
            out_file << cells[i][j].has_rotator << " ";
            if (cells[i][j].has_rotator == 1) out_file << cells[i][j].attached_rotator->direction << " ";
        }
    }

    for (int i = 0; i < MINE_TYPE_NUM; i++)
    {
        out_file << count_mines[i] << " ";
        Node* tmp = head[i];
        while (tmp != NULL)
        {
            out_file << tmp->content.type << " " << tmp->content.x << " " << tmp->content.y << " " << tmp->content.direction << " " << tmp->content.moving_direction << " " << tmp->content.is_cut << " ";
            tmp = tmp->next;
        }
        out_file << " ";
    }

    file.close();
}


