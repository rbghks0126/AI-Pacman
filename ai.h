#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "node.h"
#include "priority_queue.h"
#define TOTAL_MAX_DEPTH 0
#define TOTAL_GENERATED 1
#define TOTAL_EXPANDED 2
#define TOTAL_TIME 3
#define EXPANDED_PER_SECOND 4

void initialize_ai();

/* get_next_move function is slightly modified, has an additional argument
double *global. This is used to update the global variables as described in
pacman.c */
move_t get_next_move( state_t init_state, int budget, propagation_t propagation,
    char* stats , double *global);

/* returns 1 if there is a collision with any ghost in given state,
 and 0 otherwise */
int CollisionDeath(state_t *state);


#endif
