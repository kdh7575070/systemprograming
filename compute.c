#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <termio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include "set_ticker.c"
#define MAX_QUEUE_SIZE 6

struct user
{
    char name[20];
    char lastName[20];
};
struct user firstUser;

/* functions */
void startMenu();
void startEngine();

/* global variable */
int isObstacle;
int diX = 5, prize = 0, usedPrize = 0, score = 0, delayTime = 300, gameStatus = 1, cactusNum = 0;
int x, y, maxX, maxY, diY;
bool isBottom = true;
bool isJumping = false;
bool beforeInput_jump = false;
char userInput;
int highScore;
int rear, front;
char getMode;

void startMenu()
/* the very first menu */
{
    maxX = getmaxx(stdscr) / 2;
    maxY = getmaxy(stdscr) / 2;
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(3));
    showTeam1(maxY, maxX);
    attroff(COLOR_PAIR(3));

    /* get information from user */
    mvprintw(maxY + 2, maxX - 24, "Write inputs and press Enter to start Game.");
    mvprintw(maxY + 3, maxX - 22, "In case you wanna quit, put CTRL+C !");
    mvprintw(maxY + 4, maxX - 20, "You can jump with space key!");
    mvprintw(maxY + 6, maxX - 12, "Name: ");
    getstr(firstUser.name);
    mvprintw(maxY + 7, maxX - 12, "Last name: ");
    getstr(firstUser.lastName);

    /* select mode */
    mvprintw(maxY + 8, maxX - 12, "Select mode");
    mvprintw(maxY + 9, maxX - 12, "[ a ] : dino(default)");
    mvprintw(maxY + 10, maxX - 12, "[ b ] : human");
    mvprintw(maxY + 11, maxX - 12, "[ c ] : alpaca");
    mvprintw(maxY + 12, maxX - 12, "[ d ] : amongus");
    getMode = getch();
    noecho();

    /* show scoreboard */
    clear();
    highScore = showScoreBoard();

    /* game start */
    startEngine();
}

void startEngine()
/* The main engine! */
{
    srand(time(NULL));
    clear();
    nodelay(stdscr, TRUE);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);

    /* init game */
    isBottom = true;          /* character is on the ground */
    isJumping = false;        /* character is not jumping   */
    beforeInput_jump = false; /* character was not jumping  */
    gameStatus = 1;           /* game is in process	       */
    score = 0;                /* initial score    	       */
    delayTime = 300;          /* game speed 		       */

    maxX = getmaxx(stdscr);
    x = maxX - 20;           /* x: obstacle location    */
    y = getmaxy(stdscr) - 6; /* y: obstacle location    */
    diY = y;                 /* diY: character location */

    queue = Queue; /* obstacle queue */
    qinitialize(); /* init queue     */
    /* first obstacle: cactus */
    cactus.x = x;
    cactus.y = y;
    cactus.type = cactusNum;
    addq(cactus);

    /* notify user input with a signal */
    signal(SIGIO, on_input);   /* keyboard sends SIGIO, install a handler(on_input) */
    enable_kbd_signals();      /* turn on kbd signals      */
    signal(SIGALRM, on_alarm); /* install alarm handler    */
    set_ticker(delayTime);     /* start ticking     	      */
    while (gameStatus)
    {
        pause(); /* wait for signal */
    }
    signal(SIGALRM, SIG_IGN);
    signal(SIGIO, SIG_IGN);

    /* When code reaches here, means that user is loss */
    endGame(score, diY, diX);
    attroff(COLOR_PAIR(1));
}