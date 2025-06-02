#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "one2b.h"

#define MAX_ATTEMPTS 7

static int questioner_mode(void);
static int answerer_mode(void);
static int search_next_guess(int array[][4], int set[][4], int arraycount, int setcount);
static void bubble_sort(int arr[], int n);

// run1A2BGame
int run1A2BGame(void) {
    int role = 0;

    // 1) 讓玩家選 1=出題者, 2=猜題者
    printf("1A2Bchallenge\n");
    printf("choose your character,press 1 to be questioner or press 2 to be anwserer)\n");
    if (scanf(" %d", &role) != 1) {
        // 若輸入失敗，視為失敗
        while (getchar() != '\n');
        return 0;
    }
    getchar();

    if (role == 1) {
        // 玩家當出題者
        return questioner_mode();
    } else if (role == 2) {
        // 玩家當猜題者
        return answerer_mode();
    } else {
        printf("you can only enter 1 or 2\n");
        return 0;
    }
}

//玩家當出題者(questioner_mode)

static int questioner_mode(void) {
    // 先讓玩家輸入一組 4 位不重複的答案
    char buf[16];
    int answer[4];
    int used[10] = {0};

    printf("you are the questioner enter 4 not repeated numbers");
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        return 0;
    }
    if (strlen(buf) < 4) {
        printf("you need to enter 4 numbers!\n");
        return 0;
    }

    if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';

    for (int i = 0; i < 4; i++) {
        if (buf[i] < '0' || buf[i] > '9') {
            printf("you must enter numbers\n");
            return 0;
        }
        answer[i] = buf[i] - '0';
        if (used[answer[i]] == 1) {
            printf("numbers repeat enter again\n");
            return 0;
        }
        used[answer[i]] = 1;
    }

    // 1a2b 所有 5040 個可能性預先生成
    int all[5040][4];
    int cnt = 0;
    for (int i = 0; i <= 9; i++) {
        for (int j = 0; j <= 9; j++) {
            for (int k = 0; k <= 9; k++) {
                for (int l = 0; l <= 9; l++) {
                    if (i!=j && i!=k && i!=l && j!=k && j!=l && k!=l) {
                        all[cnt][0] = i;
                        all[cnt][1] = j;
                        all[cnt][2] = k;
                        all[cnt][3] = l;
                        cnt++;
                    }
                }
            }
        }
    }

    // 電腦第一次隨機猜一組
    srand((unsigned)time(NULL));
    int arraycount = 5040;
    int pc_index = rand() % arraycount;
    int pc_guess[4];
    for (int i = 0; i < 4; i++) {
        pc_guess[i] = all[pc_index][i];
    }

    // 剩餘的「possible set」一開始就是所有 5040 組
    int setarr[5040][4];
    memcpy(setarr, all, sizeof(all));
    int setcount = arraycount;

    int attempts = 0; 
    int A, B;
    char separator1, separator2;
    while (1) {
        if (attempts >= MAX_ATTEMPTS) {
            // 電腦在 MAX_ATTEMPTS 次內都沒猜到 → 玩家 (出題者) 勝利
            printf("congratulations the boss did not get your number before %d times \n", MAX_ATTEMPTS);
            return 1;
        }

        // 第 N 次猜測
        printf("PC Guess: %d%d%d%d\n", pc_guess[0], pc_guess[1], pc_guess[2], pc_guess[3]);
        printf("Enter your feedback (format XAYB, e.g., 1A2B):");

        // 玩家必須輸入 "xAyB" 形式，x 與 y 都是 0~4 之間
        if (scanf("%d%c%d%c", &A, &separator1, &B, &separator2) != 4) {
            // 格式錯誤
            while (getchar() != '\n');
            printf("Invalid input format! You lose.\n");
            return 0;
        }
        // 確保 separator1 == 'A' 且 separator2 == 'B'
        if (separator1 != 'A' || separator2 != 'B' || A < 0 || A > 4 || B < 0 || B > 4) {
            printf("Hint format must be XAYB where X,Y ∈ [0,4]. You lose.\n");
            return 0;
        }
        getchar();

        // 檢查玩家有沒有「亂給提示」，也就是沒有任何一組 possible candidate 能同時符合這個 A/B
        int newsetcount = 0;
        int newset[5040][4];
        int tmpA, tmpB;
        for (int i = 0; i < setcount; i++) {
            // 計算 setarr[i] 與 pc_guess 的 A/B
            tmpA = tmpB = 0;
            for (int x = 0; x < 4; x++) {
                if (setarr[i][x] == pc_guess[x]) tmpA++;
                else {
                    for (int y = 0; y < 4; y++) {
                        if (y != x && setarr[i][x] == pc_guess[y]) {
                            tmpB++;
                            break;
                        }
                    }
                }
            }
            if (tmpA == A && tmpB == B) {
                // 這個候選 setarr[i] 仍然合法，加入新的候補組
                newset[newsetcount][0] = setarr[i][0];
                newset[newsetcount][1] = setarr[i][1];
                newset[newsetcount][2] = setarr[i][2];
                newset[newsetcount][3] = setarr[i][3];
                newsetcount++;
            }
        }
        if (newsetcount == 0) {
            // 沒有任何一組能同時符合玩家給的 A/B → 玩家作弊
            printf("You Cheat!!\n");
            return 0;
        }

        // 如果 newsetcount == 1，代表只剩 1 組可能 → 下一次 PC 一定猜這組
        // 否則用 search_next_guess() 算最佳下一個 pc_guess
        // 先把 newset copy 回 setarr
        for (int i = 0; i < newsetcount; i++) {
            setarr[i][0] = newset[i][0];
            setarr[i][1] = newset[i][1];
            setarr[i][2] = newset[i][2];
            setarr[i][3] = newset[i][3];
        }
        setcount = newsetcount;

        // 接下來 PC 要用淘汰法選下一個猜測
        pc_index = search_next_guess(all, setarr, arraycount, setcount);
        for (int i = 0; i < 4; i++) {
            pc_guess[i] = all[pc_index][i];
        }

        // 如果猜中，就失敗；如果沒猜中，繼續下一輪
        if (pc_guess[0] == answer[0] &&
            pc_guess[1] == answer[1] &&
            pc_guess[2] == answer[2] &&
            pc_guess[3] == answer[3]) {
            // PC 猜中 → 玩家出題者失敗
            printf("PC Wins!\n");
            return 0;
        }

        attempts++;
    }
}

