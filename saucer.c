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

in strike_check

	when a strike is successfully detected:
		1. on a random chance, iterate through
		   the ammo_drop_array and look for the first
		   live=0 ammo drop
		2. set that ammo drop to the coordinates of where the strike
		   happend, set it to live=1, and set it to new=1
		3. in some other method that is continually running a for loop
 		   that checks to see if any of the rockets are live=1 and new=1
			a. if it finds one, set that new=0 and create an animate
			   ammo_drop thread for it.
                4. have another method continually running called 
                   ammo_collection check which will check to see if its coordinates
		   match the coordinates of the launcher: if it does then 
		   it is a sucessful collection and the user's ammo count should
		   go up.
		5. animate ammo drop will also check to see if the drop passed
	           the launcher. if it has, set the ammo_drop.live = 0 and kill
		   that animate thread.
		6. Out of ammo fail condition needs to make sure no ammo drops
		   are in the proccess of falling instead of checking to see if
		   any rockets are flying.
		7. Also remove rocket strike ammo increment.

Bottom right corner of terminal (LINES-1 COLS-1)
Top left corner of terminal (0 0)

0x20 space character - memset to dynamically make blank

BUG FOR THINGS STICKING AROUND:
The issue is that i create a new thread when something is considered dead. So during the
animation function for say a saucer, it detects its dead and starts exiting, such as priting the explosion
and then clearing it out. But during this time, the thread can potentially get cancelled by having a new
thread created in its place, resulting in a saucer or explosion staying behind.

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
#define EXPLOSION "~%@%~" /* Explosion shape */
#define AMMO_DROP "#" /* Ammo drop shape */
#define SAUCER_SCORE 100  /* Points for killing a saucer */
#define SAUCER_AMMO 2 /* Ammo for killing a saucer */
#define MAX_PLAYERS 1 /* Max number of players that can play */
#define MAX_ROCKETS 30 /* Max number of rockets on screen */
#define MAX_SAUCERS 10 /* Max number of saucers on screen */
#define MAX_AMMO_DROPS 10 /* Max number of ammo drops on screen */
#define ESCAPE_NUM 10 /* Number of escapes before game is over */
#define TIME 60 /* Gameplay time in seconds */
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
        char *str; /* Look of saucer */
        int row; /* Row location */
        int col; /* Column location */
        int delay; /* Delay time unit for animation */
        int live; /* Flag indicating whether or not saucer is active */
        int thread; /* Flag indicating whether or not this struct is being used by a thread */
};

struct ammo_drop {
        char *str; /* Look of ammo drop */
        int row; /* Row location */
        int col; /* Column location */
        int live; /* Flag indicating whether or not ammo drop is active */
        int new; /* Binary value to determine whether ammo drop is newly created */
        int thread; /* Flag indicating whether or not this struct is being used by a thread */
};

pthread_mutex_t MX = PTHREAD_MUTEX_INITIALIZER; /* Mutex lock */
int QUIT_FLAG = 0; /* Inidicates whether game should be quit */
int LAUNCH_FLAG = 0; /* Indicates whether a rocket should be launched */
int AMMO_FLAG = 0; /* Indicates whether ammo should be dropped */
int P1AMMO = AMMO; /* Player 1's ammo count */ // TODO put in own struct - requires changing stuff control_input function
int P1SCORE = 0; /* Player 1's score */ // TODO put in own struct
int ESCAPE = 0; /* Number of escaped saucers */
int TIMER = TIME; /* Time remaining in the game */ // TODO put in own struct
int LAUNCHER_COL = 0;

void setup_curses();
void setup_players(struct launcher[]);
void setup_rockets(struct rocket[]);
void setup_saucers(struct saucer[]);
void setup_ammo_drops(struct ammo_drop[]);
void *animate_launcher(void *);
void *animate_rocket(void *);
void *animate_saucer(void *);
void *animate_ammo_drops(void *);
void *saucer_factory(void *);
void *rocket_factory(void *);
void *control_input(void *);
void *ammo_drop_factory(void *);
void strike_check(struct rocket[], struct saucer[], struct ammo_drop[]);
void ammo_collection_check(struct ammo_drop[], struct launcher);
void *HUD_display();
void *countdown_timer();
void splash_screen();
void instruction_screen();
void victory_screen();
void escape_fail_screen();
void no_ammo_fail_screen();



