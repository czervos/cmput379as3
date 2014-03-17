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

void setup();

int main(int argc, char *argv[])
{
        int c; /* User input character */

        setup();

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
void setup()
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
