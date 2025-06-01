#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include "snake.h"

enum Direction { UP, DOWN, LEFT, RIGHT };
#define MAX_LENGTH       400  // 蛇的最大長度
#define EMPTY             0   // 空地
#define FOOD              1   // 食物
#define OBSTACLE          2   // 障礙物
#define BOOSTER_SPEED     3   // 速度加成
#define BOOSTER_SHRINK    4   // 縮短蛇身
#define BOOSTER_LIFE      5   // 額外生命
#define SNAKE_HEAD        6   // 蛇頭
#define SNAKE_BODY        7   // 蛇身

// 勝利條件：得分(食物數量)達到 10 分或蛇身長度 >= 15 視為過關
#define WIN_SCORE        10
#define WIN_LENGTH       15

typedef struct {
    int x, y;                 // 座標
} Point;

typedef struct {
    Point *body;              // 蛇的身體座標
    int length;               // 蛇的長度
    enum Direction dir;       // 移動方向
    int speed;                // 移動間隔 (微秒)
    bool boosters[3];         // 是否已經使用過三種加成 (速度、縮身、加命)
} Snake;

typedef struct {
    int **grid;               // 地圖格子 (0=空, 1=食物, 2=障礙, 3~5=加成, 6=蛇頭,7=蛇身)
    int width, height;        // 地圖大小
} Map;

typedef struct {
    int score;                // 得分 (食物數量)
    int lives;                // 剩餘生命數
    bool running;             // 遊戲是否進行中
} Gamestate;

// 函式宣告
static Snake* createSnake(int initialLength, int speed);
static Map*   createMap(int width, int height);
static void   initializeGame(Map *map, Snake *snake, Gamestate *state, int obstaclecount);
static void   handleInput(Snake *snake);
static void   updateGame(Map *map, Snake *snake, Gamestate *state);
static void   renderGame(Map *map, Snake *snake, Gamestate *state);
static void   spawnObject(Map *map, int type);
static void   applyBooster(Snake *snake, Gamestate *state, int type);
static void   gameOver(Gamestate *state);
static void   freeResources(Map *map, Snake *snake);

int runSnakeGame(void) {
    // 1.玩家選擇關卡的難度 (兩種：1=普通,2=困難)
    int choice;
    int mapWidth = 20, mapHeight = 20;// 預設地圖大小 20x20
    int initialSpeed, obstaclecount;

    //讓玩家在關卡裡選擇地圖大小與難度
    
    printf("welcome to greedy snake\n");
    printf("choose you map:\n1. 10x10   2. 15x15   3.20x20  >> ");
    scanf("%d", &choice);
    switch (choice) {
        case 1: mapWidth = mapHeight = 10; break;
        case 2: mapWidth = mapHeight = 15; break;
        default: mapWidth = mapHeight = 20; break;
    }

    printf("choose the difficulty \n1.normal  2.hard >> ");
    scanf("%d", &choice);
    

    //1: 普通, 2: 困難
    if (choice == 1) {
        initialSpeed  = 500000;   // 0.5 秒一次
        obstaclecount = 1;
    } else {
        initialSpeed  = 300000;   // 0.3 秒一次
        obstaclecount = 2;
    }

    Snake *snake = createSnake(3, initialSpeed);
    Map   *map   = createMap(mapWidth, mapHeight);
    Gamestate state = { .score = 0, .lives = 1, .running = true };

    // 3. 初始化地圖、蛇、食物、障礙、加成
    initializeGame(map, snake, &state, obstaclecount);

    // 4. 遊戲主迴圈
    while (state.running) {
        handleInput(snake);
        updateGame(map, snake, &state);
        renderGame(map, snake, &state);

        // 檢查「勝利條件」：得分 >= WIN_SCORE 或 長度 >= WIN_LENGTH
        if (state.score >= WIN_SCORE || snake->length >= WIN_LENGTH) {
            // 顯示過關訊息並稍作停留
            printf("\ncongratulations you win! Score=%d, Length=%d ===\n",
                   state.score, snake->length);
            Sleep(2000);
            // 釋放資源並回傳成功
            freeResources(map, snake);
            return 1;
        }

        // 控制速度，Sleep 參數單位是「毫秒」，snake->speed 是「微秒」
        Sleep(snake->speed / 1000);
    }
    freeResources(map, snake);
    return 0;
}

