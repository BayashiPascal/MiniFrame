// ============ MINIFRAME.C ================

// ================= Include =================

#include "miniframe.h"
#if BUILDMODE == 0
#include "miniframe-inline.c"
#endif

// ================ Functions declaration ====================

// Return true if the MFWorld 'that' should be pruned during search for
// worlds to expand when reaching it through transition 'trans',
// false else
bool MFWorldIsPrunedDuringExpansion(const MFWorld* const that, 
  const MiniFrame* const mf, const MFTransition* const trans);

// Search in computed worlds of the MiniFrame 'that' if there is 
// one with same status as the MFModelStatus 'status'
// If there is one return it, if not return null
MFWorld* MFSearchWorld(const MiniFrame* const that, 
  const MFModelStatus* const status);

// Set the MFWorld 'toWorld' has the result of the 'iTrans' transition
// of the world 'that'
// Update the value of the transition
void MFWorldSetTransitionToWorld(
  MFWorld* const that, const int iTrans, MFWorld* const toWorld);

// Update backward the forecast values for each 
// transitions leading to the MFWorld 'world' in the MiniFrame 'that'
// Use a penalty growing with each recursive call to 
// MFUpdateForecastValues to give priority to fastest convergence to 
// best solution
// Avoid infinite loop due to reuse of computed worlds by putting 
// visited world in the set 'updatedWorld'
void MFUpdateForecastValues(MiniFrame* const that, 
  const MFWorld* const world, int delayPenalty, 
  GSet* const updatedWorld);

// Update the values of the MFTransition 'that' with 'val'
// Return true if the value has been updated, else false
bool MFTransitionUpdateValues(MFTransition* const that, const float* val);

// Pop a MFTransition from the sources of the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
MFTransition* MFWorldPopSource(MFWorld* const that);

// Remove the MFTransition 'source' from the sources of the 
// MFWorld 'that'
void MFWorldRemoveSource(MFWorld* const that, 
  const MFTransition* const source);

// Get the best MFModelTransition for the 'iActor'-th actor in the 
// MFWorld 'that'
// Return NULL if the world has no transition
const MFModelTransition* MFWorldBestTransition(
  const MFWorld* const that, const int iActor);

// ================ Functions implementation ====================