int main(int argc, char *argv[])
{
        int i, j;
        struct launcher launcher_props[MAX_PLAYERS]; /* Player props */
        struct rocket rocket_props[MAX_ROCKETS];
        struct saucer saucer_props[MAX_SAUCERS];
        struct ammo_drop ammo_drop_props[MAX_AMMO_DROPS];
        pthread_t launcher_threads[MAX_PLAYERS];

        pthread_t saucer_factory_thread;
        pthread_t ammo_drop_factory_thread;
        pthread_t rocket_factory_thread;
        pthread_t control_thread;
        pthread_t HUD_thread;
        pthread_t timer_thread;
        void *animate_launcher(); // TODO are these needed?
        char game_over[] = "Game Over!";

        /* Creates random seed based on pid */
        srand(getpid());

        /* Sets ups curses and the various structure arrays */
        setup_curses();
        setup_players(launcher_props);
        setup_rockets(rocket_props);
        setup_saucers(saucer_props);
        setup_ammo_drops(ammo_drop_props);

        splash_screen();

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
        pthread_create(&timer_thread, NULL, countdown_timer, NULL);
        pthread_create(&ammo_drop_factory_thread, NULL, ammo_drop_factory, &ammo_drop_props);
        pthread_create(&rocket_factory_thread, NULL, rocket_factory, &rocket_props);

        /* Game loop */
        while (!QUIT_FLAG) {

            /* Checks if any live rockets hit a saucer */
            strike_check(rocket_props, saucer_props, ammo_drop_props);

            /* Checks if any ammo drop was collected */
            ammo_collection_check(ammo_drop_props, launcher_props[0]);
            /* Checks escape endgame condition */
            if (ESCAPE == ESCAPE_NUM)
                    QUIT_FLAG = 1;
            /* Checks if ammo depleted and there are no live rockets for endgame condition */
            if (P1AMMO == 0) {
                usleep(TUNIT);
                for (i=0; i < MAX_ROCKETS; i++) {
                    if (rocket_props[i].live == 1)
                            break;
                    if (i == MAX_ROCKETS-1) {
                        for (j=0; j < MAX_AMMO_DROPS; j++) {
                            if (ammo_drop_props[j].live == 1)
                                    break;
                            if (j == MAX_AMMO_DROPS-1)
                                    QUIT_FLAG = 1;
                        }
                    }
                }
            }
            if (TIMER == 0)
                    QUIT_FLAG = 1;
        }

        mvprintw(LINES/2, (COLS-strlen(game_over))/2, game_over);
        sleep(5);

        /* Set all rockets to 0 for cleanup */
        for (i=0; i < MAX_ROCKETS; i++)
                rocket_props[i].live = 0;

        if (TIMER == 0)
                victory_screen();
        else if (ESCAPE == ESCAPE_NUM)
                escape_fail_screen();
        else if (P1AMMO == 0)
                no_ammo_fail_screen();
// TODO must close threads when done -- I think at this point most threads close themselves - double check
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
            player_array[i].row =  LINES-2;
            /* Set launcher column posiition */
            player_array[i].col = (COLS-1)/2;
            LAUNCHER_COL = player_array[i].col;
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
            saucer_array[i].thread = 0;
        }
}

/*
 * TODO add description for this function
 * Setup ammo drops
 */
