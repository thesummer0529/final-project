#ifndef MAP_UI_H
#define MAP_UI_H
#include <stdbool.h>
#include "raylib.h"   // 假設使用 raylib 做簡易 2D UI

// 角色在地圖上的遊戲狀態
typedef struct {
    int  playerX, playerY;   // 玩家格子座標
    bool snakePassed;        // 是否已通過貪吃蛇
    bool one2bPassed;        // 是否已通過 1A2B
} GameState;

void InitGameWindow(void);
void CloseGameWindow(void);
void UpdateGameLoop(GameState *state);

// 兩個關卡對外介面
int runSnakeGame(void);
int run1A2BGame(void);

#endif // MAP_UI_H