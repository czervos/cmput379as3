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
#include <unistd.h>

#define AMMO 10 /* Starting number of rockets */
#define LANES 3 /* Number of lanes for the saucers to traverse */
#define LAUNCHER "|" /* Launcher shape */
#define SAUCER "<--->" /* Enemy saucer shape */
#define ROCKET "^" /* Rocket shape */
#define SAUCER_SCORE 100  /* Points for killing a saucer */
#define SAUCER_AMMO 2 /* Ammo for killing a saucer */
#define MAX_PLAYERS 1 /* Max number of players that can play */
#define MAX_ROCKETS 30 /* Max number of rockets on screen */
#define MAX_SAUCERS 10 /* Max number of saucers on screen */
#define ESCAPE_NUM 10 /* Number of escapes before game is over */
#define TUNIT 20000 /* Time unit in microseconds */


struct launcher {
        char *str; /* Look of launcher */
        int row; /* Row launcher appears on */
        int col; /* Column launcher appears on */
        int dir; /* Direction of the launcher */
};

struct rocket {
        char *str; /* Look of rocket */
        int row; /* Row location */
        int col; /* Column location */
        int live; /* Flag indicating whether or not rocket is active */
};

struct saucer {
        char *str; /* Look of rocket */
        int row; /* Row location */
        int col; /* Column location */
        int delay; /* Delay time unit for animation */
        int live; /* Flag indicating whether or not rocket is active */
};

pthread_mutex_t MX = PTHREAD_MUTEX_INITIALIZER; /* Mutex lock */
int QUIT_FLAG = 0; /* Inidicates whether game should be quit */
int LAUNCH_FLAG = 0; /* Indicates whether a rocket should be launched */
int P1AMMO = AMMO; /* Player 1's ammo count */
int P1SCORE = 0; /* Player 1's score */
int ESCAPE = 0; /* Number of escaped saucers */

void setup_curses();
void setup_players(struct launcher[]);
void setup_rockets(struct rocket[]);
void setup_saucers(struct saucer[]);
void *animate_launcher(void *);
void *animate_rocket(void *);
void *animate_saucer(void *);
void *saucer_factory(void *);
void *control_input(void *);
void strike_check(struct rocket[], struct saucer[]);
void *HUD_display();

int main(int argc, char *argv[])
{
        int i, j;
        struct launcher launcher_props[MAX_PLAYERS]; /* Player props */
        struct rocket rocket_props[MAX_ROCKETS];
        struct saucer saucer_props[MAX_SAUCERS];
        pthread_t launcher_threads[MAX_PLAYERS];
        pthread_t rocket_threads[MAX_ROCKETS];
        pthread_t saucer_factory_thread;
        pthread_t control_thread;
        pthread_t HUD_thread;
        void *animate_launcher(); // TODO are these needed?
        void *animate_rocket();

        /* Creates random seed based on pid */
        srand(getpid());

        setup_curses();
        setup_players(launcher_props);
        setup_rockets(rocket_props);
        setup_saucers(saucer_props);

        /* Set up every needed player thread */
        for (i=0; i < MAX_PLAYERS; i++) {
            /* Create thread to execute animate_launcher function, else error */
            if (pthread_create(&launcher_threads[i], NULL, animate_launcher, &launcher_props[i])) {
                fprintf(stderr, "Error creating thread\n");
                endwin();
                exit(0);
            }
        }

        // TODO error case for thread creation
        pthread_create(&control_thread, NULL, control_input, &launcher_props);
        pthread_create(&saucer_factory_thread, NULL, saucer_factory, &saucer_props);
        pthread_create(&HUD_thread, NULL, HUD_display, NULL);

/* TODO maybe have a separate thread for controls:
 * if Q is it, it would change a global quit flag that main can detect and quit the game
 * can pass the launcher_prop array into it
 * and can change a global rocket fired flag that main can fire a shot
 * this would allow main to keep checking for any hits by comparing rocket_prop array to saucer_prop array
 */

        /* Game loop */
        while (!QUIT_FLAG) {
            if (LAUNCH_FLAG == 1) {
                for (i=0; i < MAX_ROCKETS; i++) { // TODO what to do when all MAX_ROCKETS are on screen?
                    if (rocket_props[i].live == 0) {
                        rocket_props[i].row = launcher_props[0].row - 1;
                        rocket_props[i].col = launcher_props[0].col;
                        rocket_props[i].live = 1;
                        break;
                    }
                }
                LAUNCH_FLAG = 0;
                pthread_create(&rocket_threads[i], NULL, animate_rocket, &rocket_props[i]); // TODO error case
            }
            strike_check(rocket_props, saucer_props);
            if (ESCAPE == ESCAPE_NUM)
                    QUIT_FLAG = 1;
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
        /* Enables keypad input */
        keypad(stdscr, TRUE);
        /* Clear the terminal */
	clear();
        /* Parks cursor */
        move(LINES-1, COLS-1);
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
            player_array[i].col = (COLS-1)/2;
            /* Set launcher direction */
            player_array[i].dir = 0;
        }
}

