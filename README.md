Saucer Attack
===========
CMPUT 379 - Assignment 3
Costa Zervos

1. Installation:

	i. Run make saucer
	ii. Run ./saucer

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

5. References:
	Based off of tanimate.c used in the lab.
