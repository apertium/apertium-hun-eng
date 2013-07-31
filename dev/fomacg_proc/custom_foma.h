#ifndef CUSTOM_FOMA_H
#define CUSTOM_FOMA_H

/**
 * Contains modified versions of the foma library functions that are more
 * tailored to our needs as opposed to the stock versions.
 *
 * @author Dávid Márk Nemeskey
 */

#include "fomacg_common.h"

/**
 * Concatenates @p cohort to @p sentence. Both automata are assumed to be
 * linear, and hence no minimization is performed. This shaves off about 20% of
 * the running time for the composition method.
 */
struct fsm* fsm_concat_custom(struct fsm *sentence, struct fsm *cohort);

#endif
