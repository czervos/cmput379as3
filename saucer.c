/* 
 * CMPUT 379 - Assignment 3
 * Costa Zervos 
 */

#include <stdio.h>
#include <curses.h>
#include <pthread.h>
#include <stdlib.h>

#define AMMO 10 /* Total number of rockets */
#define LANES 3 /* Top lines enemy saucers can occupy */
#define SAUCER "<--->" /* Enemy saucer shape */
#define LAUNCHER "|" /* User launcher shape */
#define MAX_PLAYERS 1 /* Max number of players that can play */
#define MAX_THREADS 1 /* Max number of threads needed */

struct propset {
        char *str; /* Look of prop */
        int row; /* Row prop appears on */
        int dir; /* Direction of the prop */
        int delay; /* Prop's delay in time units */
};

pthread_mutex_t MX = PTHREAD_MUTEX_INITIALIZER; /* Mutex lock */

void setup_curses();
void setup_players(struct propset[]);
void *animate_launcher(void*);

int main(int argc, char *argv[])
{
        int c; /* User input character */
        int i;
        struct propset launcher_props[MAX_PLAYERS]; /* Player props */
        pthread_t threads[MAX_THREADS];
        void *animate_launcher();

        setup_curses();
        setup_players(launcher_props);

        /* Set up every needed thread */
        for (i=0; i < MAX_THREADS; i++) {
            /* Create thread to execute animate_launcher function, else error */
            if (pthread_create(&threads[i], NULL, animate_launcher, &launcher_props)) {
                fprintf(stderr, "Error creating thread\n");
                endwin();
                exit(0);
            }
        }

        /* Game loop */
        while (1) {
            c = getch();
            /* Quit the game */
            if (c == 'Q')
                    break;
            if (c == ',')
                    launcher_props[0].dir = -1;
            if (c == '.')
                    launcher_props[0].dir = 1;
        }
        endwin();
        return 0;
}

/*
 * TODO add description for this function
 * Sets up curses 
 */
void setup_curses()
{
        /* Initialize screen */
	initscr();
        /* Key input raises event */
	crmode();
        /* Key input isn't printed to screen */
	noecho();
        /* Clear the terminal */
	clear();
}

/*
 * TODO add description for this function
 * Setup player props
 */
void setup_players(struct propset player_array[])
{
        int i;

        /* For every player prop */
        for (i=0; i < MAX_PLAYERS; i++) {
            /* Set string of prop to launcher string */
            player_array[i].str = LAUNCHER;
            /* Set launcher row position */
            player_array[i].row = LINES-2;
        }
}

/*
 * TODO add description for this function
 * Animate the player's prop
 */
void *animate_launcher(void *arg)
{
        struct propset *prop = arg; /* Points to prop struct passed into function */
        int col = (COLS/2); /* Initializes prop column position to middle of screen */

        mvprintw(prop->row, col, LAUNCHER); /* Prints prop at initial position */

        while(1) {
            /* Waits for input from user to change the direction of launcher */
            while(prop->dir != 0) {
                col += prop->dir; /* Sets prop column position to new position based on direction */
                pthread_mutex_lock(&MX);
                move(prop->row, col); /* Moves cursor to current prop location */
                addch(' '); /* Puts a space there */
                addstr(prop->str); /* Puts the prop string at the new prop location */
                addch(' '); /* Puts a space after the prop string */
                move(LINES-1, COLS-1); /* Parks cursor */
                refresh(); /* Refreshes the screen */
                pthread_mutex_unlock(&MX);
                prop->dir = 0; /* Sets direction to 0 */
            }
        }

}