void setup_ammo_drops(struct ammo_drop ammo_drop_array[])
{
        int i;
        for (i=0; i < MAX_AMMO_DROPS; i++) {
            ammo_drop_array[i].str = AMMO_DROP;
            ammo_drop_array[i].row = 0;
            ammo_drop_array[i].col = 0;
            ammo_drop_array[i].live = 0;
            ammo_drop_array[i].new = 0;
            ammo_drop_array[i].thread = 0;
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

        while(!QUIT_FLAG) {
            /* Brief pause needed for curses not to output garbage when moving */
            usleep(TUNIT);
            /* Waits for input from user to change the direction of launcher */
            if (player->dir != 0) {
                /* Check if launcher is on one of the sides of the terminal */
                if ((player->dir == 1) && (player->col != COLS-1)) {
                    /* Sets launcher column position to new position based on direction */
                    player->col += player->dir;
                    LAUNCHER_COL += player->dir;
                }
                else if ((player->dir == -1) && (player->col != 0)) {
                    /* Sets launcher column position to new position based on direction */
                    player->col += player->dir;
                    LAUNCHER_COL += player->dir;

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
                move(LINES-1, COLS-1); /* Parks cursor */
                refresh(); /* Refreshes the screen */
                pthread_mutex_unlock(&MX);
                
                player->dir = 0; /* Sets direction to 0 */
            }
        }
        pthread_exit(NULL);
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

        /* Animates rocket upward while rocket position is not 0 and rocket is live*/
        while((myrocket->row >= 0) && myrocket->live == 1) {
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

        /* Display explosion if saucer hit by rocket */
        else {
            pthread_mutex_lock(&MX);
            move(mysaucer->row, mysaucer->col-1); /* Go to saucer's last position */
            addstr(EXPLOSION); /* Display explosion */
            move(LINES-1, COLS-1); /* Park cursor */
            refresh();
            pthread_mutex_unlock(&MX);
        }

        /* Remove saucer*/
        usleep(TUNIT * 5); // TODO why is this *5? To show explosion
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
        mysaucer->thread = 0;
        pthread_exit(NULL);
}

/*
 * TODO add description for this function
 * Animate the ammo drops
 */
void *animate_ammo_drops(void *arg)
{
        struct ammo_drop *myammo = arg;

        /* Displays initial ammo drop */
        usleep(TUNIT);
        pthread_mutex_lock(&MX);
        move(myammo->row, myammo->col);
        addstr(myammo->str);
        myammo->row += 1; /* Decrements ammo drop position */
        move(LINES-1, COLS-1);
        refresh();
        pthread_mutex_unlock(&MX);

        /* Animates rocket upward while rocket position is not 0 and rocket is live*/
        while((myammo->row <= LINES-1) && myammo->live == 1) {
            usleep(TUNIT * 10); /* Speed at which the ammo drop will fall */
            pthread_mutex_lock(&MX);
            move(myammo->row-1, myammo->col); /* Moves to old instance of ammo drop */
            addch(' '); /* Removes it */
            move(myammo->row, myammo->col); /* Moves to new ammo drop location */
            addstr(myammo->str); /* Prints it */
            myammo->row += 1; /* Increments ammo drop position */
            move(LINES-1, COLS-1); /* Park cursor */
            refresh();
            pthread_mutex_unlock(&MX);
        }

        /* Remove last visible instance of rocket */
        usleep(TUNIT);
        pthread_mutex_lock(&MX);
        move(myammo->row-1, myammo->col);
        addch(' ');
        move(LINES-1, COLS-1);
        refresh();
        pthread_mutex_unlock(&MX);

        /* Cleanup before exiting thread */
        myammo->live = 0;
        myammo->row = 0;
        myammo->col = 0;
        myammo->thread = 0;
        pthread_exit(NULL); // TODO difference between this and pthread_cancel in terms of threads array?
}
/* TODO REMOVE BACKUP */
/* void *animate_ammo_drops(void *arg) */
/* { */
/*         struct ammo_drop *myammo = arg; */

/*         /\* Displays initial ammo drop *\/ */
/*         usleep(TUNIT); */
/*         pthread_mutex_lock(&MX); */
/*         move(myammo->row, myammo->col); */
/*         addstr(myammo->str); */
/*         myammo->row += 1; /\* Decrements ammo drop position *\/ */
/*         move(LINES-1, COLS-1); */
/*         refresh(); */
/*         pthread_mutex_unlock(&MX); */

/*         /\* Animates rocket upward while rocket position is not 0 and rocket is live*\/ */
/*         while((myammo->row <= LINES-1) && myammo->live == 1) { */
/*             usleep(TUNIT * 10); /\* Speed at which the ammo drop will fall *\/ */
/*             pthread_mutex_lock(&MX); */
/*             move(myammo->row-1, myammo->col); /\* Moves to old instance of ammo drop *\/ */
/*             addch(' '); /\* Removes it *\/ */
/*             move(myammo->row, myammo->col); /\* Moves to new ammo drop location *\/ */
/*             addstr(myammo->str); /\* Prints it *\/ */
/*             myammo->row += 1; /\* Increments ammo drop position *\/ */
/*             move(LINES-1, COLS-1); /\* Park cursor *\/ */
/*             refresh(); */
/*             pthread_mutex_unlock(&MX); */
/*         } */

/*         /\* Remove last visible instance of rocket *\/ */
/*         usleep(TUNIT); */
/*         pthread_mutex_lock(&MX); */
/*         move(myammo->row-1, myammo->col); */
/*         addch(' '); */
/*         move(LINES-1, COLS-1); */
/*         refresh(); */
/*         pthread_mutex_unlock(&MX); */

/*         /\* Cleanup before exiting thread *\/ */
/*         myammo->live = 0; */
/*         myammo->row = 0; */
/*         myammo->col = 0; */
/*         pthread_exit(NULL); // TODO difference between this and pthread_cancel in terms of threads array? */
/* } */

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
        while (!QUIT_FLAG) {
            /* Sleeps for random period between 1 and 5 seconds */
            sleep(1+ rand()%5);

            for (i=0; i < MAX_SAUCERS; i++) { // TODO what to do when all MAX_SAUCERS are on screen?
                if (saucer_array[i].live == 0 && saucer_array[i].thread == 0) {
                    /* Generates random row value between 0 and LANES-1 */
                    /* Thus generates LANES number of lanes for the saucers to traverse */
                    saucer_array[i].row = rand()%LANES;
                    saucer_array[i].col = 0;
                    /* Generates delay value of 3 + random # between 0 and 12 - ie delay is random number between 3 and 15 */
                    saucer_array[i].delay = 3 + (rand()%13);
                    saucer_array[i].live = 1;
                    saucer_array[i].thread = 1;
                    break;
                }
            }
            pthread_create(&saucer_threads[i], NULL, animate_saucer, &saucer_array[i]); // TODO error case
        }
        /* Set all saucers to 0 for cleanup */
        for (i=0; i < MAX_SAUCERS; i++)
                saucer_array[i].live = 0;

        pthread_exit(NULL);
}

/*
 * TODO add description for this function
 * Creates ammo drops when needed
 */
void *ammo_drop_factory(void *arg)
{
        struct ammo_drop *ammo_drop_array = arg;
        pthread_t ammo_drop_threads[MAX_AMMO_DROPS];
        void *animate_ammo_drops();
        int i;
        
        /* Factory loop */
        while (!QUIT_FLAG) {
            if (AMMO_FLAG == 1) {
                for (i = 0; i < MAX_AMMO_DROPS; i++) {
                    if (ammo_drop_array[i].live == 1 && ammo_drop_array[i].thread == 0) {
                        // ammo_drop_array[i].new == 0; // TODO no longer needed?
                        ammo_drop_array[i].thread = 1;
                        pthread_create(&ammo_drop_threads[i], NULL, animate_ammo_drops, &ammo_drop_array[i]);
                        break;
                    }
                }
                AMMO_FLAG = 0;
            }
            
        }

        /* Sets all ammo drops to 0 for cleanup */
        for (i=0; i < MAX_AMMO_DROPS; i++)
                ammo_drop_array[i].live = 0;

        pthread_exit(NULL);
}

/*
 * TODO add description for this function
 * Detects a launch and fires the rocket
 */
void *rocket_factory(void *arg) {
        struct rocket *rocket_array = arg;
        pthread_t rocket_threads[MAX_ROCKETS];
        void *animate_rocket();
        int i;

        while (!QUIT_FLAG) {
            /* Check if user wants to launch rocket and does so */
            if (LAUNCH_FLAG == 1) {
                for (i=0; i < MAX_ROCKETS; i++) { // TODO what to do when all MAX_ROCKETS are on screen?
                    if (rocket_array[i].live == 0) {
                        rocket_array[i].row = (LINES-2) - 1;
                        rocket_array[i].col = LAUNCHER_COL;
                        rocket_array[i].live = 1;
                        break;
                    }
                }
                LAUNCH_FLAG = 0;
                pthread_create(&rocket_threads[i], NULL, animate_rocket, &rocket_array[i]); // TODO error case
            }
        }
        /* Sets all rockets to 0 for cleanup */
        for (i=0; i < MAX_ROCKETS; i++)
                rocket_array[i].live = 0;

        pthread_exit(NULL);
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
        while (!QUIT_FLAG) {
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
void strike_check(struct rocket rocket_array[], struct saucer saucer_array[], struct ammo_drop ammo_drop_array[])
{
        int i, j, k;

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

                                /* 1 in 3 chance to spawn an ammo drop */
                                if ((1 + rand()%2) == 1) { // TODO set back to 4
                                    for (k = 0; k < MAX_AMMO_DROPS; k++) {
                                        if (ammo_drop_array[k].live == 0) {
                                            ammo_drop_array[k].live = 1;
                                            ammo_drop_array[k].new = 1;
                                            ammo_drop_array[k].row = rocket_array[i].row + 1; // TODO use +1 here?
                                            ammo_drop_array[k].col = rocket_array[i].col;
                                            AMMO_FLAG = 1;
                                            break;
                                        }
                                    }
                                }

                                /* Set saucer to dead */
                                saucer_array[j].live = 0;
                                /* Set rocket to dead */
                                rocket_array[i].live = 0;
                                /* Points for killing saucer */
                                P1SCORE += SAUCER_SCORE;
                                /* Ammo for killing saucer */
                                // TODO remove
                                //P1AMMO += SAUCER_AMMO;

                            }
                        }
                    }
                }
            }
        }
}

