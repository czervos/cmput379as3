/* 
 * CMPUT 379 - Assignment 3
 * Costa Zervos 
 */

/*
TODO

MAIN THREAD:
Main thread will keep track of user input

PROP THREADS:
Launcher threads - max 2?
Rocket threads - max 25?
Saucer threads - max 10?

These threads will animate the props and make Rockets and Saucers disappear if they hit the end of the screen

STRIKE THREAD
Will loop through all the rocket structs and compare the row/col with the row/col of each saucer; if they match
a strike has occured and will set a flag in both. The animate thread will detect that the flag has been set
and make them disappear.

Bottom right corner of terminal (LINES-1 COLS-1)
Top left corner of terminal (0 0)

*/


#include <stdio.h>
#include <curses.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define AMMO 10 /* Total number of rockets */
#define LANES 3 /* Top lines enemy saucers can occupy */
#define SAUCER "<--->" /* Enemy saucer shape */ // TODO remove and change to a string like launcher
#define MAX_PLAYERS 1 /* Max number of players that can play */
#define MAX_THREADS 1 /* Max number of threads needed */

struct launcher {
        char *str; /* Look of launcher */
        int row; /* Row launcher appears on */
        int col; /* Column launcher appears on */
        int dir; /* Direction of the launcher */
};

pthread_mutex_t MX = PTHREAD_MUTEX_INITIALIZER; /* Mutex lock */

void setup_curses();
void setup_players(struct launcher[], char *);
void *animate_launcher(void *);

int main(int argc, char *argv[])
{
        int c; /* User input character */
        int i;
        struct launcher launcher_props[MAX_PLAYERS]; /* Player props */
        pthread_t threads[MAX_THREADS];
        void *animate_launcher();
        char *launcher = "|"; /* User launcher shape */

        setup_curses();
        setup_players(launcher_props, launcher);

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
            // TODO switch to switch
            if (c == 'Q')
                    break;
            if (c == ',')
                    launcher_props[0].dir = -1;
            if (c == '.')
                    launcher_props[0].dir = 1;
        }
// TODO must close threads when done
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
        /* Parks cursor */
        move(LINES-1, COLS-1);
}

/*
 * TODO add description for this function
 * Setup player props
 */
void setup_players(struct launcher player_array[], char *launcher_str)
{
        int i;

        /* For every launcher */
        for (i=0; i < MAX_PLAYERS; i++) {
            /* Set string of launcher string */
            player_array[i].str = launcher_str;
            /* Set launcher row position */
            player_array[i].row = LINES-2;
            /* Set launcher column posiition */
            player_array[i].col = (COLS-1)/2;
            /* Set launcher direction */
            player_array[i].dir = 0;
        }
}

/*
 * TODO add description for this function
 * Animate the player's prop
 */
void *animate_launcher(void *arg)
{
        struct launcher *player = arg; /* Points to launcher struct passed into function */

        // TODO why doesn't this mvprintw work?
        //mvprintw(player->row, player->col, player->str); /* Prints launcher at initial position */
        //pthread_mutex_lock(&MX);
        //move(player->row, player->col); /* Go to current location of launcher */
        //addstr(player->str); /* Puts the launcher string at the launcher location */
        //move(LINES-1, COLS-1); /* Parks cursor */
        //refresh(); /* Refreshes the screen */
        //pthread_mutex_unlock(&MX);

        while(1) {
            /* Waits for input from user to change the direction of launcher */
            while(player->dir != 0) {
                pthread_mutex_lock(&MX);
                    move(player->row, player->col); /* Go to last location of launcher */
                    addch(' '); /* Replace with a space */
                    refresh();
                    player->col += player->dir; /* Sets launcher column position to new position based on direction */
                    move(player->row, player->col); /* Moves cursor to current launcher location */
                    addstr(player->str); /* Puts the launcher string at the new launcher location */
                    addch(' '); /* Replace with a space */
                    refresh();
                    move(LINES-1, COLS-1); /* Parks cursor */
                    refresh(); /* Refreshes the screen */
                pthread_mutex_unlock(&MX);

                player->dir = 0; /* Sets direction to 0 */
            }
        }
}

//              pthread_mutex_lock(&MX);
//              move(player->row, player->col); /* Go to last location of launcher */
//              addch(' '); /* Replace with a space */
//              player->col += player->dir; /* Sets launcher column position to new position based on direction */
//              move(player->row, player->col); /* Moves cursor to current launcher location */
//              addstr(player->str); /* Puts the launcher string at the new launcher location */
//              move(LINES-1, COLS-1); /* Parks cursor */
//              refresh(); /* Refreshes the screen */
//              pthread_mutex_unlock(&MX);
// player->dir = 0; /* Sets direction to 0 */

