#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include "snake.h"

// 從 main.c 取得玩家名稱
extern char gPlayerName[32];

// ────────────────────────────────────────────
// 排行榜模組 (score.c 的精簡內嵌版)
// ────────────────────────────────────────────
#define MAX_REC 64

typedef struct {
    char  name[32];
    float time_sec;      // 通關秒數 (越小越好)
} Record;

static int  loadScores(Record *rec, int max);
static void saveScore(const char *name, float sec);
static int  cmp(const void *a, const void *b);
static void printTopScores(int n);

// ────────────────────────────────────────────
// 遊戲常數 & 型別
// ────────────────────────────────────────────

enum Direction { UP, DOWN, LEFT, RIGHT };
#define MAX_LENGTH       400
#define EMPTY             0
#define FOOD              1
#define OBSTACLE          2
#define BOOSTER_SPEED     3
#define BOOSTER_SHRINK    4
#define BOOSTER_LIFE      5
#define SNAKE_HEAD        6
#define SNAKE_BODY        7

#define WIN_SCORE        10   // 吃 10 顆食物即破關
#define WIN_LENGTH       15   // 或長度到 15

typedef struct { int x, y; } Point;

typedef struct {
    Point *body;
    int    length;
    enum Direction dir;
    int    speed;       // µs per step
    bool   boosters[3]; // speed / shrink / life
} Snake;

typedef struct {
    int  **grid;
    int    w, h;
} Map;

typedef struct {
    int  score;
    int  lives;
    bool running;
} GameState;

// ────────────────────────────────────────────
// 函式前置宣告
// ────────────────────────────────────────────
static Snake *createSnake(int initialLength, int speed);
static Map   *createMap(int w, int h);
static void   initializeGame(Map *map, Snake *snake, GameState *st, int obstacleCnt);
static void   spawnObject(Map *map, int type);
static void   applyBooster(Snake *snake, GameState *st, int type);
static void   handleInput(Snake *snake);
static void   updateGame(Map *map, Snake *snake, GameState *st);
static void   renderGame(Map *map, Snake *snake, GameState *st, int eaten, float elapsed);
static void   gameOver(GameState *st);
static void   freeResources(Map *map, Snake *snake);

// ────────────────────────────────────────────
// 入口：runSnakeGame()
// ────────────────────────────────────────────
int runSnakeGame(void) {
    int mapW = 20, mapH = 20;
    int initialSpeed, obstacleCnt;

    printf("Welcome to Greedy Snake!\n");
    printf("Choose map size: 1)10x10  2)15x15  3)20x20 >> ");
    int choice = 3;
    scanf("%d", &choice);
    if (choice == 1) mapW = mapH = 10;
    else if (choice == 2) mapW = mapH = 15;
    else                  mapW = mapH = 20;

    printf("Choose difficulty: 1)Normal  2)Hard >> ");
    scanf("%d", &choice);
    if (choice == 1) { initialSpeed = 500000; obstacleCnt = 1; }
    else            { initialSpeed = 300000; obstacleCnt = 2; }

    // 計時器啟動
    time_t startTime = time(NULL);

    Snake *snake = createSnake(3, initialSpeed);
    Map   *map   = createMap(mapW, mapH);
    GameState st = { .score = 0, .lives = 1, .running = true };
    initializeGame(map, snake, &st, obstacleCnt);

    int eaten = 0; // 食物數

    while (st.running) {
        handleInput(snake);
        updateGame(map, snake, &st);

        // 若剛吃到食物，updateGame 會 length++ & score++
        eaten = st.score;
        float elapsed = (float)difftime(time(NULL), startTime);
        renderGame(map, snake, &st, eaten, elapsed);

        if (st.score >= WIN_SCORE || snake->length >= WIN_LENGTH) {
            printf("\nCongratulations! You cleared in %.2f seconds.\n", elapsed);
            saveScore(gPlayerName, elapsed);
            printTopScores(5);
            Sleep(2000);
            freeResources(map, snake);
            return 1;
        }
        Sleep(snake->speed / 1000);
    }
    freeResources(map, snake);
    return 0;
}

// ── 工具函式實作 ─────────────────────────────
static Snake *createSnake(int initialLength, int speed) {
    Snake *s = (Snake*)malloc(sizeof(Snake));
    s->body   = (Point*)malloc(MAX_LENGTH * sizeof(Point));
    s->length = initialLength;
    s->speed  = speed;
    s->dir    = RIGHT;
    for (int i=0;i<3;i++) s->boosters[i]=false;
    return s;
}

static Map *createMap(int w,int h){
    Map *m=(Map*)malloc(sizeof(Map));
    m->w=w; m->h=h;
    m->grid=(int**)malloc(h*sizeof(int*));
    for(int i=0;i<h;i++){ m->grid[i]=(int*)malloc(w*sizeof(int)); for(int j=0;j<w;j++) m->grid[i][j]=EMPTY; }
    return m;
}

static void initializeGame(Map *map, Snake *snake, GameState *st,int obstacleCnt){
    srand((unsigned)time(NULL));
    for(int i=0;i<snake->length;i++){
        snake->body[i].x=snake->length-1-i;
        snake->body[i].y=0;
        map->grid[0][snake->length-1-i]=(i==0?SNAKE_HEAD:SNAKE_BODY);
    }
    spawnObject(map,FOOD);
    for(int i=0;i<obstacleCnt;i++) spawnObject(map,OBSTACLE);
    for(int i=0;i<3;i++)          spawnObject(map,BOOSTER_SPEED+i);
}

