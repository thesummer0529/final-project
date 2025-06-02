#ifndef SCORE_H
#define SCORE_H

typedef struct {
    char name[20];
    float time_sec;   // 破關所花時間（秒）
} Record;

int  loadScores(Record *rec, int max);
void saveScore(const char *name, float sec);
void printTopScores(int topN);

#endif // SCORE_H