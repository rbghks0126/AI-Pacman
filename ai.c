#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include "ai.h"
#include "utils.h"
#include "priority_queue.h"			


struct heap h;

float get_reward( node_t* n );

/**
 * Function called by pacman.c
*/
void initialize_ai(){
	heap_init(&h);
}

/**
 * function to copy a src into a dst state
*/
void copy_state(state_t* dst, state_t* src){
	//Location of Ghosts and Pacman
	memcpy( dst->Loc, src->Loc, 5*2*sizeof(int) );

    //Direction of Ghosts and Pacman
	memcpy( dst->Dir, src->Dir, 5*2*sizeof(int) );

    //Default location in case Pacman/Ghosts die
	memcpy( dst->StartingPoints, src->StartingPoints, 5*2*sizeof(int) );

    //Check for invincibility
    dst->Invincible = src->Invincible;

    //Number of pellets left in level
    dst->Food = src->Food;

    //Main level array
	memcpy( dst->Level, src->Level, 29*28*sizeof(int) );

    //What level number are we on?
    dst->LevelNumber = src->LevelNumber;

    //Keep track of how many points to give for eating ghosts
    dst->GhostsInARow = src->GhostsInARow;

    //How long left for invincibility
    dst->tleft = src->tleft;

    //Initial points
    dst->Points = src->Points;

    //Remiaining Lives
    dst->Lives = src->Lives;

}

node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n->parent = NULL;
	new_n->priority = 0;
	new_n->depth = 0;
	new_n->num_childs = 0;
	copy_state(&(new_n->state), init_state);
	new_n->acc_reward =  get_reward( new_n );
	return new_n;

}


float heuristic( node_t* n ){
	float h = 0;

	//FILL IN MISSING CODE
	int i = 0;
	int l = 0;
	int g = 0;

	if (n->parent) {
		/* Pacman is invincible in current state*/
		if (n->state.Invincible == 1) {
			/* Pacman was not invincible in previous state, therefore,
			he must have eaten a fruit in current state */
			if ( n->parent->state.Invincible == 0) {
				i = 10;
		}
		/* Pacman not invincible currently */
		} else {
			/* game is over because no life left */
			if (n->state.Lives < 0) {
				g = 100;
			/* Pacman dies due to collision with a ghost but game does not end */
			} else if (CollisionDeath(&(n->state))) {
				l = 10;
			}
		}
	}

	/* final heuristic value */
	h = i - l - g;

	return h;
}