//玩家當猜題者 (answerer_mode)
static int answerer_mode(void) {
    // 1. 電腦先隨機從 5040 組中選一個做為秘密答案
    int all[5040][4];
    int cnt = 0;
    for (int i = 0; i <= 9; i++) {
        for (int j = 0; j <= 9; j++) {
            for (int k = 0; k <= 9; k++) {
                for (int l = 0; l <= 9; l++) {
                    if (i!=j && i!=k && i!=l && j!=k && j!=l && k!=l) {
                        all[cnt][0] = i;
                        all[cnt][1] = j;
                        all[cnt][2] = k;
                        all[cnt][3] = l;
                        cnt++;
                    }
                }
            }
        }
    }
    // 隨機選一組
    srand((unsigned)time(NULL));
    int idx = rand() % cnt;
    int secret[4] = { all[idx][0], all[idx][1], all[idx][2], all[idx][3] };

    char buf[16];
    int guess[4];
    int A, B;
    int attempts = 0;

    printf("You are the answerer. Try to guess the answer in %d attempts.\n", MAX_ATTEMPTS);

    while (attempts < MAX_ATTEMPTS) {
        attempts++;
        printf("Attempt %d: Enter 4 unique digits >>", attempts);
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            return 0;
        }
        if (strlen(buf) < 4) {
            printf("Input too short!\n");
            return 0;
        }
        if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';

        // 轉成整數陣列並檢查合法性
        int used[10] = {0};
        for (int i = 0; i < 4; i++) {
            if (buf[i] < '0' || buf[i] > '9') {
                printf("Input must be numeric digits!\n");
                return 0;
            }
            guess[i] = buf[i] - '0';
            if (used[guess[i]] == 1) {
                printf("Duplicate digit detected. Please enter again!\n");
                return 0;
            }
            used[guess[i]] = 1;
        }

        // 計算 A/B
        A = B = 0;
        for (int i = 0; i < 4; i++) {
            if (guess[i] == secret[i]) {
                A++;
            } else {
                // 若數字相同但位置不同，就算 B
                for (int j = 0; j < 4; j++) {
                    if (j != i && guess[i] == secret[j]) {
                        B++;
                        break;
                    }
                }
            }
        }
        printf("Feedback: %dA%dB\n", A, B);

        if (A == 4) {
            // 猜中
            printf("You guessed it correctly in %d tries! You win!\n", attempts);
            return 1;
        }
    }

    // 超過 MAX_ATTEMPTS 次還沒猜中 → 失敗
    printf("Attempts exhausted. You lose.\n");
    return 0;
}
// 支援電腦猜題淘汰演算法
static int search_next_guess(int array[][4], int set[][4], int arraycount, int setcount) {
    int xAyB_check[2];
    int value[arraycount][14];
    for (int i = 0; i < arraycount; i++) {
        for (int j = 0; j < 14; j++) {
            value[i][j] = 0;
        }
    }

    // 對每一個「array[i]」(候選答案)，計算如果它當作 PC 猜測
    // 對 set 中每一組答案(set[j]) 會得到多少種 XAYB，value[i][] 就加 1
    for (int i = 0; i < arraycount; i++) {
        for (int j = 0; j < setcount; j++) {
            int A = 0, B = 0;
            // 計算 A
            for (int x = 0; x < 4; x++) {
                if (array[i][x] == set[j][x]) {
                    A++;
                }
            }
            // 計算 B
            for (int x = 0; x < 4; x++) {
                for (int y = 0; y < 4; y++) {
                    if (x != y && array[i][x] == set[j][y]) {
                        B++;
                    }
                }
            }
            // 對應到 value[i][?] 的 index
            // (A=0,B=0)->idx=0, (A=0,B=1)->idx=1, …, (A=3,B=0)->idx=12, (A=4,B=0)->idx=13
            int idx = 0;
            if (A == 0 && B == 0) idx = 0;
            if (A == 0 && B == 1) idx = 1;
            if (A == 0 && B == 2) idx = 2;
            if (A == 0 && B == 3) idx = 3;
            if (A == 0 && B == 4) idx = 4;
            if (A == 1 && B == 0) idx = 5;
            if (A == 1 && B == 1) idx = 6;
            if (A == 1 && B == 2) idx = 7;
            if (A == 1 && B == 3) idx = 8;
            if (A == 2 && B == 0) idx = 9;
            if (A == 2 && B == 1) idx = 10;
            if (A == 2 && B == 2) idx = 11;
            if (A == 3 && B == 0) idx = 12;
            if (A == 4 && B == 0) idx = 13;
            value[i][idx]++;
        }
    }

    // 每一列 value[i][] 都要從大到小排序 (bubble sort)
    for (int i = 0; i < arraycount; i++) {
        bubble_sort(value[i], 14);
    }

    // 再把每一列裡面「數值最小、也就是最均勻分布」的那個 i 回傳
    // 下面這段是原本那種「淘汰最壞情況」的挑選邏輯
    int b0[arraycount], b1[arraycount];
    for (int i = 0; i < arraycount; i++) {
        b0[i] = 1;
        b1[i] = 1;
    }
    int truecount = setcount;
    for (int i = 0; i < 14; i++) {
        while (truecount > 0) {
            int tmpcount = 0;
            for (int m = 0; m < arraycount; m++) {
                if (value[m][i] > truecount) {
                    b1[m] = 0;
                }
            }
            for (int n = 0; n < arraycount; n++) {
                if (b1[n] == 1) tmpcount++;
            }
            if (tmpcount == 0) break;
            for (int n = 0; n < arraycount; n++) {
                b0[n] = b1[n];
            }
            truecount--;
        }
        if (truecount == 0 && i == 13) break;
        for (int n = 0; n < arraycount; n++) {
            b1[n] = b0[n];
        }
    }

    int o = 0;
    if (truecount == 0) {
        while (b0[o] == 0) o++;
    } else {
        while (b1[o] == 0) o++;
    }
    return o;
}

static void bubble_sort(int arr[], int n) {
    for (int i = n - 1; i >= 0; i--) {
        for (int j = i - 1; j >= 0; j--) {
            if (arr[j] < arr[i]) {
                int tmp = arr[j];
                arr[j] = arr[i];
                arr[i] = tmp;
            }
        }
    }
}
