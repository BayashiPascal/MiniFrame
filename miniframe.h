// ============ MINIFRAME.H ================

#ifndef MINIFRAME_H
#define MINIFRAME_H

// ================= Include =================

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "pberr.h"
#include "pbmath.h"
#include "gset.h"

// ================= Define ==================

// Default time for expansion, in millisecond
#define MF_DEFAULTTIMEEXPANSION 100
// time_ms = clock() / MF_MILLISECTOCLOCKS
#define MF_MILLISECTOCLOCKS (CLOCKS_PER_SEC * 0.001) 
// Default value for pruning during expansion
#define MF_PRUNINGDELTAVAL 1000.0
// Default maximum depth of expansion
#define MF_DEFAULTMAXDEPTHEXP 1000 
// Expansion type
#define MF_EXPANSIONTYPE_BYVALUE 0
#define MF_EXPANSIONTYPE_BYWIDTH 1
#define MF_EXPANSIONTYPE_BYDEPTH_RANDOMWALK 2
#define MF_EXPANSIONTYPE_MONTECARLOTREESEARCH 3
#define MF_EXPANSIONTYPE MF_EXPANSIONTYPE_BYWIDTH
// Use of pruning
#define MF_USEPRUNING true
// Use telemetry
#define MF_USETELEMETRY true
// Reuse world
#define MF_REUSEWORLD true
// Use of depth limit
// TODO: Doesn't work, the world get dropped from teh set of worlds to 
// expand but nothing is done, so the reference to the world is lost
#define MF_LIMITDEPTH true
 
// =========== Interface with the model implementation =============

#include "miniframe-model.h"

// ================= Data structure ===================
typedef struct MFWorld MFWorld;
typedef struct MFTransition {
  // User defined transition
  MFModelTransition _transition;
  // Reference to the world to which this action is applied
  MFWorld* _fromWorld;
  // Reference to the reached world through this action
  // if null it means this action has not been computed
  MFWorld* _toWorld;
  // Array of forecasted POV value of this transition for each actor
  float _values[MF_NBMAXACTOR];
} MFTransition;

typedef struct MFWorld {
  // User defined status of the world
  MFModelStatus _status;
  // Set of transitions reaching this world
  GSet _sources;
  // Array of value of this world from the pov of each actor
  float _values[MF_NBMAXACTOR];
  // Array to memorize the transitions from this world instance
  MFTransition _transitions[MF_NBMAXTRANSITION];
  // Number of transitions from this world
  int _nbTransition;
  // Depth, internal variable used during expansion
  int _depth;
} MFWorld;

typedef enum MFExpansionType {
  MFExpansionTypeValue,
  MFExpansionTypeWidth
} MFExpansionType;
  
typedef struct MiniFrame {
  // Nb of steps
  unsigned int _nbStep;
  // Current world instance
  MFWorld* _curWorld;
  // All the computed world instances, ordered by their value from the
  // pov of the preempting player at the previous step
  GSet _worldsComputed;
  // Set of world waiting to be expanded
  GSet _worldsToExpand;
  // Time limit for expansion, in millisecond
  float _maxTimeExpansion;
  // Time unused during expansion, in millisecond
  float _timeUnusedExpansion;
  // Flag to activate the reuse of previously computed same world
  bool _reuseWorld;
  // Percentage (in [0.0, 1.0]) of world reused during the last 
  // MFExpand()
  float _percWorldReused;
  // The clock considered has start during expansion
  clock_t _startExpandClock;
  // Maximum depth during expansion, if -1 there is no limit
  int _maxDepthExp;
  // Value for pruning during expansion
  float _pruningDeltaVal;
  // Nb of world not found in MFSetCurWorld
  int _nbWorldNotFound;
  // Nb of tries in the MCTS
  int _nbTryMTCS;
  // Max depth reached during last MFExpand
  int _maxDepthExpReached;
#if MF_EXPANSIONTYPE == MF_EXPANSIONTYPE_BYDEPTH_RANDOMWALK
  // Internal flag
  int _expByDepthAppendPos;
#endif
} MiniFrame;

// ================ Functions declaration ====================

// Create a new MiniFrame the initial world 'initStatus'
// The current world is initialized with a copy of 'initStatus'
// Return the new MiniFrame
MiniFrame* MiniFrameCreate(const MFModelStatus* const initStatus);

// Create a new MFWorld with a copy of the MFModelStatus 'status'
// Return the new MFWorld
MFWorld* MFWorldCreate(const MFModelStatus* const status);

