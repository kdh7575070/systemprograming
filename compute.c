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

typedef struct
{
	int x;
	int y;
	int type;
} obstacle;
obstacle Queue[MAX_QUEUE_SIZE];
obstacle *queue;
obstacle cactus;

/* functions */
void startMenu();
void startEngine();




void show();










void qinitialize();
int isEmpty();
int isFull();
void addq(obstacle item);
void deleteq();
void updateObstacle();
void showObstacle();

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


void show()
/* displayed on the screen */
{
	/* Show day or night */
	if (((score / 100)) % 2)
	/* night */
	{
		clear();
		attron(COLOR_PAIR(3));
		moon(10, (maxX / 2) - 10);
	}
	else
	/* day */
	{
		attron(COLOR_PAIR(1));
		sun(10, (maxX / 2) - 10);
	}

	/* increase score */
	score++;
	
	/* show informations */
	mvprintw(1, 6, "%s %s", firstUser.name, firstUser.lastName);
	mvprintw(1, getmaxx(stdscr) - 9, "%d : %d", highScore, score);

	/* show the border as a solid line */
	box(stdscr, ACS_VLINE, ACS_HLINE);

	/* for clearing screen */
	clearDinasourUp(diY, diX);
	
	if (userInput == ' ' && isBottom && !isJumping)
	/* if input is equal to ' ', character is on ground and character is not jumping then character will jump */
	{
		isJumping = true;	 /* character will jump */
		isBottom = false;	 /* character will not touch the ground */
		beforeInput_jump = true;/* character will jump */
	}

	if (isJumping)
	/* jump */
	{
		diY -= 7;
	}
	else if (beforeInput_jump)
	/* if character is above ground and character will fall */
	{
		beforeInput_jump = false;
	}
	else
	/* fall */
	{
		diY += 7;
		clearDinasourUp(diY, diX);
	}


	if (diY >= y)
	/* if character is below ground */
	{
		diY = y;	  /* character is on the ground */
		isBottom = true;
	}


	if (isJumping)
	/* if character is jumping */
	{
		clearDinasourDown(diY, diX); /* erase old position(bottom)  */
		isJumping = false;   	      /* character will fall         */
	}
	else{
		clearDinasourUp(diY, diX);   /* erase old position(jumping) */
	}
	
	
	if (diY <= 7)
	/* if character is above ground */
	{
		isJumping = false; /* character will fall */
	}
	
	/* add obstacle in queue */
	isObstacle = rand() % 10; 		   /* create obstacle or not 		     */
	if (isObstacle == 1)      		   /* probability of creating obstacle: 10% */
	{					
		cactusNum = rand() % 3;	   /* decide the type of cactus             */
		cactus.x = getmaxx(stdscr) - 20;  /* initial x-axis position of cactus     */
		cactus.y = y;		           /* initial y-axis position of cactus     */
		cactus.type = cactusNum;	   /* type of cactus                        */
		addq(cactus);
	}
	
	/* first: closest obstacle from dinosaur */
	obstacle first = queue[(front + 1) % MAX_QUEUE_SIZE]; 
	/* show obstacle */
	showObstacle();
	/* collision check */
	gameStatus = checkGame(first.y, first.x, diY, diX);
	/* show ground */
	mvhline(y + 1, 1, '-', getmaxx(stdscr) - 3);
	/* move obstacle left */
	updateObstacle();
	/* show character you chose*/
	switch (getMode)
	{
	case 'b':
		showPeople(diY, diX);
		break;
	case 'c':
		showAlphaca(diY, diX);
		break;
	case 'd':
		showAmongus(diY, diX);
		break;
	default:
		showDinasour(diY, diX);
		break;
	}
	refresh();
	
	usleep(delayTime);
	delayTime = computeTime(delayTime);
	userInput = 'q';
}

/* generate obstacle randomly using circular queue */
void qinitialize()
{
	front = rear = 0;
}
int isEmpty()
{
	return (front == rear);
}
int isFull()
{
	return ((rear + 1) % MAX_QUEUE_SIZE == front);
}
void addq(obstacle item)
{
	if (isFull())
		return;
	rear++;
	rear = rear % MAX_QUEUE_SIZE;
	queue[rear] = item;
}
void deleteq()
{
	front = (front + 1) % MAX_QUEUE_SIZE;
}
void updateObstacle()
/* erase all obstacles in screen 
 * move all obstacles left
 * if obstacle is out of screen, get rid of it from queue
 */
{
	int i = front;
	for (; i != rear;)
	{
		i = (i + 1) % MAX_QUEUE_SIZE;
		clearCactus1(queue[i].y, queue[i].x);
		queue[i].x -= 7;
		if (queue[i].x < 0)
		{
			clearCactus1(queue[i].y, queue[i].x);
			deleteq();
		}
	}
}
void showObstacle()
/* show all obstacles in queue 
 * show different type of cactus depending on 'type' member of struct obstacle 
 */
{
	int i = front;
	for (; i != rear;)
	{
		i = (i + 1) % MAX_QUEUE_SIZE;
		if (queue[i].type == 0)
			cactus1(queue[i].y, queue[i].x);
		else if (queue[i].type == 1)
			cactus2(queue[i].y, queue[i].x);
		else
			cactus3(queue[i].y, queue[i].x);
	}
}

int checkGame(int y, int x, int diY, int diX)
/* collision check 
 * x, y     : obstacle location
 * diY, diX : character location
 * if character collides with obstacle, return 0
 */
{
	if (diY == y)
	{
		if (abs((diX + 14) - x) <= 3)
		{
			return 0;
		}
	}
	return 1;
}