/*
 * TODO add description for this function
 * Setup rocket props
 */
void setup_rockets(struct rocket rocket_array[])
{
        int i;
        for (i=0; i < MAX_ROCKETS; i++) {
            rocket_array[i].str = ROCKET;
            rocket_array[i].row = 0;
            rocket_array[i].col = 0;
            rocket_array[i].live = 0;
        }
}

/*
 * TODO add description for this function
 * Setup saucer props
 */
void setup_saucers(struct saucer saucer_array[])
{
        int i;
        for (i=0; i < MAX_SAUCERS; i++) {
            saucer_array[i].str = SAUCER;
            saucer_array[i].row = 0;
            saucer_array[i].col = 0;
            saucer_array[i].delay = 0;
            saucer_array[i].live = 0;
        }
}

/*
 * TODO add description for this function
 * Animate the player's prop
 */
void *animate_launcher(void *arg)
{
        struct launcher *player = arg; /* Points to launcher struct passed into function */

        /* Prints launcher at initial position */
        pthread_mutex_lock(&MX);
        usleep(TUNIT);
        mvprintw(player->row, player->col, player->str);
        move(LINES-1, COLS-1); /* Parks cursor */
        refresh();
        pthread_mutex_unlock(&MX);

        while(1) {
            /* Brief pause needed for curses not to output garbage when moving */
            usleep(TUNIT);
            /* Waits for input from user to change the direction of launcher */
            if (player->dir != 0) {
                /* Check if launcher is on one of the sides of the terminal */
                if ((player->dir == 1) && (player->col != COLS-1)) {
                    /* Sets launcher column position to new position based on direction */
                    player->col += player->dir; 
                }
                else if ((player->dir == -1) && (player->col != 0)) {
                    /* Sets launcher column position to new position based on direction */
                    player->col += player->dir; 
                }
                pthread_mutex_lock(&MX);
                /* 
                 * If launcher is at column 0, do not print a space before launcher since negative numbers are not
                 * mapped on the terminal
                 */
                if (player->col == 0) {
                    move(player->row, player->col); /* Moves cursor to left of current launcher location */
                    addstr(player->str); /* Puts the launcher string at the new launcher location */
                    addch(' '); /* Spaces clear any old instance of the launcher */
                }
                else {
                    move(player->row, player->col-1); /* Moves cursor to left of current launcher location */
                    addch(' '); /* Spaces clear any old instance of the launcher */
                    addstr(player->str); /* Puts the launcher string at the new launcher location */
                    addch(' ');
                }
                //refresh();
                move(LINES-1, COLS-1); /* Parks cursor */
                refresh(); /* Refreshes the screen */
                pthread_mutex_unlock(&MX);
                
                player->dir = 0; /* Sets direction to 0 */
            }
        }
}

/*
 * TODO add description for this function
 * Animate the rocket prop
 */
void *animate_rocket(void *arg)
{
        struct rocket *myrocket = arg;

        /* Displays initial rocket so that it doesn't cover the launcher */
        usleep(TUNIT);
        pthread_mutex_lock(&MX);
        move(myrocket->row, myrocket->col);
        addstr(myrocket->str);
        myrocket->row -= 1; /* Increments rocket position */
        move(LINES-1, COLS-1);
        refresh();
        pthread_mutex_unlock(&MX);

        /* Animates rocket upward while rocket position is not 0 */
        while(myrocket->row >= 0) {
            usleep(TUNIT);
            pthread_mutex_lock(&MX);
            move(myrocket->row+1, myrocket->col); /* Moves to old instance of rocket */
            addch(' '); /* Removes it */
            move(myrocket->row, myrocket->col); /* Moves to new rocket location */
            addstr(myrocket->str); /* Prints it */
            myrocket->row -= 1; /* Increments rocket position */
            move(LINES-1, COLS-1); /* Park cursor */
            refresh();
            pthread_mutex_unlock(&MX);
        }

        /* Remove last visible instance of rocket */
        usleep(TUNIT);
        pthread_mutex_lock(&MX);
        move(myrocket->row+1, myrocket->col);
        addch(' ');
        move(LINES-1, COLS-1);
        refresh();
        pthread_mutex_unlock(&MX);

        /* Cleanup before exiting thread */
        myrocket->live = 0;
        myrocket->row = 0;
        myrocket->col = 0;
        pthread_exit(NULL); // TODO difference between this and pthread_cancel in terms of threads array?
}

/*
 * TODO add description for this function
 * Animate the saucer prop
 */
