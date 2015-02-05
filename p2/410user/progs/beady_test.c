/** @file beady_test.c
 *
 *  @brief Simple game for testing the thread library. Feel free to copy it
 *         into your user_tests, then change or add to it as you see fit!
 *
 *  User uses ',' and '.' keys to move a small cursor, attempting to keep
 *  it within range of a ``bead'' on the string. User gets points for keeping 
 *  the cursor on the bead.
 *
 *  This program utilizes four threads: a scoring thread, cursor control 
 *  thread,
 *  screen update thread, and bead control thread. The threads intercommunicate
 *  through a world lock mutex and and update condition variable: when a thread
 *  changes world state, it signals the update cv to request a screen re-draw.
 *  A condition variable holds the main thread from terminating. When
 *  termination occurs, a flag is set that each thread actively polls on, and
 *  each thread cleans its house, then returns to the main thread. The
 *  exit codes returned by each thread are intended to help identify
 *  which threads have returned.
 *
 *  NOTE: assumes your implementation allows for the `ancestor thread'
 *  (thread that called thr_init()) to be condition-waited...
 *
 *  Tests: threads, mutex, condition variables, syscalls, printf, malloc
 *
 *  @author Mark T. Tomczak (mtomczak)
 *  @author Andy Herrman (aherrman)
 *
 *  @bug Keyboard access thread gets cycle-hungry when keys are available...
 *       Avoid allowing the direction keys to repeat
 **/

#include <thread.h>
#include <stdio.h>
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>
#include <mutex.h>
#include <cond.h>
#include <thrgrp.h>

#define NUM_THREADS 4

enum THR_EXIT_CODES{
  UPDATE_EXIT_CODE=1,
  CONTROL_EXIT_CODE,
  BEAD_EXIT_CODE,
  SCORE_EXIT_CODE
};


#undef try
#define try(evt) if((error=(evt)) < 0) \
                 {lprintf ("beady_test error %d\n",error);}

#define TARGET_TIME 25 /** sleep time for the bead control thread **/
#define SCORE_TIME 5   /** sleep time for the scoring thread **/

#define GAMEBAR_BAR "-" /** picture of the bar **/
#define GAME_TARGET "*" /** picture of the bead **/
#define GAME_CURSOR "^" /** picture of the cursor **/
#define ERASE_CURSOR " " /** symbol to erase the cursor with **/

#define GAMEBAR_ROW 12 /** location of the bead string **/
#define P1_ROW 13      /** location of the cursor **/
#define CURSOR_MAX 75  /** cursor max travel **/
#define CURSOR_MIN 5   /** cursor min travel **/
#define FIRSTSTEP 0xfeedface /** random number seed **/

mutex_t worldLock; /** lock on the world's state **/
cond_t  updates;   /** condition variable for world update events **/

// we don't have to protect goFlag with locking because it is only
// modified by the controlThread... we don't care how many threads read
// from it simultaneously.
int     goFlag=0;  /**< Flag, set to 1 when the game is over */
cond_t  gameOver;  /** locks down the game from terminating **/
mutex_t updateLock; /** lock on the update variable **/

int updatePending=0;  /** is an update pending? **/
int p1Score=0;  /** score for player 1 **/

int p1Cursor=40; /** cursor position for p1 **/
int oldP1Cursor=40; /** old cursor position for p1 **/

int targetPos=50; /** initial position for target slide **/
int oldTargetPos = 50; /** old target position */

/** @brief Notifies an update event should occur
 *
 *  This function sets the update pending flag, then signals an update
 *
 **/
void scheduleUpdate(void)
{
	/** lock the update flag **/
	mutex_lock(&updateLock);

	/** set the pending update flag **/
	updatePending=1;

	/** signal update if it was waiting **/
	cond_signal(&updates);
	
	/** unlock update flag **/
	mutex_unlock(&updateLock);

}

/** @brief Blocks until an update event is ready
 *
 *  the update function calls this function, which does not return
 *  until the update is ready to occur
 *
 ***/

void waitUpdate(void)
{
	/** lock the update mutex **/
	mutex_lock(&updateLock);
	if(updatePending==0)
		{
			/** wait for an update **/
			cond_wait(&updates, &updateLock);

		 
		}
	updatePending=0;
	/* give back the update mutex **/
	mutex_unlock(&updateLock);
}

/** @brief Update the world
 *
 *  Re-draws the world image. This function requires (reader's) lock on the
 *  world, then updates the world image
 **/

void *update(void *ignored)
{

	while(1)
		{
			/** await for a request to update the world **/
			waitUpdate();

       if(goFlag)
        return (void *)UPDATE_EXIT_CODE;

			//lprintf("updateThread: ping!");
			/** request received; re-draw the world **/
			mutex_lock(&worldLock);
			
			/* draw the target */
			set_cursor_pos(GAMEBAR_ROW, oldTargetPos);
			print(1, GAMEBAR_BAR);
			set_cursor_pos(GAMEBAR_ROW, targetPos);
			print(1, GAME_TARGET);
			oldTargetPos=targetPos;

			/* draw the player */
			set_cursor_pos(P1_ROW, oldP1Cursor);
			print(1, ERASE_CURSOR);
			set_cursor_pos(P1_ROW, p1Cursor);
			print(1, GAME_CURSOR);
			oldP1Cursor=p1Cursor;

			/* draw the score */
			set_cursor_pos(22,0);
			printf("Score: %d\n",p1Score);
			
		
			
			/** unlock world **/
			mutex_unlock(&worldLock);

     
      
			yield(-1);
		}
}
	 
