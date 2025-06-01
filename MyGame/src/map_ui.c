#include "map_ui.h"

#include <stdio.h>
#include <stdlib.h>

static const int screenWidth  = 640;   // 視窗寬度
static const int screenHeight = 640;   // 視窗高度
static const int gridSize     = 5;     // 5x5 地圖
static const int cellSize     = 100;   // 每格 100x100 像素
static const int marginX      = (screenWidth - gridSize * cellSize) / 2;
static const int marginY      = (screenHeight - gridSize * cellSize) / 2;

// 初始化視窗
void InitGameWindow(void) {
    InitWindow(screenWidth, screenHeight, "Nothing Can Go Wrong - Map");
    SetTargetFPS(60);
}

// 關閉視窗
void CloseGameWindow(void) {
    CloseWindow();
}

//Game Over
void ShowGameOverScreen(void) {
    BeginDrawing();
      ClearBackground(BLACK);
      DrawText("GAME OVER!", screenWidth/2 - MeasureText("GAME OVER!", 40)/2, screenHeight/2 - 20, 40, RED);
      DrawText("press ENTER to restart", screenWidth/2 - MeasureText("press ENTER to restart", 20)/2, screenHeight/2 + 30, 20, WHITE);
    EndDrawing();
    // 等玩家按 Enter，才跳回遊戲主地圖
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ENTER)) break;
        if (IsKeyPressed(KEY_ESCAPE)) { 
            CloseWindow(); 
            exit(0); 
        }
    }
}

//顯示通關成功畫面
void ShowWinScreen(void) {
    BeginDrawing();
      ClearBackground(RAYWHITE);
      DrawText("CONGRATULATIONS!", screenWidth/2 - MeasureText("CONGRATULATIONS!", 40)/2, screenHeight/2 - 20, 40, GREEN);
      DrawText("You have beaten both levels!", screenWidth/2 - MeasureText("You have beaten both levels!", 20)/2, screenHeight/2 + 30, 20, DARKGREEN);
    EndDrawing();
    //等玩家按 ESC 離開
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) break;
    }
}

//畫 5x5 地圖與角色、處理鍵盤
void UpdateGameLoop(GameState* state) {
    // 如果已經贏/輸，直接返回
    if (state->snakePassed && state->one2bPassed) {
        ShowWinScreen();
        CloseGameWindow();
        exit(0);
    }

    if (WindowShouldClose()) {
        CloseGameWindow();
        exit(0);
    }

    // 處理鍵盤：W/A/S/D 移動對應的格子 (上下左右)
    if (IsKeyPressed(KEY_W) && state->playerY > 0) {
        state->playerY--;
    } else if (IsKeyPressed(KEY_S) && state->playerY < gridSize - 1) {
        state->playerY++;
    } else if (IsKeyPressed(KEY_A) && state->playerX > 0) {
        state->playerX--;
    } else if (IsKeyPressed(KEY_D) && state->playerX < gridSize - 1) {
        state->playerX++;
    }

    // 判斷是否踩到關卡 1：貪吃蛇 -> 座標 (1,3) (0-based index: (x=1, y=3))
    if (!state->snakePassed && state->playerX == 1 && state->playerY == 3) {
        int res = runSnakeGame(); 
        if (res == 1) {
            state->snakePassed = true;
        } else {
            // 失敗就 Game Over
            ShowGameOverScreen();
            // 重置遊戲狀態
            state->playerX = 0; 
            state->playerY = 0;
            state->snakePassed  = false; 
            state->one2bPassed = false;
        }
    }

    // 判斷是否踩到關卡 2：1A2B -> 座標 (3,1)
    if (!state->one2bPassed && state->playerX == 3 && state->playerY == 1) {
        int res = run1A2BGame(); 
        if (res == 1) {
            state->one2bPassed = true;
        } else {
            // 失敗就 Game Over
            ShowGameOverScreen();
            state->playerX = 0; 
            state->playerY = 0;
            state->snakePassed  = false; 
            state->one2bPassed = false;
        }
    }

    // 繪製整個地圖與角色
    BeginDrawing();
      ClearBackground(RAYWHITE);

      // 畫 5x5 方格）
      for (int row = 0; row < gridSize; row++) {
          for (int col = 0; col < gridSize; col++) {
              int x = marginX + col * cellSize;
              int y = marginY + row * cellSize;
              DrawRectangleLines(x, y, cellSize, cellSize, DARKGRAY);
        
              if (col == 1 && row == 3) {
                  DrawRectangle(x + 2, y + 2, cellSize - 4, cellSize - 4, RED);
                  DrawText("Snake", x + 5, y + 5, 20, WHITE);
              }
              if (col == 3 && row == 1) {
                  DrawRectangle(x + 2, y + 2, cellSize - 4, cellSize - 4, BLUE);
                  DrawText("1A2B", x + 5, y + 5, 20, WHITE);
              }
          }
      }

      // 畫玩家角色：一個小綠色方塊
      int px = marginX + state->playerX * cellSize + cellSize / 4;
      int py = marginY + state->playerY * cellSize + cellSize / 4;
      int pw = cellSize / 2, ph = cellSize / 2;
      DrawRectangle(px, py, pw, ph, GREEN);

      //提示
      DrawText("Use W/A/S/D to move. Reach both red & blue cells.", 10, 10, 20, BLACK);
      DrawText(TextFormat("Snake: %s    1A2B: %s", 
                state->snakePassed ? "Done" : "Not Yet", 
                state->one2bPassed ? "Done" : "Not Yet"), 
              10, 40, 20, DARKGRAY);

    EndDrawing();
}


