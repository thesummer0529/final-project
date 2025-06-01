#ifndef MAP_UI_H
#define MAP_UI_H

#include <stdbool.h>
#include <raylib.h>

//記錄遊戲狀態
typedef struct {
    int playerX;        // 玩家所在格子的列 (0~4)
    int playerY;        // 玩家所在格子的行 (0~4)
    bool snakePassed;   // 關卡1：貪吃蛇有無通過
    bool one2bPassed;   // 關卡2：1A2B 有無通過
} GameState;

//初始化畫面
void InitGameWindow(void);

//釋放資源
void CloseGameWindow(void);

// 顯示 Game Over 畫面
void ShowGameOverScreen(void);

// 顯示通關成功畫面
void ShowWinScreen(void);

// 主更新迴圈：畫 5x5 地圖與角色、處理鍵盤
void UpdateGameLoop(GameState* state);

// 這兩個函式在 snake.c / one2b.c 裡實作
int runSnakeGame(void);
int run1A2BGame(void);

#endif // MAP_UI_H
