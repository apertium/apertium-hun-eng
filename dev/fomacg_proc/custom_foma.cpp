#include "custom_foma.h"
#include <cstdlib>

/*
 * The functions below have been copied from the foma .c files. This is not the
 * best solution; far from it, but we'll make do with what we got.
 */

/* constructions.c */
static void fsm_add_to_states(struct fsm *net, int add) {
  struct fsm_state *fsm;
  int i;

  fsm=net->states;
  for(i=0; (fsm+i)->state_no != -1; i++) {
    (fsm+i)->state_no = (fsm+i)->state_no + add;
    if ((fsm+i)->target != -1)
      (fsm+i)->target = (fsm+i)->target + add;
  }
}

int add_fsm_arc(struct fsm_state *fsm, int offset, int state_no, int in, int out, int target, int final_state, int start_state) {

  (fsm+offset)->state_no = state_no;
  (fsm+offset)->in = in;
  (fsm+offset)->out= out;
  (fsm+offset)->target = target;
  (fsm+offset)->final_state = final_state;
  (fsm+offset)->start_state = start_state;
  offset++;
  return(offset);
}

/* mem.c */
inline void *xxmalloc(size_t size) {
    return(malloc(size));
}

inline void xxfree(void *ptr) {
    free(ptr);
}

/* sigma.c */
int sigma_find_number(int number, struct sigma *sigma) {
    if (sigma == NULL)
        return -1;
    if (sigma->number == -1) {
        return -1;
    }
    /* for (;(sigma != NULL) && (sigma->number <= number); sigma = sigma->next) { */
    for (;(sigma != NULL) && (sigma->number != -1); sigma = sigma->next) {
        if (number == sigma->number) {
            return (sigma->number);
        }
    }
    return -1;
}

struct fsm* fsm_concat_custom(struct fsm *net1, struct fsm *net2) {
  struct fsm_state *fsm1, *fsm2, *new_fsm;
  int i,j,current_final;

  fsm_merge_sigma(net1, net2);

  fsm1 = net1->states;
  fsm2 = net2->states;
  fsm_count(net1);
  fsm_count(net2);
  /* The concatenation of a language with no final state should yield the empty language */
  if ((net1->finalcount == 0) || (net2->finalcount == 0)) {
      fsm_destroy(net1);
      fsm_destroy(net2);
      net1 = fsm_empty_set();
      return(net1);
  }

  /* Add |fsm1| states to the state numbers of fsm2 */
  fsm_add_to_states(net2, net1->statecount);

  new_fsm = (struct fsm_state*)xxmalloc(((sizeof(struct fsm_state))*(net1->linecount + net2->linecount + net1->finalcount + 2 )));
  current_final = -1;
  /* Copy fsm1, fsm2 after each other, adding appropriate epsilon arcs */
  for(i=0,j=0; (fsm1+i)->state_no != -1; i++) {
    if (((fsm1+i)->final_state == 1) && ((fsm1+i)->state_no != current_final)) {
      add_fsm_arc(new_fsm, j, (fsm1+i)->state_no, EPSILON, EPSILON, net1->statecount, 0, (fsm1+i)->start_state);
      current_final = (fsm1+i)->state_no;
      j++;
    }
    if (!(((fsm1+i)->target == -1) && ((fsm1+i)->final_state == 1))) {
      add_fsm_arc(new_fsm, j, (fsm1+i)->state_no, (fsm1+i)->in, (fsm1+i)->out, (fsm1+i)->target, 0, (fsm1+i)->start_state);
      j++;
    }
  }

  for(i=0; (fsm2+i)->state_no != -1; i++, j++) {
    add_fsm_arc(new_fsm, j, (fsm2+i)->state_no, (fsm2+i)->in, (fsm2+i)->out, (fsm2+i)->target, (fsm2+i)->final_state, 0);
  }
  add_fsm_arc(new_fsm, j, -1, -1, -1, -1, -1, -1);
  xxfree(net1->states);
  fsm_destroy(net2);
  net1->states = new_fsm;
  if (sigma_find_number(EPSILON, net1->sigma) == -1) {
    sigma_add_special(EPSILON, net1->sigma);
  }
  fsm_count(net1);
  net1->is_epsilon_free = NO;
  net1->is_deterministic = NO;
  net1->is_minimized = NO;
  net1->is_pruned = NO;
  return net1;
}