// Create a new MiniFrame the initial world 'initStatus'
// The current world is initialized with a copy of 'initStatus'
// Return the new MiniFrame
MiniFrame* MiniFrameCreate(const MFModelStatus* const initStatus) {
#if BUILDMODE == 0
  if (initStatus == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'initStatus' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Allocate memory
  MiniFrame *that = PBErrMalloc(MiniFrameErr, sizeof(MiniFrame));
  // Set properties
  that->_nbStep = 0;
  MFSetMaxTimeExpansion(that, MF_DEFAULTTIMEEXPANSION);
  that->_curWorld = MFWorldCreate(initStatus);
  that->_worlds = GSetCreateStatic();
  that->_worldsToExpand = GSetCreateStatic();
  that->_worldsToFree = GSetCreateStatic();
  that->_worldsOnHold = GSetCreateStatic();
  MFAddWorldToComputed(that, MFCurWorld(that));
  that->_timeUnusedExpansion = 0.0;
  that->_reuseWorld = false;
  that->_percWorldReused = 0.0;
  that->_startExpandClock = 0;
  that->_maxDepthExp = MF_DEFAULTMAXDEPTHEXP;
  that->_expansionType = MFExpansionTypeValue;
  that->_nbTransMonteCarlo = MF_NBTRANSMONTECARLO;
  that->_pruningDeltaVal = MF_PRUNINGDELTAVAL;
  // Return the new MiniFrame
  return that;
}

// Create a new MFWorld with a copy of the MFModelStatus 'status'
// Return the new MFWorld
MFWorld* MFWorldCreate(const MFModelStatus* const status) {
#if BUILDMODE == 0
  if (status == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'status' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Allocate memory
  MFWorld *that = PBErrMalloc(MiniFrameErr, sizeof(MFWorld));
  // Set the status
  MFModelStatusCopy(status, &(that->_status));
  // Initialise the set of transitions reaching this world
  that->_sources = GSetCreateStatic();
  // Set the values
  MFModelStatusGetValues(status, that->_values);
  // Set the possible transitions from this world 
  MFModelTransition transitions[MF_NBMAXTRANSITION];
  MFModelStatusGetTrans(status, transitions, &(that->_nbTransition));
  MFTransition* thatTransitions = that->_transitions;
  for (int iTrans = that->_nbTransition; iTrans--;)
    thatTransitions[iTrans] = 
      MFTransitionCreateStatic(that, transitions + iTrans);
  // Init the depth
  that->_depth = 0;
  // Return the new MFWorld
  return that;
}

// Create a new static MFTransition for the MFWorld 'world' with the
// MFModelTransition 'transition'
// Return the new MFTransition
MFTransition MFTransitionCreateStatic(const MFWorld* const world,
  const MFModelTransition* const transition) {
#if BUILDMODE == 0
  if (world == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'world' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the new action
  MFTransition that;
  // Set properties
  that._transition = *transition;
  that._fromWorld = (MFWorld*)world;
  that._toWorld = NULL;
  float* thatValues = that._values;
  for (int iActor = MF_NBMAXACTOR; iActor--;)
    thatValues[iActor] = 0.0;
  // Return the new MFTransition
  return that;
}

// Free memory used by the MiniFrame 'that'
void MiniFrameFree(MiniFrame** that) {
  // Check argument
  if (that == NULL || *that == NULL) return;
  // Free memory
  (*that)->_curWorld = NULL;
  while(MFGetNbComputedWorlds(*that) > 0) {
    MFWorld* world = GSetPop(&((*that)->_worlds));
    MFAddWorldToFree(*that, world);
  }
  while(MFGetNbWorldsToExpand(*that) > 0) {
    MFWorld* world = GSetPop(&((*that)->_worldsToExpand));
    MFAddWorldToFree(*that, world);
  }
  while(MFGetNbWorldsOnHold(*that) > 0) {
    MFWorld* world = GSetPop(&((*that)->_worldsOnHold));
    MFAddWorldToFree(*that, world);
  }
  MFFreeDisposableWorld(*that);
  free(*that);
  *that = NULL;
}

// Free memory used by the MFWorld 'that'
void MFWorldFree(MFWorld** that) {
  // Check argument
  if (that == NULL || *that == NULL) return;
  // Free memory
  GSetFlush(&((*that)->_sources));
  MFModelStatusFreeStatic(&((*that)->_status));
  MFTransition* thatTransitions = (*that)->_transitions;
  for (int iAct = (*that)->_nbTransition; iAct--;) {
    if (thatTransitions[iAct]._toWorld != NULL)
      MFTransitionFreeStatic(thatTransitions + iAct);
  }
  free(*that);
  *that = NULL;
}

// Free memory used by properties of the MFTransition 'that'
void MFTransitionFreeStatic(MFTransition* that) {
  // Check argument
  if (that == NULL) return;
  // Free memory
  MFModelTransitionFreeStatic(&(that->_transition));
}

// Expand the MiniFrame 'that' until it reaches its time limit or can't 
// expand anymore
void MFExpand(MiniFrame* that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the time at beginning of the whole 
  // expansion process
  clock_t clockStart = MFGetStartExpandClock(that);
  // Declare a variable to memorize the maximum time used for one 
  // step of expansion
  double maxTimeOneStep = 0.0;
  // Declare a variable to memorize the number of reused worlds
  int nbReusedWorld = 0;
  // Declare a variable to memorize the number of worlds searched for 
  // reuse
  int nbWorldSearchForReuse = 0;
  // Declare a variable to memorize the time spent expanding
  double timeUsed = 
    ((double)(clock() - clockStart)) / MF_MILLISECTOCLOCKS;
  // If there are worlds on hold, put them back into the set of worlds 
  // to expand
  if (MFGetNbWorldsOnHold(that) > 0) {
    GSetMerge((GSet*)MFWorldsToExpand(that), (GSet*)MFWorldsOnHold(that));
  }
  // Ensure the cur world is the first to be expanded
  GSetAppend((GSet*)MFWorldsToExpand(that), MFCurWorld(that));
  GSetRemoveFirst(&(that->_worlds), MFCurWorld(that));
  // Declare a variable to memorize the limit of expansion by depth
  int limitDepthExpansion = 
    MFCurWorld(that)->_depth + MFGetMaxDepthExp(that);
  // Loop until we have time for one more step of expansion or there
  // is no world to expand
  // Take care of clock() wrapping around
  while (timeUsed + maxTimeOneStep < MFGetMaxTimeExpansion(that) &&
    GSetNbElem(MFWorldsToExpand(that)) > 0 &&
    timeUsed >= 0.0) {
    // Declare a variable to memorize the time at the beginning of one
    // step of expansion
    clock_t clockStartLoop = clock();
    // Drop the world to expand with highest value
    MFWorld* worldToExpand = GSetDrop((GSet*)MFWorldsToExpand(that));
    // If this world is above the limit by depth of expansion
    if (worldToExpand->_depth > limitDepthExpansion) {
      // Put it to the worlds on hold
      MFAddWorldToOnHold(that, worldToExpand);
    // Else, if this world is disposable
    } else if (worldToExpand != MFCurWorld(that) && (
      GSetNbElem(MFWorldSources(worldToExpand)) == 0 || 
      MFModelStatusIsDisposable(MFWorldStatus(worldToExpand), 
      MFWorldStatus(MFCurWorld(that))))) {
      // Add it to the worlds to free
      MFAddWorldToFree(that, worldToExpand);
    // Else, the world needs to be expanded
    } else {
      // Get the number of expandable transition
      int nbTransExpandable = 
        MFWorldGetNbTransExpandable(worldToExpand);
      // Get the threshold for expansion to activate MonteCarlo when 
      // there are too many transitions
      float thresholdMonteCarlo = 2.0;
      if (nbTransExpandable > 0)
        thresholdMonteCarlo = (float)MFGetNbTransMonteCarlo(that) / 
          (float)nbTransExpandable;
      // For each transitions from the expanded world and until we have
      // time available
      // Take care of clock() wrapping around
      for (int iTrans = 0; iTrans < MFWorldGetNbTrans(worldToExpand) &&
        timeUsed + maxTimeOneStep < MFGetMaxTimeExpansion(that) && 
        timeUsed >= 0.0;
        ++iTrans) {
        // If this transition is expandable
        const MFTransition* const trans = 
          MFWorldTransition(worldToExpand, iTrans);
        if (MFTransitionIsExpandable(trans) && 
          (thresholdMonteCarlo >= 1.0 || rnd() < thresholdMonteCarlo)) {
          // Expand through this transition
          MFModelStatus status = 
            MFWorldComputeTransition(worldToExpand, iTrans);
          // Search if the resulting status has already been computed,
          // MFSearchWorld always return NULL if the reuse mode
          // is false
          MFWorld* sameWorld = MFSearchWorld(that, &status);
          // Increment the number of worlds searched for reuse
          ++nbWorldSearchForReuse;
          // If there is no world to reuse
          if (sameWorld == NULL) {
            // Create a MFWorld for the new status
            MFWorld* expandedWorld = MFWorldCreate(&status);
            // Update the depth of the world
            expandedWorld->_depth = worldToExpand->_depth + 1;
            // If the expanded world is pruned
            if (MFWorldIsPrunedDuringExpansion(
              expandedWorld, that, trans)) {
              // Add it to the computed worlds
              MFAddWorldToComputed(that, expandedWorld);
            // Else, the world is not pruned
            } else {
              // Add the world to the set of worlds to expand 
              MFAddWorldToExpand(that, expandedWorld);
            }
            // Set the expanded world as the result of the transition
            MFWorldSetTransitionToWorld(
              worldToExpand, iTrans, expandedWorld);
          } else {
            // Increment the number of reused world
            ++nbReusedWorld;
            // Set the already computed one as the result of the 
            // transition 
            MFWorldSetTransitionToWorld(worldToExpand, iTrans, sameWorld);
          }
        }
      }
      // If all the expandable transitions have been expanded
      if (MFWorldGetNbTransExpandable(worldToExpand) == 0) {
        // Move the expanded world from the worlds to expands to the 
        // computed worlds
        MFAddWorldToComputed(that, worldToExpand);
      // Else, the world still needs to be expanded
      } else {
        // Put it to the worlds on hold
        MFAddWorldToOnHold(that, worldToExpand);
      }
      // Update the total time used from beginning of expansion 
      timeUsed = 
        ((double)(clock() - clockStart)) / MF_MILLISECTOCLOCKS;
      // Update backward the forecast values for each transitions 
      // leading to the expanded world according to its new transitions
      GSet updatedWorld = GSetCreateStatic();
      MFUpdateForecastValues(that, worldToExpand, 0, &updatedWorld);
      GSetFlush(&updatedWorld);
    }
    // Declare a variable to memorize the time at the end of one
    // step of expansion
    clock_t clockEndLoop = clock();
    // Calculate the time for this step
    double timeOneStep = 
      ((double)(clockEndLoop - clockStartLoop)) / MF_MILLISECTOCLOCKS;  
    // Update max time used by one step
    if (maxTimeOneStep < timeOneStep)
      maxTimeOneStep = timeOneStep;
    // Update the total time used from beginning of expansion 
    timeUsed = 
      ((double)(clockEndLoop - clockStart)) / MF_MILLISECTOCLOCKS;
  }
  // Update the total time used from beginning of expansion 
  timeUsed = ((double)(clock() - clockStart)) / MF_MILLISECTOCLOCKS;
  // Take care of clock() wrapping around
  if (timeUsed < 0.0)
    timeUsed = MFGetMaxTimeExpansion(that);
  // Telemetry for debugging
  that->_timeUnusedExpansion = MFGetMaxTimeExpansion(that) - timeUsed;
  if (nbWorldSearchForReuse > 0)
    that->_percWorldReused = 
      ((float)nbReusedWorld) / ((float)nbWorldSearchForReuse);
  else
    that->_percWorldReused = 0.0;
}

// Return true if the MFWorld 'that' should be pruned during search for
// worlds to expand when reaching it through transition 'trans',
// false else
bool MFWorldIsPrunedDuringExpansion(const MFWorld* const that, 
  const MiniFrame* const mf, const MFTransition* const trans) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (mf == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'mf' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (trans == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'trans' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (MFTransitionFromWorld(trans) == NULL) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "The transition has no origin");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the result
  bool ret = false;
  // Get the origin world of the transition
  const MFWorld* const fatherWorld = MFTransitionFromWorld(trans);
  // Get the sente of the father world
  const int sente = MFModelStatusGetSente(MFWorldStatus(fatherWorld));
  // Get the value of the world in argument
  const float val = MFWorldGetForecastValue(that, sente);
  // Loop on transitions from the father world
  for (int iTrans = MFWorldGetNbTrans(fatherWorld); iTrans-- && !ret;) {
    // Get the origin of the current transition
    const MFWorld* const brother = 
      MFTransitionToWorld(MFWorldTransition(fatherWorld, iTrans));
    if (brother != that && brother != NULL) {
      // Get the value of the brother
      const float valBrother = MFWorldGetForecastValue(brother, sente);
      // If the pruning constraint is true
      if (val < valBrother - MFGetPruningDeltaVal(mf))
        // Update the result
        ret = true;
    }
  }
  // Return the result
  return ret;
}
  
// Return true if the MFWorld 'that' has at least one transition to be
// expanded
bool MFWorldIsExpandable(const MFWorld* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the result
  bool isExpandable = false;
  // Loop on transitions
  for (int iTrans = that->_nbTransition; iTrans-- && !isExpandable;) {
    // If this transition has not been computed
    if (MFTransitionIsExpandable(MFWorldTransition(that, iTrans)))
      isExpandable = true;
  }
  // Return the result
  return isExpandable;
}

// Search in computed worlds of the MiniFrame 'that' if there is 
// one with same status as the MFModelStatus 'status'
// If there is one return it, if not return null
MFWorld* MFSearchWorld(const MiniFrame* const that, 
  const MFModelStatus* const status) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (status == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'status' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the returned world
  MFWorld* sameWorld = NULL;
  // If the reuse of worlds is activated
  if (MFIsWorldReusable(that)) {
    if (MFGetNbComputedWorlds(that) > 0) {
      // Loop on computed worlds
      GSetIterForward iter = GSetIterForwardCreateStatic(MFWorlds(that));
      do {
        MFWorld* world = GSetIterGet(&iter);
        // If this world is the same as the searched one
        if (MFModelStatusIsSame(status, MFWorldStatus(world))) {
          sameWorld = world;
        }
      } while (sameWorld == NULL && GSetIterStep(&iter));
    }
    if (sameWorld == NULL && MFGetNbWorldsToExpand(that) > 0) {
      // Loop on worlds to expand
      GSetIterForward iter = 
        GSetIterForwardCreateStatic(MFWorldsToExpand(that));
      do {
        MFWorld* world = GSetIterGet(&iter);
        // If this world is the same as the searched one
        if (MFModelStatusIsSame(status, MFWorldStatus(world))) {
          sameWorld = world;
        }
      } while (sameWorld == NULL && GSetIterStep(&iter));
    }
    if (sameWorld == NULL && MFGetNbWorldsOnHold(that) > 0) {
      // Loop on worlds on hold
      GSetIterForward iter = 
        GSetIterForwardCreateStatic(MFWorldsOnHold(that));
      do {
        MFWorld* world = GSetIterGet(&iter);
        // If this world is the same as the searched one
        if (MFModelStatusIsSame(status, MFWorldStatus(world))) {
          sameWorld = world;
        }
      } while (sameWorld == NULL && GSetIterStep(&iter));
    }
  }
  // Return the found world
  return sameWorld;
}

// Set the MFWorld 'toWorld' has the result of the 'iTrans' transition
// of the MFWorld 'that'
// Update the value of the transition
void MFWorldSetTransitionToWorld(
  MFWorld* const that, const int iTrans, MFWorld* const toWorld) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (iTrans < 0 || iTrans >= that->_nbTransition) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'iTrans' is invalid (0<=%d<%d)",
      iTrans, that->_nbTransition);
    PBErrCatch(MiniFrameErr);
  }
  if (toWorld == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'toWorld' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the transition
  MFTransition* trans = that->_transitions + iTrans;
  // Set the transition result
  trans->_toWorld = toWorld;
  // Add the transition to the sources to the result's world
  GSetAppend(&(toWorld->_sources), trans);
  // Update the forecast value of this transition for each actor
  for (int iActor = MF_NBMAXACTOR; iActor--;)
    MFTransitionSetValue(trans, iActor, 
      MFWorldGetForecastValue(toWorld, iActor));
}

// Return true if the MFTransition 'that' is expandable, i.e. its
// 'toWorld' is null, else return false
bool MFTransitionIsExpandable(const MFTransition* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // If the transition has already been expanded
  if (MFTransitionToWorld(that) != NULL) {
    // Return false
    return false;
  // Else, the transition has not been expanded yet
  } else {
    // Get the origin of the transition
    const MFWorld* fromWorld = MFTransitionFromWorld(that);
    // Declare a variable to memorize if the transition has a brother
    // which leads to an end world
    bool hasEndWorldBrother = false;
    // For each brother transition, until we have found an end world
    for (int iTrans = MFWorldGetNbTrans(fromWorld); 
      iTrans-- && !hasEndWorldBrother;) {
      // Get the brother transition's toWorld
      const MFWorld* brother = 
        MFTransitionToWorld(MFWorldTransition(fromWorld, iTrans));
      // If the brother world is an end world
      if (brother != NULL &&
        MFModelStatusIsEnd(MFWorldStatus(brother))) {
        // Set the flag
        hasEndWorldBrother = true;
      }
    }
    // If the transition has a brother leading to an end world
    if (hasEndWorldBrother)
      // This transition is not expandable
      return false;
    // Else, the transition has no brother leading to an end world
    else
      // This transition is expandable
      return true;
  }
}

// Return the forecasted value of the MFWorld 'that' for the 
// actor 'iActor'.
// This is the best value of the transitions from this world,
// or the value of this world if it has no transition.
float MFWorldGetForecastValue(const MFWorld* const that, 
  const int iActor) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (iActor < 0 || iActor >= MF_NBMAXACTOR) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'iActor' is invalid (0<=%d<%d)",
      iActor, MF_NBMAXACTOR);
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Get the best transition
  const MFTransition* bestTrans = 
    (const MFTransition*)MFWorldBestTransition(that, iActor);
  // If there is a best transition
  if (bestTrans != NULL) {
    // Return the value of the best transition
    return MFTransitionGetValue(bestTrans, iActor);
  // Else, there was no transition from this world
  } else {
    // Return the value of this world
    return MFWorldGetValue(that, iActor);
  }
}