void *animate_saucer(void *arg)
{
        struct saucer *mysaucer = arg;
        char *blank = "     "; // TODO make this dynamic?

        /* 
         * While saucer position + length of saucer is less than the terminal width
         * and saucer is still live
         * Note: terminal width is from 0 to COLS; visible terminal width is from 0 to COLS-1
         */
        while(((mysaucer->col + (strlen(SAUCER) - 1)) < COLS) && mysaucer->live == 1) {
            /* Delay sets the speed of the launcher crossing the screen */
            usleep(mysaucer->delay * TUNIT);

            pthread_mutex_lock(&MX);
            /* Draws first saucer without a space in front of it */
            if (mysaucer->col == 0) {
                move(mysaucer->row, mysaucer->col);
                addstr(mysaucer->str);
                move(LINES-1, COLS-1);
            }
            /* Draws remaining instances of the saucer */
            else {
                move(mysaucer->row, mysaucer->col-1); /* Move cursor to old location */
                addch(' '); /* Remove front of old launcher */
                addstr(mysaucer->str); /* Draw new launcher at new position */
            }
            move(LINES-1, COLS-1); /* Park cursor */
            refresh();
            pthread_mutex_unlock(&MX);

            mysaucer->col += 1; /* Increment saucer position */
        }

        /* Increment escape count if saucer escaped */
        if (mysaucer->live == 1)
                ESCAPE += 1;

        /* Remove saucer upon exiting screen */
        pthread_mutex_lock(&MX);
        move(mysaucer->row, mysaucer->col-1); /* Go to saucer's last position */
        addstr(blank); /* Clear the saucer */
        move(LINES-1, COLS-1); /* Park cursor */
        refresh();
        pthread_mutex_unlock(&MX);

        /* Cleanup and exit thread */
        mysaucer->live = 0;
        mysaucer->row = 0;
        mysaucer->col = 0;
        mysaucer->delay = 0;
        pthread_exit(NULL);
}

/*
 * TODO add description for this function
 * Creates saucers
 */
void *saucer_factory(void *arg)
{
        struct saucer *saucer_array = arg;
        pthread_t saucer_threads[MAX_SAUCERS];
        void *animate_saucer();
        int i;

        /* Factory loop */
        while (1) {
            /* Sleeps for random period between 1 and 5 seconds */
            sleep(1+ rand()%5);

            for (i=0; i < MAX_SAUCERS; i++) { // TODO what to do when all MAX_SAUCERS are on screen?
                if (saucer_array[i].live == 0) {
                    /* Generates random row value between 0 and LANES-1 */
                    /* Thus generates LANES number of lanes for the saucers to traverse */
                    saucer_array[i].row = rand()%LANES;
                    saucer_array[i].col = 0;
                    /* Generates delay value of 1 plus random # between 0 and 14 - ie random number between 1 and 15 */
                    saucer_array[i].delay = 1 + (rand()%15);
                    saucer_array[i].live = 1;
                    break;
                }
            }
            pthread_create(&saucer_threads[i], NULL, animate_saucer, &saucer_array[i]); // TODO error case
        }
        // TODO exit the thread
}

/*
 * TODO add description for this function
 * Detects control input
 */
void *control_input(void *arg)
{
        struct launcher *launcher_array = arg;
        int c;

        /* Input loop */
        while (1) {
            c = getch();
            /* Quit the game */
            // TODO switch to switch
            if (c == 'Q') {
                QUIT_FLAG = 1;
                break;
            }
            if (c == KEY_LEFT)
                    launcher_array[0].dir = -1;
            if (c == KEY_RIGHT)
                    launcher_array[0].dir = 1;
            if (c == ' ') {
                if (P1AMMO != 0) {
                    P1AMMO -= 1;
                    LAUNCH_FLAG = 1;
                }
            }
        }
        pthread_exit(NULL);
}

/*
 * TODO add description for this function
 * Detects rocket strike on saucer
 */
void strike_check(struct rocket rocket_array[], struct saucer saucer_array[])
{
        int i, j;

        /* Iterate through all rockets */
        for (i=0; i < MAX_ROCKETS; i++) {
            /* If rocket is live */
            if (rocket_array[i].live == 1) {
                /* Iterate through saucers */
                for (j=0; j < MAX_SAUCERS; j++) {
                    /* If saucer is live */
                    if (saucer_array[j].live == 1) {
                        /* If rocket and saucer are on same row */
                        if (rocket_array[i].row == saucer_array[j].row) {
                            /* If rocket is within the saucer's position */
                            if ((rocket_array[i].col >= saucer_array[j].col) && 
                                (rocket_array[i].col <= (saucer_array[j].col + 4))) {
                                /* Set saucer to dead */
                                saucer_array[j].live = 0;
                                /* Points for killing saucer */
                                P1SCORE += SAUCER_SCORE;
                                /* Ammo for killing saucer */
                                P1AMMO += SAUCER_AMMO;
                                // TODO set rocket to dead
                            }
                        }
                    }
                }
            }
        }
}

/*
 * TODO add description for this function
 * Displays the HUD
 */
void *HUD_display()
{
        char *blank = "                                                         "; // TODO make this dynamic
        while(1) {
            usleep(TUNIT);
            pthread_mutex_lock(&MX);
            mvprintw(LINES-1,0, blank);
            mvprintw(LINES-1,0,"Player1 - Score: %d - Ammo: %d - Escaped Saucers: %d", P1SCORE, P1AMMO, ESCAPE);
            move(LINES-1, COLS-1); /* Park cursor */
            refresh();
            pthread_mutex_unlock(&MX);
        }

// TODO quit thread
}
