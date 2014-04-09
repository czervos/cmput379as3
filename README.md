Saucer Attack
===========
CMPUT 379 - Assignment 3
Costa Zervos

1. Installation:
	1. Run make saucer ALTERNATIVELY run make all
	2. Run ./saucer

2. Controls:
	Move: LEFT and RIGHT arrow keys.
	Shoot: SPACEBAR
	Quit: SHIFT-Q

3. Instructions:
	During the 60 seconds of gameplay, saucers will fly across the screen
	from left to right. You must position the launcher to shoot down these
	saucers using the rockets before 10 of them make it across the screen
	and before you run out of ammo.

	When you hit a saucer, it has a 50% chance of dropping ammo. Position
	the launcher under the ammo drop as it reaches the bottom to collect the
	ammo.

	If you make it to the end of 60 seconds without letting 10 saucers
	escape or running out of ammo, you win the game!

4. Props:
	Launcher: |
	Saucer: <--->
	Rocket: ^
	Ammo drop: #

5. Enhancements:
	1. Ammo drops: Instead of getting ammo instantly when destroying a
	   saucer, there is a random (50%) chance that the saucer will drop
	   ammo after exploding. The user will be required to line up the
	   launcher with the ammo as it reaches the bottom to collect the ammo.
	   This requires the use of threads for each ammo drop, along with a
	   thread to determine when to create an ammo drop.
	2. Gameplay timer: the game is over after 60 seconds. If the player
	   successfully holds off the saucer attack within this time limit,
	   the player wins. This requires the use of a thread for the timer.
	3. General UI: the game has a title screen, instruction screen, and
	   endgame condition screens depending on which endgame condition is 
	   achieved.

5. References:
	Based off of tanimate.c used in the lab.