// Get the best MFModelTransition for the 'iActor'-th actor in the 
// current MFWorld of the MiniFrame 'that'
// Return NULL if the current world has no transition
const MFModelTransition* MFBestTransition(
  const MiniFrame* const that, const int iActor) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (iActor < 0 || iActor >= MF_NBMAXACTOR) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'iActor' is invalid (0<=%d<%d)",
      iActor, MF_NBMAXACTOR);
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Return the best transition
  return MFWorldBestTransition(MFCurWorld(that), iActor);
}

// Get the best MFModelTransition for the 'iActor'-th actor in the 
// MFWorld 'that'
// Return NULL if the world has no transition
const MFModelTransition* MFWorldBestTransition(
  const MFWorld* const that, const int iActor) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (iActor < 0 || iActor >= MF_NBMAXACTOR) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'iActor' is invalid (0<=%d<%d)",
      iActor, MF_NBMAXACTOR);
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the highest value among transitions
  float valBestTrans = 0.0;
  // Declare a variable to memorize the transition with highest value
  const MFTransition* bestTrans = NULL;
  // Get the sente
  int sente = MFModelStatusGetSente(MFWorldStatus(that));
  sente = (sente == -1 ? iActor : sente);
  // Loop on transitions
  for (int iTrans = MFWorldGetNbTrans(that); iTrans--;) {
    // Declare a variable to memorize the transition
    const MFTransition* const trans = 
      MFWorldTransition(that, iTrans);
    // If this transitions has been expanded
    if (MFTransitionIsExpanded(trans)) {
      // Get the value of the transition from the point of view of 
      // the sente
      float val = MFTransitionGetValue(trans, sente);
      // If it's the first considered transition
      if (bestTrans == NULL) {
        // Init the best value with the value of this transition
        valBestTrans = val;
        // Init the best transition
        bestTrans = trans;
      // Else if the value is better
      } else if (valBestTrans < val) {
        // Update the best transition
        valBestTrans = val;
        bestTrans = trans;
      }
    }
  }
  // Return the best transition
  return (const MFModelTransition*)bestTrans;
}