// 建立蛇
static Snake* createSnake(int initialLength, int speed) {
    Snake *snake = (Snake *)malloc(sizeof(Snake));
    snake->body  = (Point *)malloc(MAX_LENGTH * sizeof(Point));
    snake->length = initialLength;
    snake->speed  = speed;
    snake->dir    = RIGHT;
    for (int i = 0; i < 3; i++) {
        snake->boosters[i] = false;
    }
    return snake;
}

// 建立地圖
static Map* createMap(int width, int height) {
    Map *map = (Map *)malloc(sizeof(Map));
    map->width  = width;
    map->height = height;
    map->grid   = (int **)malloc(height * sizeof(int *));
    for (int i = 0; i < height; i++) {
        map->grid[i] = (int *)malloc(width * sizeof(int));
        for (int j = 0; j < width; j++) {
            map->grid[i][j] = EMPTY;
        }
    }
    return map;
}

// 初始化遊戲 (建立初始蛇身、放置食物、障礙、加成)
static void initializeGame(Map *map, Snake *snake, Gamestate *state, int obstaclecount) {
    srand((unsigned int)time(NULL));

    // 初始化蛇的位置 (從左上角向右展開)
    for (int i = 0; i < snake->length; i++) {
        snake->body[i].x = snake->length - 1 - i;
        snake->body[i].y = 0;
        map->grid[0][snake->length - 1 - i] = (i == 0 ? SNAKE_HEAD : SNAKE_BODY);
    }

    // 生成一個食物
    spawnObject(map, FOOD);

    // 生成障礙物
    for (int i = 0; i < obstaclecount; i++) {
        spawnObject(map, OBSTACLE);
    }

    // 生成三種加成道具 (速度、縮身、加命)
    for (int i = 0; i < 3; i++) {
        spawnObject(map, BOOSTER_SPEED + i);
    }
}

//處理玩家輸入 (W/A/S/D 控制方向)
static void handleInput(Snake *snake) {
    if (_kbhit()) {
        char input = _getch();
        switch (input) {
            case 'w':
            case 'W':
                if (snake->dir != DOWN) snake->dir = UP;
                break;
            case 's':
            case 'S':
                if (snake->dir != UP) snake->dir = DOWN;
                break;
            case 'a':
            case 'A':
                if (snake->dir != RIGHT) snake->dir = LEFT;
                break;
            case 'd':
            case 'D':
                if (snake->dir != LEFT) snake->dir = RIGHT;
                break;
        }
    }
}