/** @brief Receives control input and updates the world 
 *
 *  Function to receive control updates. It updates the position of the
 *  cursor and calls the screen update function 
 *
 **/

void *controlThread(void *ignored)
{

  int done = 0;	
	while(!done)
		{
			//lprintf("controlThread: ping!\n");
			switch(getchar())
				{
				case '.':
					mutex_lock(&worldLock);
					p1Cursor++;
					if(p1Cursor > CURSOR_MAX)
						{
							p1Cursor=CURSOR_MAX;
						}
					scheduleUpdate();
					mutex_unlock(&worldLock);
					break;
				case ',':
					mutex_lock(&worldLock);
					p1Cursor--;
					if(p1Cursor < CURSOR_MIN)
						{
							p1Cursor=CURSOR_MIN;
						}
					scheduleUpdate();
					mutex_unlock(&worldLock);
					break;
        case 'q':
          printf("Game over.\n");
          goFlag=1;
          cond_signal(&gameOver);
          /* exit the loop */
          return (void *)CONTROL_EXIT_CODE;
          break;
				}
			yield(-1);
		}
  return (void *)CONTROL_EXIT_CODE;
}

/** @brief Moves the target 
 * 
 *  function ``randomly'' walks target left and right every n ticks.
 *
 **/

void *targetThread(void *ignored)
{
	int nextStep=FIRSTSTEP;
	
	while(1)
		{
			//lprintf("targetThread: Ping!");
			sleep(TARGET_TIME);
			
			/** choose ``random'' direction for target **/
			nextStep ^= nextStep>> 5;
			nextStep ^= nextStep<< 13;
			
			/** grab world lock **/
			mutex_lock(&worldLock);
			
			/** move target **/
			if((nextStep & 0x01))
				{
					targetPos++;
					if(targetPos > CURSOR_MAX)
						{
							targetPos = CURSOR_MAX;
						}
				}
			else
				{
					targetPos--;
					if(targetPos < CURSOR_MIN)
						{
							targetPos = CURSOR_MIN;
						}
				}
			
			/** schedule an update **/
			scheduleUpdate();
			mutex_unlock(&worldLock);
      if(goFlag)
        return (void *) BEAD_EXIT_CODE;
			
		}
	
}

/** @brief Updates the score 
 *
 *  This thread runs at some fraction of the bar update rate, and
 *  increases the score by 1 every time it notes the cursor and the bar
 *  are on top of each other. This function doesn't call update, because
 *  it would be slower than necessary
 *
 *  Our implementation is very busy. This isn't really necessary... 
 *  technically,
 *  the cursor moving function should wake up the scoring function.
 **/

void *scoreThread(void *ignore)
{

	while (1)
		{
			sleep(SCORE_TIME);

			/* lock the world */
			mutex_lock(&worldLock);
			
			/* check for scoring state */
			if(targetPos == p1Cursor)
				{
					p1Score++;
				}

			mutex_unlock(&worldLock);
      if(goFlag)
        return (void *)SCORE_EXIT_CODE;
		}
}

void initScreen(void)
{
	int i;
	for(i=0; i<24; i++)
	printf("                                                                                ");
	set_cursor_pos(GAMEBAR_ROW, 3);
	print(1,"<");
	for(i=0; i<74; i++)
		{
			print(1, GAMEBAR_BAR);
		}
	print(1,">");
}

int main(int argc, char ** argv)
{
	int error;
  int i;
  int retcode;
  thrgrp_group_t tg;


	try(thr_init(7*1024));
	try(mutex_init(&worldLock));
	try(mutex_init(&updateLock));
	try(cond_init(&updates));
	try(cond_init(&gameOver));
	initScreen();
	
  thrgrp_init_group(&tg);
	try(thrgrp_create(&tg,update, 0));
	try(thrgrp_create(&tg,controlThread, 0));
	try(thrgrp_create(&tg,targetThread, 0));
	try(thrgrp_create(&tg,scoreThread, 0));

	/** finish drawing the screen **/
	//scheduleUpdate();

	/** lock down until the game finishes **/
	mutex_lock(&worldLock);
	cond_wait(&gameOver, &worldLock);
    mutex_unlock(&worldLock);

  /** game is over **/
  cond_signal(&updates);

  /** Once the game is over, cleanup after yourself! **/
  for(i=0; i<NUM_THREADS; i++)
    {
      try(thrgrp_join(&tg, (void **)&retcode));
      printf("Worker thread returned with code %d.\n",retcode);
    }
 
	thr_exit(0);
	return 0;
}