float get_reward ( node_t* n ){
	float reward = 0;

	//FILL IN MISSING CODE
	int score = 0;
	int scoreParent = 0;
	float h = 0.0;

	/* get the heuristic value of current node */
	h = heuristic(n);

	score = (n->state).Points;
	/* this node has parent node */
	if (n->parent != NULL) {
		scoreParent = ((n->parent)->state).Points;
		reward = h + score - scoreParent;
	/* this is the initial node, so reward is 0 */
	} else {
		reward = 0;
	}
	/* discount factor */
	float discount = pow(0.99, n->depth);

	return discount * reward;
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
bool applyAction(node_t* n, node_t** new_node, move_t action ){

	bool changed_dir = false;

	//FILL IN MISSING CODE

	/* allocate space for the new node */
	(*new_node) = (node_t *) malloc(sizeof(node_t));
	assert(*new_node);

	/* first copy the entire state from current node to new node */
	copy_state(&((*new_node)->state), &(n->state));

	/* 1. make new_node point to its parent, n */
	(*new_node)->parent = n;

	/* 2. update the state with the action chosen */
    changed_dir = execute_move_t( &((*new_node)->state), action );

	/* it is an invalid move, delete the new node and return false */
	if (changed_dir == false) {
		free(*new_node);
		return changed_dir;
	}

	/* 3. update the depth/priority of new node */
	(*new_node)->depth = (n->depth) + 1;
	(*new_node)->priority = -1 * (*new_node)->depth;

	/* 4,5. update the reward/acc_reward */
	(*new_node)->acc_reward = (n->acc_reward) + get_reward(*new_node);

	/* 6. update auxiliary data in the node */
	(*new_node)->move = action;
	/* check to prevent double adding num_childs for ancestor at depth 1 */
	if (n->depth != 1)
		(n->num_childs)++;

	/* increment number of child of its ancestor at depth 1 */
	node_t *ancestor;
	/* allocate space for a node which will travel all the way up
	to update required auxiliary variables of the ancestor */
	ancestor = (node_t *) malloc(sizeof(node_t));
	/* temporary node_t pointer so we can free 'ancestor' later on */
	node_t *temp = ancestor;
	/* save variables of interest of current state in ancestor */
	ancestor->depth = (*new_node)->depth;
	ancestor->parent = (*new_node)->parent;

	/* travel back up to the ancestor node, at depth 1 */
	while (ancestor->depth > 1) {
		ancestor = ancestor->parent;
	}
	/* increase the number of decendents of that ancestor by 1 */
	ancestor->num_childs++;
	ancestor = temp;
	/* free the node we used to travel back up to the ancestor */
	free(ancestor);

	/* return True, it was an applicable move */
	return changed_dir;
}

/* returns the action type of given node's ancestor node */
int find_ancestor_move(node_t *node) {
	int move;

	/* we are at one of the decendents */
	if (node->depth > 1) {
		/* create ancestor node which we will use to find the ancestor */
		node_t *ancestor;
		ancestor = (node_t *) malloc(sizeof(node_t));
		node_t *temp = ancestor;
		ancestor->depth = node->depth;
		ancestor->parent = node->parent;

		/* travelling back up to find the ancestor */
		while (ancestor->depth > 1) {
			ancestor = ancestor->parent;
		}
		/* retrieve the move type of the ancestor (left, right, etc.)*/
		move = ancestor->move;
		ancestor = temp;
		free(ancestor);

		return move;
	/* given node is the ancestor */
	} else if (node->depth == 1) {
		return node->move;

	/* at initial node, this won't run */
	} else {
		return -1;
	}
}

/**
 * Propagates back the accumulated reward to the ancestor node at depth 1
*/
int propagateBack(node_t *node, propagation_t propagation, move_t action,
	float *best_action_score, int *num_dec) {

	/* max propagation method */
	if (propagation == max) {
		/* set initial best score to ancestor's acc_reward value */
		if (node->depth == 1) {
			best_action_score[action] = node->acc_reward;
		/* if one of ancestor's decendents has greater acc_reward, update it */
		} else if (node->depth > 1) {
			int ancestor_move = find_ancestor_move(node);
			if (best_action_score[ancestor_move] < node->acc_reward) {
				best_action_score[ancestor_move] = node->acc_reward;
			}
		}
	/* avg progation method */
	} else {
		if (node->depth == 1) {
			best_action_score[action] = node->acc_reward;
		/* sum up acc_reward for all generated nodes, this will be divided
		by num_childs of ancestor later on in get_next_move function */
		} else if (node->depth > 1) {
			int ancestor_move = find_ancestor_move(node);
			best_action_score[ancestor_move] += node->acc_reward;
			/* increment the count for corresponding action by 1 */
			num_dec[ancestor_move]++;
		}
	}
	/* return the depth of the node we worked on, used to find max_depth */
	return node->depth;
}

/**
 * Find best action by building all possible paths up to budget
 * and back propagate using either max or avg
 */

move_t get_next_move( state_t init_state, int budget, propagation_t propagation,
	char* stats, double *global ){
	move_t best_action = rand() % 4;
	float best_action_score[4];
	for(unsigned i = 0; i < 4; i++)
	    best_action_score[i] = INT_MIN;

	/* array which will store the number of all decendents of each of the first
	(upto) four	ancestor nodes. This is used for the 'avg' propagation type */
	int num_dec[4];
	for (int i = 0; i < 4; i++)
		num_dec[i] = 0;

	unsigned generated_nodes = 0;
	unsigned expanded_nodes = 0;
	unsigned max_depth = 0;

	/* depth of current node that is generated, used with propagate function
	to update max_depth */
	int curr_depth;

	/* used for explored array */
	int insert_loc = 0;

	//Add the initial node
	node_t* n = create_init_node( &init_state );

	//Use the max heap API provided in priority_queue.h
	heap_push(&h,n);

	//FILL IN THE GRAPH ALGORITHM

	/* make an array which stores the addresses of explored nodes */
	node_t **explored;
	explored = (node_t **) malloc((budget) * sizeof(node_t *));
	assert(explored);

	/* used later on for iterating through the 4 actions */
	move_t action;

	/* while the frontier/priority queue is not empty */
	while (h.count > 0) {
		/* pop the highest priority node */
		n = heap_delete(&h);

		/* insert n into explored array if there is space */
		if (insert_loc < budget) {
			explored[insert_loc] = n;
		/* allocate more space if there is no space */
		} else {
			explored = (node_t **) realloc(explored, sizeof(n) * (insert_loc + 1));
			assert(explored);
			explored[insert_loc] = n;
		}
		insert_loc++;

		/* continue expanding if we have explored less than the budget */
		if (expanded_nodes < budget) {
			expanded_nodes++;
			global[TOTAL_EXPANDED]++;
			/* for each left, right, up, down move */
			for (action = left; action <= down; action++) {
				node_t *new_node;
				/* only continue if it is a valid (applicable) move */
				if (applyAction(n, &new_node, action)) {
					generated_nodes++;
					global[TOTAL_GENERATED]++;
					curr_depth = propagateBack(new_node, propagation, action,
						best_action_score, num_dec);
					/* updating max_depth and global MAX_DEPTH so far */
					if (curr_depth > max_depth) {
						max_depth = curr_depth;
						if (max_depth > global[TOTAL_MAX_DEPTH]) {
							global[TOTAL_MAX_DEPTH] = max_depth;
						}
					}
					/* Pacman dies as a result of an action */
					if (CollisionDeath(&(new_node->state))) {
						/* delete the new node */
						free(new_node);
					/* Does not die, enqueue the node */
					} else {
						heap_push(&h, new_node);
					}
				}
			}
		}
	}
	int i;
	/* free every node in explored array */
	for (i = 0; i < insert_loc; i++) {
		if (explored[i]) {
			free(explored[i]);
		}
	}
	/* free the explored array itself */
	free(explored);

	/* finalize best_action_score values for avg propagation method */
	if (propagation == avg) {
		for(i = 0; i < 4; i++) {
			if (best_action_score[i] != INT_MIN) {
				/* dividing the sum of all scores by the number of decendents */
				best_action_score[i] = best_action_score[i] / num_dec[i];
			}
		}
	}

	/* choose best action based on best_action_score array */

	/* this will store the max score of four initial moves */
	float max_score = INT_MIN;
	/* array that stores move_t value if it is the best action */
	int best_action_array[4] = {-1, -1, -1, -1};

	/* find the maximum score */
	for (i = 0; i < 4; i++) {
		if (best_action_score[i] > max_score) {
			max_score = best_action_score[i];
		}
	}

	/* tie flag. 1 if 1 max score, 2 if 2 same max scores, 3 if 3 same, etc. */
	int count_tie = 0;
	/* see if there are ties for max score */
	for (i = 0; i < 4; i++) {
		if (best_action_score[i] == max_score) {
			count_tie++;
			/* update that 'i' is one of the best actions */
			best_action_array[i] = i;
		}
	}
	/* there is only 1 path with maximum score. This is the best action */
	if (count_tie == 1) {
		for (i = 0; i < 4; i++) {
			if (best_action_score[i] == max_score) {
				best_action = best_action_array[i];
			}
		}
	/* there are ties (2 or more same max score) */
	} else {
		/* we want to randomly choose the 'decider'th best action in order */
		int decider = rand() % count_tie;
		for (i = 0; i < 4; i++) {
			/* have reached the 'decider'th best action */
			if (decider == 0 && best_action_array[i] != -1) {
				best_action = best_action_array[i];
			/* decrement decider by 1 only when we are at one of best actions */
			} else if (decider != 0 && best_action_array[i] != -1) {
				decider--;
			}
		}
	}


	sprintf(stats, "Max Depth: %d Expanded nodes: %d  Generated nodes: %d\n",max_depth,expanded_nodes,generated_nodes);

	if(best_action == left)
		sprintf(stats, "%sSelected action: Left\n",stats);
	if(best_action == right)
		sprintf(stats, "%sSelected action: Right\n",stats);
	if(best_action == up)
		sprintf(stats, "%sSelected action: Up\n",stats);
	if(best_action == down)
		sprintf(stats, "%sSelected action: Down\n",stats);

	sprintf(stats, "%sScore Left %f Right %f Up %f Down %f",stats,best_action_score[left],best_action_score[right],best_action_score[up],best_action_score[down]);
	return best_action;
}
/* my functions */

/* Check if there is a collision with a ghost in a state, return 1 if death
modifed version of the CollisionDeathSim function */
int CollisionDeath(state_t *state) {
    //Temporary variable for ghost
    int a = 0;
    /* Death flag */
    int death = 0;

    //Check each ghost, one at a time for collision
    for(a = 0; a < 4; a++) {

        //Ghost X and Y location is equal to Pacman X and Y location (collision)
        if((state->Loc[a][0] == state->Loc[4][0]) && (state->Loc[a][1] == state->Loc[4][1])) {

            //Pacman is vulnerable, Pacman dies.
            if(state->Invincible == 0) {
                death = 1;
            }
        }
    }
    return death;
}
