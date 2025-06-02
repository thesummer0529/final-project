// ==================== map_ui.c ====================
#include "map_ui.h"
#include <stdio.h>
#include <stdlib.h>

// ---------------- 視窗與格子設定 ----------------
static const int screenWidth  = 640;
static const int screenHeight = 640;
static const int gridSize     = 5;      // 5×5
static const int cellSize     = 100;    // 每格 100 像素
static const int marginX      = (screenWidth  - gridSize * cellSize) / 2;
static const int marginY      = (screenHeight - gridSize * cellSize) / 2;

// ---------------- 介面函式 ----------------
void InitGameWindow(void) {
    InitWindow(screenWidth, screenHeight, "Nothing Can Go Wrong - Map");
    SetTargetFPS(60);
}

void CloseGameWindow(void) {
    CloseWindow();
}

/*-----------------------------------------------
  這兩個等待畫面都改成「每幀重畫 + 處理輸入」
------------------------------------------------*/
void ShowGameOverScreen(void) {
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ENTER)) break;
        if (IsKeyPressed(KEY_ESCAPE)) { CloseWindow(); exit(0); }

        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("GAME OVER!", screenWidth/2 - MeasureText("GAME OVER!", 40)/2,
                     screenHeight/2 - 20, 40, RED);
            DrawText("Press ENTER to restart",
                     screenWidth/2 - MeasureText("Press ENTER to restart", 20)/2,
                     screenHeight/2 + 30, 20, WHITE);
        EndDrawing();
    }
}

void ShowWinScreen(void) {
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) break;

        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("CONGRATULATIONS!", screenWidth/2 - MeasureText("CONGRATULATIONS!", 40)/2,
                     screenHeight/2 - 20, 40, GREEN);
            DrawText("You have beaten both levels!",
                     screenWidth/2 - MeasureText("You have beaten both levels!", 20)/2,
                     screenHeight/2 + 30, 20, DARKGREEN);
        EndDrawing();
    }
}

/*-----------------------------------------------
  遊戲主地圖：鍵盤處理、關卡觸發、地圖繪製
------------------------------------------------*/
void UpdateGameLoop(GameState* state) {

    if (state->snakePassed && state->one2bPassed) {
        ShowWinScreen();
        CloseGameWindow();
        exit(0);
    }
    if (WindowShouldClose()) {
        CloseGameWindow();
        exit(0);
    }

    // ---- 鍵盤移動 ----
    if (IsKeyPressed(KEY_W) && state->playerY > 0)               state->playerY--;
    else if (IsKeyPressed(KEY_S) && state->playerY < gridSize-1) state->playerY++;
    else if (IsKeyPressed(KEY_A) && state->playerX > 0)          state->playerX--;
    else if (IsKeyPressed(KEY_D) && state->playerX < gridSize-1) state->playerX++;

    // ---- 進入貪吃蛇關 ----
    if (!state->snakePassed && state->playerX == 1 && state->playerY == 3) {
        if (runSnakeGame()) state->snakePassed = true;
        else {
            ShowGameOverScreen();
            *state = (GameState){ .playerX=0, .playerY=0, .snakePassed=false, .one2bPassed=false };
        }
    }

    // ---- 進入 1A2B 關 ----
    if (!state->one2bPassed && state->playerX == 3 && state->playerY == 1) {
        if (run1A2BGame()) state->one2bPassed = true;
        else {
            ShowGameOverScreen();
            *state = (GameState){ .playerX=0, .playerY=0, .snakePassed=false, .one2bPassed=false };
        }
    }

    // ---- 繪圖 ----
    BeginDrawing();
        ClearBackground(RAYWHITE);

        // 地圖格線 + 關卡方塊
        for (int row=0; row<gridSize; row++) {
            for (int col=0; col<gridSize; col++) {
                int x = marginX + col*cellSize;
                int y = marginY + row*cellSize;
                DrawRectangleLines(x, y, cellSize, cellSize, DARKGRAY);

                if (col==1 && row==3) {
                    DrawRectangle(x+2, y+2, cellSize-4, cellSize-4, RED);
                    DrawText("Snake", x+5, y+5, 20, WHITE);
                }
                if (col==3 && row==1) {
                    DrawRectangle(x+2, y+2, cellSize-4, cellSize-4, BLUE);
                    DrawText("1A2B",  x+15, y+5, 20, WHITE);
                }
            }
        }

        // 玩家方塊
        int px = marginX + state->playerX*cellSize + cellSize/4;
        int py = marginY + state->playerY*cellSize + cellSize/4;
        DrawRectangle(px, py, cellSize/2, cellSize/2, GREEN);

        // 提示文字
        DrawText("Use W/A/S/D to move. Reach both red & blue cells.", 10, 10, 20, BLACK);
        DrawText(TextFormat("Snake: %s    1A2B: %s",
                 state->snakePassed ? "Done" : "Not Yet",
                 state->one2bPassed ? "Done" : "Not Yet"),
                 10, 40, 20, DARKGRAY);
    EndDrawing();
}
