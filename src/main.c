#include "map_ui.h"
#include <stdio.h>
#include <string.h>

// 全域玩家名稱，讓其他模組（Snake、1A2B、排行榜）都能存取
char gPlayerName[32] = "Player";

int main(void) {
    printf("Enter your name: ");
    if (fgets(gPlayerName, sizeof(gPlayerName), stdin)) {
        size_t len = strlen(gPlayerName);
        if (len > 0 && (gPlayerName[len - 1] == '\n' || gPlayerName[len - 1] == '\r')) {
            gPlayerName[len - 1] = '\0';
        }
    }

    // 初始化遊戲狀態
    GameState state = { .playerX = 0, .playerY = 0, .snakePassed = false, .one2bPassed = false };

    printf("Welcome, %s! Start your adventure.\n", gPlayerName);
    printf("Rules: complete both challenges to win.\n");
    printf("Red: Greedy Snake   Blue: 1A2B\n");
    printf("If you lose any, it's game over!\n");

    InitGameWindow();
    while (!WindowShouldClose()) {
        UpdateGameLoop(&state);
    }
    CloseGameWindow();
    return 0;
}