// Create a new static MFWorld with a copy of the MFModelStatus 'status'
// Return the new MFWorld
MFWorld MFWorldCreateStatic(const MFModelStatus* const status);

// Create a new static MFTransition for the MFWorld 'world' with the
// MFModelTransition 'transition'
// Return the new MFTransition
MFTransition MFTransitionCreateStatic(const MFWorld* const world,
  const MFModelTransition* const transition);

// Free memory used by the MiniFrame 'that'
void MiniFrameFree(MiniFrame** that);

// Free memory used by the MFWorld 'that'
void MFWorldFree(MFWorld** that);

// Free memory used by properties of the MFTransition 'that'
void MFTransitionFreeStatic(MFTransition* that);

// Get the current MFWorld of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
const MFWorld* MFCurWorld(const MiniFrame* const that);

// Get the GSet of computed MFWorlds of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
const GSet* MFWorldsComputed(const MiniFrame* const that);

// Get the GSet of worlds to expand of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
const GSet* MFWorldsToExpand(const MiniFrame* const that);

// Get the nb of world To expande of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
int MFGetNbWorldsToExpand(const MiniFrame* const that);

// Return true if the expansion algorithm looks in previously 
// computed worlds for same world to reuse, else false
#if BUILDMODE != 0
inline
#endif
bool MFIsWorldReusable(const MiniFrame* const that);

#if MF_REUSEWORLD
// Set the flag controling if the expansion algorithm looks in 
// previously computed worlds for same world to reuse to 'reuse'
#if BUILDMODE != 0
inline
#endif
void MFSetWorldReusable(MiniFrame* const that, const bool reuse);
#endif

// Add the MFWorld 'world' to the computed MFWorlds of the 
// MiniFrame 'that', ordered by the world's value from the pov of 
// actor 'iActor'
#if BUILDMODE != 0
inline
#endif
void MFAddWorldToComputed(MiniFrame* const that, \
  const MFWorld* const world);

// Add the MFWorld 'world' to the world to be expanded of the 
// MiniFrame 'that'
void MFAddWorldToExpand(MiniFrame* const that, \
  const MFWorld* const world);
  
// Get the time limit for expansion of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
float MFGetMaxTimeExpansion(const MiniFrame* const that);

// Get the time unused during last expansion of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
float MFGetTimeUnusedExpansion(const MiniFrame* const that);

// Get the nb of computed worlds of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
int MFGetNbComputedWorlds(const MiniFrame* const that);

// Get the percentage of resued world of the MiniFrame 'that' during 
// the last MFEpxand()
#if BUILDMODE != 0
inline
#endif
float MFGetPercWorldReused(const MiniFrame* const that);

// Get the clock considered has start during expansion
#if BUILDMODE != 0
inline
#endif
clock_t MFGetStartExpandClock(const MiniFrame* const that);

// Set the clock considered has start during expansion to 'c'
#if BUILDMODE != 0
inline
#endif
void MFSetStartExpandClock(MiniFrame* const that, clock_t c);

// Set the time limit for expansion of the MiniFrame 'that' to 
// 'timeLimit', in millisecond
// The time is measured with the function clock(), see "man clock"
// for details
#if BUILDMODE != 0
inline
#endif
void MFSetMaxTimeExpansion(MiniFrame* const that, \
  const float timeLimit);

// Return the MFModelStatus of the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
const MFModelStatus* MFWorldStatus(const MFWorld* const that);

// Expand the MiniFrame 'that' until it reaches its time limit or can't 
// expand anymore
void MFExpand(MiniFrame* that);

// Return the forecasted value of the MFWorld 'that' for the 
// actor 'iActor'.
// This is the best value of the transitions from this world,
// or the value of this world if it has no transition.
float MFWorldGetForecastValue(const MFWorld* const that, 
  const int iActor);

// Get the number of transition for the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
int MFWorldGetNbTrans(const MFWorld* const that);

// Get the number of expandable transition for the MFWorld 'that'
int MFWorldGetNbTransExpandable(const MFWorld* const that);

// Get the MFWorld which the MFTransition 'that' is leading to
#if BUILDMODE != 0
inline
#endif
const MFWorld* MFTransitionToWorld(const MFTransition* const that);

// Set the MFWorld to which the MFTransition 'that' is leading to 
// 'world'
#if BUILDMODE != 0
inline
#endif
void MFTransitionSetToWorld(MFTransition* const that, 
  MFWorld* const world);

