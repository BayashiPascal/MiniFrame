// ============ MINIFRAME_MODEL.H ================

// ================= Include ==================

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "/home/bayashi/GitHub/PBErr/pberr.h"
#include "/home/bayashi/GitHub/NeuraNet/neuranet.h"

// ================= Define ==================

// Current implementation doesn't allow more than 2 players
// due to undefined end condition
#define NBPLAYER 2
#define NBHOLEPLAYER 6
#define NBHOLE (NBHOLEPLAYER * NBPLAYER)
#define NBINITSTONEPERHOLE 4
#define NBSTONE (NBHOLE * NBINITSTONEPERHOLE)
#define NBMAXTURN 200

#define MF_MODEL_NN_NBINPUT NBHOLE
#define MF_MODEL_NN_NBOUTPUT 10
#define MF_MODEL_NN_NBHIDDEN 1
#define MF_MODEL_NN_NBBASES 100
#define MF_MODEL_NN_NBLINKS 100

// True if all actors act simultaneously, else false. As no effect if 
// MF_NBMAXACTOR equals 1
#define MF_SIMULTANEOUS_PLAY false
// Max number of actors in the world
// must be at least one
#define MF_NBMAXACTOR NBPLAYER
// Max number of transitions possible from any given status
// must be at least one
#define MF_NBMAXTRANSITION NBHOLEPLAYER

// ================= Data structure ===================

// Structure describing the transition from one instance of 
// MFModelStatus to another
typedef struct MFModelTransition {
  // Index of the hole from where stones are moved by the current player
  int _iHole;
} MFModelTransition;

// Structure describing the status of the world at one instant
typedef struct MFModelStatus {
  int _nbTurn;
  int _nbStone[NBHOLE];
  int _score[NBPLAYER];
  // Flag for special end condition
  char _end;
  // Index of the player who has the sente
  int _curPlayer;
  // NeuraNet for each player
  NeuraNet* _nn[NBPLAYER];
} MFModelStatus;

// ================ Functions declaration ====================

// Get the number of active actors
int MFModelStatusGetNbActor(const MFModelStatus* const that);

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

// Preprocess of one turn of the game
// Return the game as it is if nothing to do
MFModelStatus MFModelStatusStepInit(const MFModelStatus* const that);

// Postprocess of one turn of the game
// Return the game as it is if nothing to do
MFModelStatus MFModelStatusStepEnd(const MFModelStatus* const that);

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
  const MFModelStatus* const curStatus);

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