// Update backward the forecast values for each 
// transitions leading to the MFWorld 'world' in the MiniFrame 'that'
// Use a penalty growing with each recursive call to 
// MFUpdateForecastValues to give priority to fastest convergence to 
// best solution
// Avoid infinite loop due to reuse of computed worlds by putting 
// visited world in the set 'setWorld'
void MFUpdateForecastValues(MiniFrame* const that, 
  const MFWorld* const world, int delayPenalty, 
  GSet* const updatedWorld) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (world == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'world' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (updatedWorld == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'updatedWorld' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // If the world has not been updated yet and has ancestor
  if (GSetNbElem(MFWorldSources(world)) > 0 &&
    GSetFirstElem(updatedWorld, world) == NULL) {
    GSetPush(updatedWorld, (void*)world);
    // Get the sente of the world
    int sente = MFModelStatusGetSente(MFWorldStatus(world));
    // Search the best transition of the world
    MFTransition* bestTrans = 
      (MFTransition*)MFWorldBestTransition(world, sente);
    // Declare a variable to memorize the updated forecast values
    float updatedForecastValues[MF_NBMAXACTOR] = {0.0};
    // Declare a pointer toward the forecast values
    float* forecastValues = NULL;
    if (bestTrans == NULL)
      forecastValues = ((MFWorld*)world)->_values;
    else
      forecastValues = bestTrans->_values;
    // Calculate the updated forecast values
    for (int iActor = MF_NBMAXACTOR; iActor--;)
      updatedForecastValues[iActor] = 
        forecastValues[iActor] - (float)delayPenalty * PBMATH_EPSILON;
    // For each transition to the world
    GSetIterForward iter = 
      GSetIterForwardCreateStatic(MFWorldSources(world));
    do {
      // Get the transition
      MFTransition* const trans = GSetIterGet(&iter);
      // Update the values of the transition
      bool updated = 
        MFTransitionUpdateValues(trans, updatedForecastValues);
      // If the values has been modified
      if (updated) {
        // Update recursively the source of the transition
        MFUpdateForecastValues(that, MFTransitionFromWorld(trans),
          delayPenalty + 1, updatedWorld);
      }
    } while (GSetIterStep(&iter));
    // Remove the world for the set of visited world to allow
    // another path to reupdate it later
    GSetRemoveFirst(updatedWorld, (void*)world);
  }
}

// Update the values of the MFTransition 'that' with 'val'
// Return true if the value has been updated, else false
bool MFTransitionUpdateValues(MFTransition* const that, const float* val) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (val == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'val' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the returned flag
  bool updated = false;
  // Update values
  float* const thatValues = that->_values;
  for (int iActor = MF_NBMAXACTOR; iActor--;) {
    // If the new value is different from the current one
    if (!ISEQUALF(thatValues[iActor], val[iActor])) {
      thatValues[iActor] = val[iActor];
      updated = true;
    }
  }
  // Return the flag
  return updated;
}

// Print the MFWorld 'that' on the stream 'stream'
void MFWorldPrint(const MFWorld* const that, FILE* const stream) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (stream == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'stream' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  fprintf(stream, "(");
  MFModelStatusPrint(MFWorldStatus(that), stream);
  fprintf(stream, ") values[");
  for (int iActor = 0; iActor < MF_NBMAXACTOR; ++iActor) {
    fprintf(stream, "%f", MFWorldGetValue(that, iActor));
    if (iActor < MF_NBMAXACTOR - 1)
      fprintf(stream, ",");
  }
  fprintf(stream, "]");
  fprintf(stream, " forecast[");
  for (int iActor = 0; iActor < MF_NBMAXACTOR; ++iActor) {
    fprintf(stream, "%f", MFWorldGetForecastValue(that, iActor));
    if (iActor < MF_NBMAXACTOR - 1)
      fprintf(stream, ",");
  }
  fprintf(stream, "]");
} 
 
// Print the MFTransition 'that' on the stream 'stream'
void MFTransitionPrint(const MFTransition* const that, 
  FILE* const stream) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (stream == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'stream' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  fprintf(stream, "transition from (");
  MFModelStatusPrint(
    MFWorldStatus(MFTransitionFromWorld(that)), stream);
  fprintf(stream, ") to (");
  if (MFTransitionToWorld(that) != NULL)
    MFModelStatusPrint(
      MFWorldStatus(MFTransitionToWorld(that)), stream);
  else
    fprintf(stream, "<null>");
  fprintf(stream, ") through (");
  MFModelTransitionPrint((MFModelTransition*)that, stream);
  fprintf(stream, ") values[");
  for (int iActor = 0; iActor < MF_NBMAXACTOR; ++iActor) {
    fprintf(stream, "%f", that->_values[iActor]);
    if (iActor < MF_NBMAXACTOR - 1)
      fprintf(stream, ",");
  }
  fprintf(stream, "]");
}
  
// Print the MFWorld 'that' and its MFTransition on the stream 'stream'
void MFWorldTransPrintln(const MFWorld* const that, 
  FILE* const stream) {
  #if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (stream == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'stream' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  MFWorldPrint(that, stream);
  fprintf(stream, "\n");
  for (int iTrans = 0; iTrans < MFWorldGetNbTrans(that); ++iTrans) {
    fprintf(stream, "  %d) ", iTrans);
    MFTransitionPrint(MFWorldTransition(that, iTrans), stream);
    fprintf(stream, "\n");
  }
}
  
// Set the current world of the MiniFrame 'that' to match the 
// MFModelStatus 'status'
// If the world is in computed worlds reuse it, else create a new one
void MFSetCurWorld(MiniFrame* const that, 
  const MFModelStatus* const status) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (status == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'status' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a flag to memorize if we have found the world
  bool flagFound = false;
  // Flag to manage removal of worlds from sets
  bool moved = false;
  // If there are computed worlds
  if (MFGetNbComputedWorlds(that) > 0) {
    // Loop on computed worlds
    GSetIterForward iter = GSetIterForwardCreateStatic(MFWorlds(that));
    do {
      moved = false;
      MFWorld* world = GSetIterGet(&iter);
      // If this is the current world
      if (!flagFound && 
        MFModelStatusIsSame(MFWorldStatus(world), status)) {
        // Ensure that the status is exactly the same by copying the 
        // MFModelStatus struct, in case MFModelStatusIsSame refers only
        // to a subset of properties of the MFModelStatus
        memcpy(world, status, sizeof(MFModelStatus));
        // Update the curWorld in MiniFrame
        that->_curWorld = world;
        flagFound = true;
      // Else if this world is disposable
      } else if (GSetNbElem(MFWorldSources(world)) == 0 || 
        MFModelStatusIsDisposable(MFWorldStatus(world), 
        MFWorldStatus(MFCurWorld(that)))) {
        // Remove it from the set of worlds to expand
        moved = GSetIterRemoveElem(&iter);
        // Add it to the worlds to free
        MFAddWorldToFree(that, world);
      }
    } while (moved || GSetIterStep(&iter));
  }
  // If we haven't found the searched status
  if (!flagFound) {
    // If there are worlds to expand
    if (MFGetNbWorldsToExpand(that) > 0) {
      // Loop on worlds to expand
      GSetIterForward iter = 
        GSetIterForwardCreateStatic(MFWorldsToExpand(that));
      do {
        moved = false;
        MFWorld* world = GSetIterGet(&iter);
        // If this is the current world
        if (MFModelStatusIsSame(MFWorldStatus(world), status)) {
          // Ensure that the status is exactly the same by copying the 
          // MFModelStatus struct, in case MFModelStatusIsSame refers only
          // to a subset of properties of the MFModelStatus
          memcpy(world, status, sizeof(MFModelStatus));
          // Update the curWorld in MiniFrame
          that->_curWorld = world;
          flagFound = true;
        }
      } while (!flagFound && (moved || GSetIterStep(&iter)));
    }
  }
  // If we haven't found the searched status
  if (!flagFound) {
    // If there are worlds on hold
    if (MFGetNbWorldsOnHold(that) > 0) {
      // Loop on worlds on hold
      GSetIterForward iter = 
        GSetIterForwardCreateStatic(MFWorldsOnHold(that));
      do {
        moved = false;
        MFWorld* world = GSetIterGet(&iter);
        // If this is the current world
        if (MFModelStatusIsSame(MFWorldStatus(world), status)) {
          // Ensure that the status is exactly the same by copying the 
          // MFModelStatus struct, in case MFModelStatusIsSame refers only
          // to a subset of properties of the MFModelStatus
          memcpy(world, status, sizeof(MFModelStatus));
          // Update the curWorld in MiniFrame
          that->_curWorld = world;
          flagFound = true;
        }
      } while (!flagFound && (moved || GSetIterStep(&iter)));
    }
  }
  // If we haven't found the searched status
  if (!flagFound) {
    // Create a new MFWorld with the current status
    MFWorld* world = MFWorldCreate(status);
    // Add it to the computed worlds
    MFAddWorldToComputed(that, world);
    // Update the current world
    that->_curWorld = world;
  }
  // If the expansion mode is by value and there are worlds to expand
  if (MFGetExpansionType(that) == MFExpansionTypeValue &&
    MFGetNbWorldsToExpand(that) > 0) {
    // Look for disposable worlds in the worlds to expand
    // Loop on worlds to expand
    GSetIterForward iter = 
      GSetIterForwardCreateStatic(MFWorldsToExpand(that));
    do {
      moved = false;
      MFWorld* world = GSetIterGet(&iter);
      if (GSetNbElem(MFWorldSources(world)) == 0 || 
        MFModelStatusIsDisposable(MFWorldStatus(world), 
        MFWorldStatus(MFCurWorld(that)))) {
        // Remove it from the set of worlds to expand
        moved = GSetIterRemoveElem(&iter);
        // Add it to the worlds to free
        MFAddWorldToFree(that, world);
      }
    } while (moved || GSetIterStep(&iter));
  }
}

// Add the MFWorld 'world' to the world to be freeed of the 
// MiniFrame 'that'
void MFAddWorldToFree(MiniFrame* const that, MFWorld* const world) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (world == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'world' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Remove this world from its sources
  while (GSetNbElem(MFWorldSources(world)) > 0) {
    MFTransition* transSource = MFWorldPopSource(world);
    MFTransitionSetToWorld(transSource, NULL);
  }
  // Remove this world from the sources of its next worlds
  for (int iTrans = MFWorldGetNbTrans(world); iTrans--;) {
    const MFTransition* trans = MFWorldTransition(world, iTrans);
    MFWorld* toWorld = (MFWorld*)MFTransitionToWorld(trans);
    if (toWorld != NULL)
      MFWorldRemoveSource(toWorld, trans);
  }
  // Add the world to the set of disposable worlds
  GSetPush(&(that->_worldsToFree), world);  
}

// Free the memory used by the disposable worlds in the computed worlds
// of the MinFrame 'that'
void MFFreeDisposableWorld(MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Loop on free-able worlds
  while (MFGetNbWorldsToFree(that) > 0) {
    MFWorld* world = GSetPop((GSet*)MFWorldsToFree(that));
    // If this is not the current world
    if (world != MFCurWorld(that)) {
      // Free memory
      MFWorldFree(&world);
    }
  }
}

// Remove the MFTransition 'source' from the sources of the 
// MFWorld 'that'
void MFWorldRemoveSource(MFWorld* const that, 
  const MFTransition* const source) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (source == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'source' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // If the world has sources
  if (GSetNbElem(MFWorldSources(that)) > 0) {
    bool removed = false;
    // Loop on transitions
    GSetIterForward iter = 
      GSetIterForwardCreateStatic(MFWorldSources(that));
    do {
      MFTransition* trans = GSetIterGet(&iter);
      // If it's the transition to be removed
      if (trans == source) {
        (void)GSetIterRemoveElem(&iter);
      }
    } while (!removed && GSetIterStep(&iter));
  }
}


// Pop a MFTransition from the sources of the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
MFTransition* MFWorldPopSource(MFWorld* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return GSetPop(&(that->_sources));
}

// Print the best forecasted story from the MFWorld 'that' for the 
// actor 'iActor' on the stream 'stream'
void MFWorldPrintBestStoryln(const MFWorld* const that, 
  const int iActor, FILE* const stream) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (iActor < 0 || iActor >= MF_NBMAXACTOR) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'iActor' is invalid (0<=%d<%d)",
      iActor, MF_NBMAXACTOR);
    PBErrCatch(MiniFrameErr);
  }
  if (stream == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'stream' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the current displayed world
  const MFWorld* curWorld = that;
  // Declare a GSet to manage infinite loop
  GSet setWorld = GSetCreateStatic();
  // Loop until the end of the forecast
  while (curWorld != NULL) { 
    // Display the current world
    //MFWorldPrint(curWorld, stream);
    //fprintf(stream, "\n");
    MFWorldTransPrintln(curWorld, stream);
    // Add the world to the set of visited worlds
    GSetAppend(&setWorld, (void*)curWorld);
    // If we are not at an end status
    if (!MFModelStatusIsEnd(MFWorldStatus(curWorld))) {
      // Get the sente for the current world
      int sente = MFModelStatusGetSente(MFWorldStatus(curWorld));
      // If it's a simultaneous game
      if (sente == -1)
        sente = iActor;
      // Get the best transition from this world
      const MFModelTransition* bestTrans = 
        MFWorldBestTransition(curWorld, sente);
      // If there is no transition
      if (bestTrans == NULL) {
        // Stop the story here
        curWorld = NULL;
      // Else, there is a best transition
      } else {
        // Print the best transition
        fprintf(stream, "--> ");
        MFTransitionPrint((const MFTransition*)bestTrans, stream);
        fprintf(stream, "\n");
        // Move to the world resulting from the best transition
        curWorld = MFTransitionToWorld((const MFTransition*)bestTrans);
      }
    } else {
      fprintf(stream, "--> reached a end status\n");
      curWorld = NULL;
    }
    // If we reach a world already visited
    if (curWorld != NULL && GSetFirstElem(&setWorld, curWorld) != NULL) {
      MFWorldPrint(curWorld, stream);
      fprintf(stream, "\n");
      fprintf(stream, "--> infinite loop in best story, quit\n");
      curWorld = NULL;
    }
  }
  // Free memory
  GSetFlush(&setWorld);
}