/*
 * TODO add description for this function
 * Detects ammo collection
 */
void ammo_collection_check(struct ammo_drop ammo_drop_array[], struct launcher launcher)
{
        int i, j;

        for (i=0; i < MAX_AMMO_DROPS; i++) {
            if (ammo_drop_array[i].live == 1){
                if (ammo_drop_array[i].row == launcher.row) {
                    if (ammo_drop_array[i].col == launcher.col) {
                        ammo_drop_array[i].live = 0;
                        P1AMMO += 2;
                        break;
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
        char *blank = "                                                                        "; // TODO make this dynamic
        while(!QUIT_FLAG) {
            usleep(TUNIT);
            pthread_mutex_lock(&MX);
            mvprintw(LINES-1,0, blank);
            mvprintw(LINES-1,0,"Player1 - Score: %d - Ammo: %d - Escaped Saucers: %d - Time Left: %d", P1SCORE, P1AMMO, ESCAPE, TIMER);
            move(LINES-1, COLS-1); /* Park cursor */
            refresh();
            pthread_mutex_unlock(&MX);
        }
        pthread_exit(NULL);
}

/*
 * TODO add description for this function
 * Countdown timer
 */
void *countdown_timer()
{
        while ((TIMER > 0) && !QUIT_FLAG) {
            sleep(1);
            TIMER -= 1;
        }
        pthread_exit(NULL);
}

/*
 * TODO add description for this function
 * Splash screen
 */
void splash_screen()
{
        char title[] = "SAUCER ATTACK";
        char anykey[] = "Press any key to start playing or i for instructions";
        char c;
        int active_screen = 1;

        mvprintw(LINES/2, (COLS-strlen(title))/2, title);
        mvprintw(LINES/2+1, (COLS-strlen(anykey))/2, anykey);
        move(LINES-1, COLS-1); /* Park cursor */

        while (active_screen) {
            c = getch();
            if (c == 'i')
                    instruction_screen();
            else {
                    clear();
                    active_screen = 0;
            }
        }
}

/*
 * TODO add description for this function
 * Instruction screen
 */
void instruction_screen()
{
        char title[] = "Instructions:";
        char i1[] = "Defend Earth!";
        char i2[] = "- The aliens have opened a wormhole that will last 60 seconds!";
        char i3[] = "- Do not let 10 alien saucers bypass our defences within that time!";
        char i4[] = "- Do not run out of ammo or Earth will be defenseless!";
        char i5[] = "- Pick up ammo drops (%) that may drop from the saucers!";
        char i6[] = "Good luck!";
        char i7[] = "Move: ARROW KEYS";
        char i8[] = "Shoot: SPACEBAR";
        char i9[] = "Quit: SHIFT-Q";
        char anykey[] = "Press any key to start playing";
        char newline[] = "\n";
        char controls[] = "CONTROLS";

        clear();
        mvprintw(LINES/2-5, (COLS-strlen(title))/2, title);
        mvprintw(LINES/2-4, 5, i1);
        mvprintw(LINES/2-3, 5, i2);
        mvprintw(LINES/2-2, 5, i3);
        mvprintw(LINES/2-1, 5, i4);
        mvprintw(LINES/2, 5, i5);
        mvprintw(LINES/2+1, 5, i6);

        mvprintw(LINES/2+2, 0, newline);
        mvprintw(LINES/2+3, 5, controls);
        mvprintw(LINES/2+4, 9, i7);
        mvprintw(LINES/2+5, 9, i8);
        mvprintw(LINES/2+6, 9, i9);

        mvprintw(LINES/2+7, 0, newline);
        mvprintw(LINES/2+8, 5, anykey);
        move(LINES-1, COLS-1); /* Park cursor */


}

/*
 * TODO add description for this function
 * Victory screen
 */
void victory_screen()
{
        char victory1[] = "VICTORY!!!";
        char victory2[] = "You have successfully fended off the saucer attack!";
        char victory3[] = "Press SHIFT-Q to end the game";
        int c;

        clear();
        mvprintw(LINES/2, (COLS-strlen(victory1))/2, victory1);
        mvprintw(LINES/2+1, (COLS-strlen(victory2))/2, victory2);
        mvprintw(LINES/2+3, (COLS-strlen(victory3))/2, victory3);
        move(LINES-1, COLS-1); /* Park cursor */

        while (1) {
            c = getch();
            if (c == 'Q')
                    break;
        }
}

/*
 * TODO add description for this function
 * Escape fail screen
 */
void escape_fail_screen()
{
        char fail1[] = "FAILURE!!!";
        char fail2[] = "You let too many saucers invade.";
        char fail3[] = "Press SHIFT-Q to end the game";
        int c;

        clear();
        mvprintw(LINES/2, (COLS-strlen(fail1))/2, fail1);
        mvprintw(LINES/2+1, (COLS-strlen(fail2))/2, fail2);
        mvprintw(LINES/2+3, (COLS-strlen(fail3))/2, fail3);
        move(LINES-1, COLS-1); /* Park cursor */

        while (1) {
            c = getch();
            if (c == 'Q')
                    break;
        }
}

/*
 * TODO add description for this function
 * No ammo fail screen
 */
void no_ammo_fail_screen()
{
        char fail1[] = "FAILURE!!!";
        char fail2[] = "You ran out of ammo and can no longer defend against the saucer invasion.";
        char fail3[] = "Press SHIFT-Q to end the game";
        int c;

        clear();
        mvprintw(LINES/2, (COLS-strlen(fail1))/2, fail1);
        mvprintw(LINES/2+1, (COLS-strlen(fail2))/2, fail2);
        mvprintw(LINES/2+3, (COLS-strlen(fail3))/2, fail3);
        move(LINES-1, COLS-1); /* Park cursor */

        while (1) {
            c = getch();
            if (c == 'Q')
                    break;
        }
}