// Get the MFWorld which the MFTransition 'that' is coming from
#if BUILDMODE != 0
inline
#endif
const MFWorld* MFTransitionFromWorld(const MFTransition* const that);

// Return true if the MFWorld 'that' has at least one transition to be
// expanded
bool MFTransitionIsExpandable(const MFTransition* const that);

// Get the 'iTrans' MFTransition of the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
const MFTransition* MFWorldTransition(const MFWorld* const that, 
  const int iTrans);

// Get the set of MFTransition reaching the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
const GSet* MFWorldSources(const MFWorld* const that);

// Return the array of values of the MFWorld 'that' for each actor
#if BUILDMODE != 0
inline
#endif
const float* MFWorldValues(const MFWorld* const that);

// Compute the MFModelStatus resulting from the 'iTrans' MFTransition 
// of the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
MFModelStatus MFWorldComputeTransition(const MFWorld* const that, 
  const int iTrans);

// Get the forecast value of the MFWorld 'that' for the actor 'iActor'
float MFWorldGetForecastValue(const MFWorld* const that, int iActor);

// Set the value of the MFTransition 'that' for the actor 'iActor' to 
// 'val'
#if BUILDMODE != 0
inline
#endif
void MFTransitionSetValue(MFTransition* const that, const int iActor,
  const float val);

// Return the value of the MFTransition 'that' for the 
// actor 'iActor'.
#if BUILDMODE != 0
inline
#endif
float MFTransitionGetValue(const MFTransition* const that, 
  const int iActor);

// Return the value of the MFWorld 'that' for the 
// actor 'iActor'.
#if BUILDMODE != 0
inline
#endif
float MFWorldGetValue(const MFWorld* const that, const int iActor);

// Get the best MFModelTransition for the 'iActor'-th actor in the 
// current MFWorld of the MiniFrame 'that'
// Return an undefined MFTransition if the curernt world has no 
// transition
const MFModelTransition* MFBestTransition(
  const MiniFrame* const that, const int iActor);

// Print the MFWorld 'that' on the stream 'stream'
void MFWorldPrint(const MFWorld* const that, FILE* const stream);
  
// Print the MFTransition 'that' on the stream 'stream'
void MFTransitionPrint(const MFTransition* const that, 
  FILE* const stream);
  
// Print the MFWorld 'that' and its MFTransition on the stream 'stream'
void MFWorldTransPrintln(const MFWorld* const that, 
  FILE* const stream);

// Set the current world of the MiniFrame 'that' to match the 
// MFModelStatus 'status'
// If the world is in computed worlds reuse it, else create a new one
void MFSetCurWorld(MiniFrame* const that, 
  const MFModelStatus* const world);

// Print the best forecasted story from the MFWorld 'that' for the 
// actor 'iActor' on the stream 'stream'
void MFWorldPrintBestStoryln(const MFWorld* const that, 
  const int iActor, FILE* const stream);

// Set the values of the MFWorld 'that' to 'values'
void MFWorldSetValues(MFWorld* const that, const float* const values);

// Return the max depth during expansion for the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
int MFGetMaxDepthExp(const MiniFrame* const that);

#if MF_LIMITDEPTH
// Set the max depth during expansion for the MiniFrame 'that' to 'depth'
// If depth is less than -1 it is converted to -1
// If the expansion type is not by width the max expansion depth is 
// ignored during expansion
#if BUILDMODE != 0
inline
#endif
void MFSetMaxDepthExp(MiniFrame* const that, const int depth);
#endif

// Return the type of expansion for the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
MFExpansionType MFGetExpansionType(const MiniFrame* const that);

// Return true if the MFTransition is expanded, false else
#if BUILDMODE != 0
inline
#endif
bool MFTransitionIsExpanded(const MFTransition* const that);

#if MF_USEPRUNING
// Set the pruning threshold during expansion for the MiniFrame 'that' 
// to 'val'
#if BUILDMODE != 0
inline
#endif
void MFSetPruningDeltaVal(MiniFrame* const that, const float val);
#endif

// Get the pruning threshold during expansion for the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
float MFGetPruningDeltaVal(const MiniFrame* const that);

// Get the nb of worlds not found during MFSetCurWorld
#if BUILDMODE != 0
inline
#endif
int MFGetNbWorldNotFound(const MiniFrame* const that);

// Get the max depth reached during last call of MFExpand
#if BUILDMODE != 0
inline
#endif
int MFGetMaxDepthExpReached(const MiniFrame* const that);

// ================ Inliner ====================

#if BUILDMODE != 0
#include "miniframe-inline.c"
#endif


#endif
