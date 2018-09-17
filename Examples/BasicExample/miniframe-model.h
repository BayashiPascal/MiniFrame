// ============ MINIFRAME_MODEL.H ================

// As an example the code below implements a world where one actor
// moves along a discrete axis by step of one unit to reach a fixed 
// target position
// Status of the world is defined by the current actor position and 
// the target position
// Available actions are -1, 0, +1 (next position = current position 
// + action) if the actor hasn't reached the target, else no actions 
// The position of the actor is bounded to -5, 5
// The value of the world is given by -abs(position-target)

// ================= Include ==================

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "/home/bayashi/GitHub/PBErr/pberr.h"

// ================= Define ==================

// Max number of actors in the world
// must be at least one
#define MF_NBMAXACTOR 1
// Max number of transitions possible from any given status
// must be at least one
#define MF_NBMAXTRANSITION 3

// ================= Data structure ===================

// Structure describing the transition from one instance of 
// MFModelStatus to another
typedef struct MFModelTransition {
  int _move;
} MFModelTransition;

// Structure describing the status of the world at one instant
typedef struct MFModelStatus {
  int _step;
  int _pos;
  int _tgt;
} MFModelStatus;

// ================ Functions declaration ====================

// Copy the properties of the MFModelStatus 'that' into the 
// MFModelStatus 'tho'
// Dynamically allocated properties must be cloned
void MFModelStatusCopy(const MFModelStatus* const that,
  MFModelStatus* const tho);

// Free memory used by the properties of the MFModelStatus 'that'
// The memory used by the MFModelStatus itself is managed by MiniFrame
void MFModelStatusFreeStatic(MFModelStatus* that);

// Free memory used by the properties of the MFModelTransition 'that'
// The memory used by the MFModelTransition itself is managed by 
// MiniFrame
void MFModelTransitionFreeStatic(MFModelTransition* that);

// Return true if 'that' and 'tho' are to be considered as the same
// by MiniFrame when trying to reuse previously computed status, 
// else false
bool MFModelStatusIsSame(const MFModelStatus* const that,
  const MFModelStatus* const tho);

// Return the index of the actor who has preemption in the MFModelStatus
// 'that'
// If no actor has preemption (all the actor act simultaneously)
// return -1
int MFModelStatusGetSente(const MFModelStatus* const that);

// Return true if the actor 'iActor' is active given the MFModelStatus 
// 'that'
bool MFModelStatusIsActorActive(const MFModelStatus* const that, 
  const int iActor);

// Get the possible transitions from the MFModelStatus 'that' and 
// memorize them in the array of MFModelTransition 'transitions', and 
// memorize the number of transitions in 'nbTrans'
// 'transitions' as MF_NBMAXTRANSITION size, got MFModelTransition are 
// expected in transitions[0~(nbTrans-1)]
void MFModelStatusGetTrans(const MFModelStatus* const that, 
  MFModelTransition* const transitions, int* const nbTrans);

// Get the values of the MFModelStatus 'that' from the point of view 
// of each actor and  memorize them in the array of float 'values'
// 'values' as MF_NBMAXACTOR size, all values are set to 0.0 before 
// calling this function
void MFModelStatusGetValues(const MFModelStatus* const that, 
  float* const values);

// Return the MFModelStatus resulting from applying the 
// MFModelTransition 'trans' to the MFModelStatus 'that' 
MFModelStatus MFModelStatusStep(const MFModelStatus* const that, 
  const MFModelTransition* const trans);

// Print the MFModelStatus 'that' on the stream 'stream' 
void MFModelStatusPrint(const MFModelStatus* const that, 
  FILE* const stream);

// Print the MFModelTransition 'that' on the stream 'stream' 
void MFModelTransitionPrint(const MFModelTransition* const that, 
  FILE* const stream);

// Return true if the MFStatus 'that' is disposable (its memory can be 
// freed) given the current status 'curStatus' and the number of 
// world instances in memory, else false
// As many as possible should be kept in memory, especially if worlds
// are reusable, but its up to the user to decide which and when whould
// be discarded to fit the physical memory available
// Having too many world instances in memory also slow down the 
// exploration of worlds during expansion
bool MFModelStatusIsDisposable(const MFModelStatus* const that, 
  const MFModelStatus* const curStatus, const int nbStatus);

// Return true if the MFModelStatus 'that' is the end of the 
// game/simulation, else false
bool MFModelStatusIsEnd(const MFModelStatus* const that);

// Init the board
void MFModelStatusInit(MFModelStatus* const that);

#if BUILDMODE != 0
inline
#endif
void toto();

// ================ Inliner ====================

#if BUILDMODE != 0
#include "miniframe-inline-model.c"
#endif
