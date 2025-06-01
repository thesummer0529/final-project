#include "map_ui.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    char playerName[32];
    printf("Enter your name: ");
    if (fgets(playerName, sizeof(playerName), stdin)) {
        size_t len = strlen(playerName);
        if (len > 0 && (playerName[len - 1] == '\n' || playerName[len - 1] == '\r')) {
            playerName[len - 1] = '\0';
        }
    }

    //初始化遊戲狀態
    GameState state = { .playerX = 0, .playerY = 0, .snakePassed = false, .one2bPassed = false };

    //歡迎訊息
    printf("Welcome, %s! Start your adventure.\n", playerName);
    printf("Rules: complete both challenges to win.\n");
    printf("Red: Greedy Snake   Blue: 1A2B\n");
    printf("If you lose any, it's game over!\n");

    //建立視窗
    InitGameWindow();

    while (!WindowShouldClose()) {
        UpdateGameLoop(&state);
    }
    CloseGameWindow();
    return 0;
}