// Set the values of the MFWorld 'that' to 'values'
void MFWorldSetValues(MFWorld* const that, const float* const values) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (values == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'values' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  float* thatValues = that->_values;
  for (int iActor = MF_NBMAXACTOR; iActor--;) {
    thatValues[iActor] = values[iActor];
  }
}

// Get the number of expandable transition for the MFWorld 'that'
int MFWorldGetNbTransExpandable(const MFWorld* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the result
  int nb = 0;
  // Loop on transitions
  for (int iTrans = MFWorldGetNbTrans(that); iTrans--;) {
    // Get the transition
    const MFTransition* const trans = MFWorldTransition(that, iTrans);
    // If this transition is expandable
    if (MFTransitionIsExpandable(trans))
      // Increment the result
      ++nb;
  }
  // Return the result
  return nb;
}

// Add the MFWorld 'world' to the world to be expanded of the 
// MiniFrame 'that'
void MFAddWorldToExpand(MiniFrame* const that, \
  const MFWorld* const world) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (world == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'world' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  if (MFGetExpansionType(that) == MFExpansionTypeWidth)
    GSetPush(&(that->_worldsToExpand), world);  
  else
    GSetAddSort(&(that->_worldsToExpand), world, 
      MFWorldGetForecastValue(world, 
      MFModelStatusGetSente(MFWorldStatus(world))));
}