static void spawnObject(Map *map,int type){
    int x,y; do{ x=rand()%map->w; y=rand()%map->h;}while(map->grid[y][x]!=EMPTY);
    map->grid[y][x]=type;
}

static void applyBooster(Snake *snake,GameState *st,int type){
    switch(type){
        case BOOSTER_SPEED: snake->speed-=100000; if(snake->speed<100000) snake->speed=100000; break;
        case BOOSTER_SHRINK: if(snake->length>3) snake->length-=2; break;
        case BOOSTER_LIFE:   st->lives++; break;
    }
}

static void handleInput(Snake *snake){
    if(_kbhit()){
        char c=_getch();
        switch(c){
            case 'w': case 'W': if(snake->dir!=DOWN)  snake->dir=UP;    break;
            case 's': case 'S': if(snake->dir!=UP)    snake->dir=DOWN;  break;
            case 'a': case 'A': if(snake->dir!=RIGHT) snake->dir=LEFT;  break;
            case 'd': case 'D': if(snake->dir!=LEFT)  snake->dir=RIGHT; break;
        }
    }
}

static void updateGame(Map *map, Snake *snake, GameState *st){
    Point newHead=snake->body[0];
    if(snake->dir==UP) newHead.y--; else if(snake->dir==DOWN) newHead.y++;
    else if(snake->dir==LEFT) newHead.x--; else newHead.x++;

    if(newHead.x<0||newHead.x>=map->w||newHead.y<0||newHead.y>=map->h){ gameOver(st); return; }
    int cell=map->grid[newHead.y][newHead.x];
    if(cell==OBSTACLE){ st->lives--; if(st->lives<=0){gameOver(st);} else {map->grid[newHead.y][newHead.x]=EMPTY; spawnObject(map,OBSTACLE);} return; }
    if(cell==FOOD){ st->score++; snake->length++; spawnObject(map,FOOD); int newSp=500000-(st->score/10)*100000; if(newSp<200000) newSp=200000; snake->speed=newSp; }
    if(cell>=BOOSTER_SPEED&&cell<=BOOSTER_LIFE){ int idx=cell-BOOSTER_SPEED; if(!snake->boosters[idx]){ snake->boosters[idx]=true; applyBooster(snake,st,cell);} }
    for(int i=0;i<snake->length;i++) if(snake->body[i].x==newHead.x&&snake->body[i].y==newHead.y){ gameOver(st); return; }

    for(int i=snake->length-1;i>0;i--) snake->body[i]=snake->body[i-1];
    snake->body[0]=newHead;

    for(int i=0;i<map->h;i++) for(int j=0;j<map->w;j++) if(map->grid[i][j]==SNAKE_HEAD||map->grid[i][j]==SNAKE_BODY) map->grid[i][j]=EMPTY;
    for(int i=0;i<snake->length;i++){ int xx=snake->body[i].x; int yy=snake->body[i].y; map->grid[yy][xx]=(i==0?SNAKE_HEAD:SNAKE_BODY);} }

static void renderGame(Map *map, Snake *snake, GameState *st, int eaten, float elapsed){
    system("cls");
    for(int i=0;i<map->h;i++){ for(int j=0;j<map->w;j++){ int c=map->grid[i][j];
            char ch='.'; if(c==FOOD)ch='F'; else if(c==OBSTACLE)ch='X'; else if(c==BOOSTER_SPEED)ch='S';
            else if(c==BOOSTER_SHRINK)ch='R'; else if(c==BOOSTER_LIFE)ch='L'; else if(c==SNAKE_HEAD)ch='H'; else if(c==SNAKE_BODY)ch='O';
            putchar(ch);} putchar('\n'); }
    printf("Player: %s  Score:%d  Lives:%d  Food:%d/10  Time: %.1fs\n",
           gPlayerName, st->score, st->lives, eaten, elapsed);
}

static void gameOver(GameState *st){ printf("\nGAME OVER! Your score is %d\n", st->score); st->running=false; }

static void freeResources(Map *map, Snake *snake){ if(map){ for(int i=0;i<map->h;i++) free(map->grid[i]); free(map->grid); free(map);} if(snake){ free(snake->body); free(snake);} }

// ────────────────────────────────────────────
// 排行榜模組實作
// ────────────────────────────────────────────
static int loadScores(Record *rec,int max){ FILE *fp=fopen("score.txt","r"); if(!fp) return 0; int n=0; while(n<max && fscanf(fp,"%31s%f",rec[n].name,&rec[n].time_sec)==2) n++; fclose(fp); return n; }
static void saveScore(const char *name,float sec){ FILE *fp=fopen("score.txt","a"); if(fp){ fprintf(fp,"%s %.2f\n",name,sec); fclose(fp);} }
static int cmp(const void *a,const void *b){ float d=((Record*)a)->time_sec - ((Record*)b)->time_sec; return (d>0)-(d<0);} // 升序
static void printTopScores(int n){ Record rec[MAX_REC]; int cnt=loadScores(rec,MAX_REC); if(cnt==0){ printf("No records yet.\n"); return;} qsort(rec,cnt,sizeof(Record),cmp);
    printf("\n=== Leaderboard (Top %d) ===\n",n);
    for(int i=0;i<cnt&&i<n;i++) printf("%2d. %-20s %.2fs\n",i+1,rec[i].name,rec[i].time_sec);
}
