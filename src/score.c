#include "score.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCORE_FILE "score.txt"

int loadScores(Record *rec, int max) {
    FILE *fp = fopen(SCORE_FILE, "r");
    if (!fp) return 0;
    int n = 0;
    while (n < max && fscanf(fp, "%19s%f", rec[n].name, &rec[n].time_sec) == 2) n++;
    fclose(fp);
    return n;
}

void saveScore(const char *name, float sec) {
    FILE *fp = fopen(SCORE_FILE, "a");
    if (!fp) return;
    fprintf(fp, "%s %.2f\n", name, sec);
    fclose(fp);
}

static int cmp(const void *a, const void *b) {
    float diff = ((const Record*)a)->time_sec - ((const Record*)b)->time_sec;
    return (diff > 0) - (diff < 0); // 升序：時間越短越前面
}

void printTopScores(int topN) {
    Record rec[64];
    int n = loadScores(rec, 64);
    if (n == 0) {
        printf("No records yet.\n");
        return;
    }
    qsort(rec, n, sizeof(Record), cmp);
    printf("\n===== Leaderboard (Top %d) =====\n", topN);
    for (int i = 0; i < n && i < topN; i++) {
        printf("%2d. %-20s %.2f s\n", i + 1, rec[i].name, rec[i].time_sec);
    }
    printf("===============================\n\n");
}