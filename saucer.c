/* 
 * CMPUT 379 - Assignment 3
 * Costa Zervos 
 */

#include <stdio.h>
#include <curses.h>

#define AMMO 10 /* Total number of rockets */
#define LANES 3 /* Top lines enemy saucers can occupy */
#define SAUCER "<--->" /* Enemy saucer shape */
#define LAUNCHER "|" /* User launcher shape */
#define MAX_PLAYERS 1 /* Max number of players that can play */

struct propset {
        char *str; /* Look of prop */
        int row; /* Row prop appears on */
        int delay; /* Prop's delay in time units */
};

void setup_curses();
void setup_players(struct propset[]);

int main(int argc, char *argv[])
{
        int c; /* User input character */
        struct propset launcher_props[MAX_PLAYERS]; /* Player props */

        setup_curses();

        /* Game loop */
        while (1) {
            c = getch();
            /* Quit the game */
            if (c == 'Q')
                    break;
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

        for (i=0; i < MAX_PLAYERS; i++) {
            player_array[i].str = LAUNCHER;
            player_array[i].row = 0;
        }
}