//更新遊戲狀態 (移動蛇、吃東西、碰撞檢查、加成效果)
static void updateGame(Map *map, Snake *snake, Gamestate *state) {
    //計算新蛇頭座標
    Point newHead = snake->body[0];
    switch (snake->dir) {
        case UP:    newHead.y--; break;
        case DOWN:  newHead.y++; break;
        case LEFT:  newHead.x--; break;
        case RIGHT: newHead.x++; break;
    }

    //碰撞檢查：撞牆
    if (newHead.x < 0 || newHead.x >= map->width ||
        newHead.y < 0 || newHead.y >= map->height) {
        gameOver(state);
        return;
    }

    // 讀取新位置的格子內容
    int cell = map->grid[newHead.y][newHead.x];

    //碰到障礙物
    if (cell == OBSTACLE) {
        state->lives--;
        if (state->lives <= 0) {
            gameOver(state);
            return;
        } else {
            //若還有生命，移除該障礙並重生一個障礙
            map->grid[newHead.y][newHead.x] = EMPTY;
            spawnObject(map, OBSTACLE);
            return; //只扣命不繼續移動
        }
    }

    //吃到食物
    if (cell == FOOD) {
        state->score++;
        snake->length++;
        spawnObject(map, FOOD);
        // 調整速度：每吃 10 分，減少 100000 微秒 (最多減到 200000 微秒)
        int newSpeed = 500000 - (state->score / 10) * 100000;
        if (newSpeed < 200000) newSpeed = 200000;
        snake->speed = newSpeed;
    }

    //吃到加成道具
    if (cell >= BOOSTER_SPEED && cell <= BOOSTER_LIFE) {
        int idx = cell - BOOSTER_SPEED;
        if (!snake->boosters[idx]) {
            snake->boosters[idx] = true;
            applyBooster(snake, state, cell);
        }
    }

    //檢查是否咬到自己
    for (int i = 0; i < snake->length; i++) {
        if (snake->body[i].x == newHead.x && snake->body[i].y == newHead.y) {
            // 撞到自己
            gameOver(state);
            return;
        }
    }

    //更新蛇身
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    snake->body[0] = newHead;

    // 清空地圖中舊的蛇頭/蛇身標記
    for (int i = 0; i < map->height; i++) {
        for (int j = 0; j < map->width; j++) {
            if (map->grid[i][j] == SNAKE_HEAD || map->grid[i][j] == SNAKE_BODY) {
                map->grid[i][j] = EMPTY;
            }
        }
    }
    for (int i = 0; i < snake->length; i++) {
        int xx = snake->body[i].x;
        int yy = snake->body[i].y;
        map->grid[yy][xx] = (i == 0 ? SNAKE_HEAD : SNAKE_BODY);
    }
}

// 隨機生成物件 (食物 / 障礙 / 加成)
static void spawnObject(Map *map, int type) {
    int x, y;
    do {
        x = rand() % map->width;
        y = rand() % map->height;
    } while (map->grid[y][x] != EMPTY);
    map->grid[y][x] = type;
}

// 套用加成效果 (BOOSTER_SPEED / BOOSTER_SHRINK / BOOSTER_LIFE)
static void applyBooster(Snake *snake, Gamestate *state, int type) {
    switch (type) {
        case BOOSTER_SPEED:
            // 進一步提高速度 (最多 100000 微秒)
            snake->speed -= 100000;
            if (snake->speed < 100000) snake->speed = 100000;
            break;
        case BOOSTER_SHRINK:
            // 縮短兩節身體 (如果長度 > 3 才縮)
            if (snake->length > 3) snake->length -= 2;
            break;
        case BOOSTER_LIFE:
            // 增加一條命
            state->lives++;
            break;
    }
}

// 顯示地圖與分數
static void renderGame(Map *map, Snake *snake, Gamestate *state) {
    system("cls");

    for (int i = 0; i < map->height; i++) {
        for (int j = 0; j < map->width; j++) {
            int c = map->grid[i][j];
            switch (c) {
                case EMPTY:         printf("."); break;
                case FOOD:          printf("F"); break;
                case OBSTACLE:      printf("X"); break;
                case BOOSTER_SPEED: printf("S"); break;
                case BOOSTER_SHRINK:printf("R"); break;
                case BOOSTER_LIFE:  printf("L"); break;
                case SNAKE_HEAD:    printf("H"); break;
                case SNAKE_BODY:    printf("O"); break;
                default:            printf("?"); break;
            }
        }
        printf("\n");
    }
    printf("Score: %d   Lives: %d   (Eat %d more to win)\n",
           state->score,
           state->lives,
           (WIN_SCORE - state->score > 0 ? WIN_SCORE - state->score : 0)
    );
}

// 遊戲結束
static void gameOver(Gamestate *state) {
    printf("\nGAME OVER! youe point is %d\n", state->score);
    state->running = false;
}

static void freeResources(Map *map, Snake *snake) {
    if (map) {
        for (int i = 0; i < map->height; i++) {
            free(map->grid[i]);
        }
        free(map->grid);
        free(map);
    }
    if (snake) {
        free(snake->body);
        free(snake);
    }
}
