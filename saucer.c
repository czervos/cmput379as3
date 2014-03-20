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

struct launcher {
        char *str; /* Look of launcher */
        int row; /* Row launcher appears on */
        int col; /* Column launcher appears on */
        int dir; /* Direction of the launcher */
        int delay; /* Launcher's delay in time units */
};

pthread_mutex_t MX = PTHREAD_MUTEX_INITIALIZER; /* Mutex lock */

void setup_curses();
void setup_players(struct launcher[]);
void *animate_launcher(void*);

int main(int argc, char *argv[])
{
        int c; /* User input character */
        int i;
        struct launcher launcher_props[MAX_PLAYERS]; /* Player props */
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
void setup_players(struct launcher player_array[])
{
        int i;

        /* For every launcher */
        for (i=0; i < MAX_PLAYERS; i++) {
            /* Set string of launcher string */
            player_array[i].str = LAUNCHER;
            /* Set launcher row position */
            player_array[i].row = LINES-2;
            /* Set launcher column posiition */
            player_array[i].col = COLS/2;
        }
}

/*
 * TODO add description for this function
 * Animate the player's prop
 */
void *animate_launcher(void *arg)
{
        struct launcher *player = arg; /* Points to launcher struct passed into function */
        int col = (COLS/2); /* Initializes launcher column position to middle of screen */

         mvprintw(player->row, col, LAUNCHER); /* Prints launcher at initial position */

        while(1) {
            /* Waits for input from user to change the direction of launcher */
            while(player->dir != 0) {
                player->col += player->dir; /* Sets launcher column position to new position based on direction */
                pthread_mutex_lock(&MX);
                move(player->row, player->col); /* Moves cursor to current launcher location */
                addch(' '); /* Puts a space there */
                addstr(player->str); /* Puts the launcher string at the new launcher location */
                addch(' '); /* Puts a space after the launcher string */
                move(0, 0); /* Parks cursor */
                refresh(); /* Refreshes the screen */
                pthread_mutex_unlock(&MX);
                player->dir = 0; /* Sets direction to 0 */
            }
        }

}
