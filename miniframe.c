// ============ MINIFRAME.C ================

// ================= Include =================

#include "miniframe.h"
#if BUILDMODE == 0
#include "miniframe-inline.c"
#endif

// ================ Functions declaration ====================

// Get the set of worlds to be expanded (having at least one transition
// whose _toWorld is null) for the MiniFrame 'that'
// Stop searching for world if clock() >= clockLimit
GSet MFGetWorldsToExpand(const MiniFrame* const that, 
  const clock_t clockLimit);

// Return true if the MFWorld 'that' has at least one transition to be
// expanded
bool MFWorldIsExpandable(const MFWorld* const that);

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

// Update backward the forecast values for each transitions 
// leading to the MFWorld 'world' in the MiniFrame 'that'
void MFUpdateForecastValues(MiniFrame* const that, 
  const MFWorld* const world, float delayPenalty);

// Update the values of the MFTransition 'that' for each actor with 
// the values 'values'
// Update only if the new value is higher than the current one
// Return true if at least one value has been updated, else false
bool MFTransitionUpdateValues(MFTransition* const that, 
  const float* const values);

// Pop a MFTransition from the sources of the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
MFTransition* MFWorldPopSource(MFWorld* const that);

// Remove the MFTransition 'source' from the sources of the 
// MFWorld 'that'
void MFWorldRemoveSource(MFWorld* const that, 
  const MFTransition* const source);

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
  MFAddWorld(that, MFCurWorld(that), -1);
  that->_timeSearchWorld = MF_DEFAULTTIMEEXPANSION;
  that->_nbWorldExpanded = 0;
  that->_timeUnusedExpansion = 0.0;
  that->_reuseWorld = false;
  that->_percWorldReused = 0.0;
  that->_startExpandClock = 0;
  // Estimate the time used at end of expansion which is the time
  // used to flush a gset
  GSet set = GSetCreateStatic();
  int nb = 100;
  float timeFlush = 0.0;
  do {
    for (int i = nb; i--;)
      GSetPush(&set, NULL);
    clock_t timeStart = clock();
    GSetFlush(&set);
    clock_t timeEnd = clock();
    timeFlush = ((double)(timeEnd - timeStart)) / MF_MILLISECTOCLOCKS;
  } while (timeFlush < 0.0);
  that->_timeEndExpansion = timeFlush / (float)nb;
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
  for (int iActor = MF_NBMAXACTOR; iActor--;)
    that->_values[iActor] = 0.0;
  MFModelStatusGetValues(status, that->_values);
  // Set the possible transitions from this world 
  MFModelTransition transitions[MF_NBMAXTRANSITION];
  MFModelStatusGetTrans(status, transitions, &(that->_nbTransition));
  for (int iTrans = that->_nbTransition; iTrans--;)
    that->_transitions[iTrans] = 
      MFTransitionCreateStatic(that, transitions + iTrans);
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
  for (int iActor = MF_NBMAXACTOR; iActor--;)
    that._values[iActor] = 0.0;
  // Return the new MFTransition
  return that;
}

// Free memory used by the MiniFrame 'that'
void MiniFrameFree(MiniFrame** that) {
  // Check argument
  if (that == NULL || *that == NULL) return;
  // Free memory
  while(GSetNbElem(&((*that)->_worlds)) > 0) {
    MFWorld* world = GSetPop(&((*that)->_worlds));
    MFWorldFree(&world);
  }
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
  for (int iAct = (*that)->_nbTransition; iAct--;) {
    if ((*that)->_transitions[iAct]._toWorld != NULL)
      MFTransitionFreeStatic((*that)->_transitions + iAct);
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
  // Create the set of world instances to be expanded, ordered by 
  // world's value from the point of view of the preempting actor
  // for each world
  // The time available for this step is limited to avoid spending
  // time to search for worlds to expand and finally not having time
  // to compute them
  clock_t clockLimit = clockStart + 
    that->_timeSearchWorld * MF_MILLISECTOCLOCKS;
  GSet worldsToExpand = MFGetWorldsToExpand(that, clockLimit);
  // Memorize the number of worlds to expand
  int nbWorldToExpand = GSetNbElem(&worldsToExpand);
  // Declare a variable to memorize the time spend expanding
  double timeUsed = 
    ((double)(clock() - clockStart)) / MF_MILLISECTOCLOCKS;
  // Declare a variable to memorize the number of reused worlds
  int nbReusedWorld = 0;
  // Loop until we have time for one more step of expansion or there
  // is no world to expand
  // Take care of clock() wrapping around
  while (timeUsed >= 0.0 &&
    timeUsed + maxTimeOneStep + 
    MFGetTimeEndExpansion(that) * GSetNbElem(&worldsToExpand) < 
    MFGetMaxTimeExpansion(that) &&
    GSetNbElem(&worldsToExpand) > 0) {
    // Declare a variable to memorize the time at the beginning of one
    // step of expansion
    clock_t clockStartLoop = clock();
    // Drop the world to expand with highest value
    MFWorld* worldToExpand = GSetDrop(&worldsToExpand);
    // Get the sente for this world
    int sente = MFModelStatusGetSente(MFWorldStatus(worldToExpand));
    // For each transitions from the expanded world and until we have
    // time available
    // Take care of clock() wrapping around
    for (int iTrans = 0; iTrans < MFWorldGetNbTrans(worldToExpand) &&
      timeUsed >= 0.0 &&
      timeUsed + maxTimeOneStep < MFGetMaxTimeExpansion(that); 
      ++iTrans) {
      // If this transition has not been computed
      const MFTransition* const trans = 
        MFWorldTransition(worldToExpand, iTrans);
      if (MFTransitionIsExpandable(trans)) {
        // Expand through this transition
        MFModelStatus status = 
          MFWorldComputeTransition(worldToExpand, iTrans);
        // If the resulting status has not already been computed
        MFWorld* sameWorld = MFSearchWorld(that, &status);
        if (sameWorld == NULL) {
          // Create a MFWorld for the new status
          MFWorld* expandedWorld = MFWorldCreate(&status);
          // Add the world to the set of computed world 
          MFAddWorld(that, expandedWorld, sente);
          // Add the world to the set of worlds to expand
          ++nbWorldToExpand;
          int sente = 
            MFModelStatusGetSente(MFWorldStatus(expandedWorld));
          float value = MFWorldGetPOVValue(expandedWorld, sente);
          GSetAddSort(&worldsToExpand, expandedWorld, value);
          // Set the expanded world as the result of the transition
          MFWorldSetTransitionToWorld(
            worldToExpand, iTrans, expandedWorld);
        } else {
          // Increment the number of reused world
          ++nbReusedWorld;
          // Set the already computed one as the result of the 
          // transition 
          MFWorldSetTransitionToWorld(worldToExpand, iTrans, sameWorld);
//printf("  World reused: ");
//MFWorldPrint(sameWorld, stdout);
//printf("\n");
        }
      }
      // Update the total time used from beginning of expansion 
      timeUsed = 
        ((double)(clock() - clockStart)) / MF_MILLISECTOCLOCKS;
    }
    // Update backward the forecast values for each transitions 
    // leading to the expanded world according to its new transitions
    MFUpdateForecastValues(that, worldToExpand, 0.0);
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
  // Memorize the remaining number of worlds to expand
  int nbRemainingWorldToExpand = GSetNbElem(&worldsToExpand);
  // Update the total time used from beginning of expansion 
  timeUsed = ((double)(clock() - clockStart)) / MF_MILLISECTOCLOCKS;
  // Update the percentage of time allocated to searching for worlds
  // to expand
  // If we could expand all the worlds
  if (nbRemainingWorldToExpand == 0) {
    if (timeUsed > PBMATH_EPSILON) {
      that->_timeSearchWorld *= 
        MFGetMaxTimeExpansion(that) / timeUsed;
      if (that->_timeSearchWorld > MFGetMaxTimeExpansion(that))
        that->_timeSearchWorld = MFGetMaxTimeExpansion(that);
    } else {
      that->_timeSearchWorld = MFGetMaxTimeExpansion(that);
    }
  // Else, we had not enough time to expand all the worlds
  } else {
    that->_timeSearchWorld *= 
      (float)nbRemainingWorldToExpand / (float)nbWorldToExpand;
  }
  // Empty the list of worlds to expand
  GSetFlush(&worldsToExpand);
  // Update the total time used from beginning of expansion 
  timeUsed = ((double)(clock() - clockStart)) / MF_MILLISECTOCLOCKS;
  // Take care of clock() wrapping around
  if (timeUsed < 0.0)
    timeUsed = MFGetMaxTimeExpansion(that);
  // Telemetry for debugging
  that->_timeUnusedExpansion = MFGetMaxTimeExpansion(that) - timeUsed;
  that->_nbWorldExpanded = 
    nbWorldToExpand - nbRemainingWorldToExpand + nbReusedWorld;
  if (that->_nbWorldExpanded > 0)
    that->_percWorldReused = 
      (float)nbReusedWorld / (float)(that->_nbWorldExpanded);
  else
    that->_percWorldReused = 0.0;
}

// Return the value of the MFWorld 'that' from the point of view of the 
// actor 'hero'.
// If 'hero' equals -1 it mens we are getting the value of a world
// during expansion and there is no preempting actor (everyone acting
// at the same time). Then we give priority during expansion to 
// worlds maximising simultaneously individual values.
float MFWorldGetPOVValue(const MFWorld* const that, const int hero) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (hero < -1 || hero >= MF_NBMAXACTOR) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'hero' is invalid (-1<=%d<%d)", \
      hero, MF_NBMAXACTOR);
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the returned value
  float value = 0.0;
  // Loop on actors
  for (int iActor = MF_NBMAXACTOR; iActor--;) {
    // If this actor is active
    if (MFModelStatusIsActorActive(MFWorldStatus(that), iActor)) {
      // Update the value
      if (iActor == hero || hero == -1)
        value += MFWorldGetValue(that, iActor);
      else
        value -= MFWorldGetValue(that, iActor);
    }
  }
  // Return the value
  return value;
}

// Return the value of the MFTransition 'that' from the point of view 
// of the actor 'hero'.
// If 'hero' equals -1 it mens we are getting the value of a world
// during expansion and there is no preempting actor (everyone acting
// at the same time). Then we give priority during expansion to 
// worlds maximising simultaneously individual values.
float MFTransitionGetPOVValue(const MFTransition* const that, 
  const int hero) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (hero < -1 || hero >= MF_NBMAXACTOR) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'hero' is invalid (-1<=%d<%d)", \
      hero, MF_NBMAXACTOR);
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the returned value
  float value = 0.0;
  // Loop on actors
  for (int iActor = MF_NBMAXACTOR; iActor--;) {
    // If this actor is active
    if (MFModelStatusIsActorActive(
      MFWorldStatus(MFTransitionToWorld(that)), iActor)) {
      // Update the value
      if (iActor == hero || hero == -1)
        value += MFTransitionGetValue(that, iActor);
      else
        value -= MFTransitionGetValue(that, iActor);
    }
  }
  // Return the value
  return value;
}

// Get the set of worlds to be expanded (having at least one transition
// whose _toWorld is null) for the MiniFrame 'that'
// Stop searching for world if clock() >= clockLimit
// Will return at least one world even if clockLimit == current clock
// The MiniFrame must have at least one world in its set of computed 
// worlds
GSet MFGetWorldsToExpand(const MiniFrame* const that, 
  const clock_t clockLimit) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (GSetNbElem(MFWorlds(that)) == 0) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "The MiniFrame has no computed world");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare the set to memorize worlds to expand
  GSet set = GSetCreateStatic();
  // Loop through the computed worlds
  GSetIterForward iter = GSetIterForwardCreateStatic(MFWorlds(that));
  do {
    MFWorld* world = GSetIterGet(&iter);
    // If this world has transition to expand
    if (MFWorldIsExpandable(world)) {
      // Add this world to the result set ordered by the value
      int sente = MFModelStatusGetSente(MFWorldStatus(world));
      float value = MFWorldGetPOVValue(world, sente);
      GSetAddSort(&set, world, value);
    }
  } while (GSetIterStep(&iter) && clock() < clockLimit);
  // Return the set of worlds to expand
  return set;
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
  // If the world is not at the end of the game/simutation
  if (!MFModelStatusIsEnd(MFWorldStatus(that))) {
    // Loop on transitions
    for (int iTrans = that->_nbTransition; iTrans-- && !isExpandable;) {
      // If this transition has not been computed
      if (MFTransitionIsExpandable(
        MFWorldTransition(that, iTrans)))
        isExpandable = true;
    }
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
      MFWorldGetValue(toWorld, iActor));
}

// Get the forecast value of the MFWorld 'that' for the actor 'iActor'
// It's the value of the MFWorldif it has no transitions, or the
// highest value of its transitions
float MFWorldGetForecastValue(const MFWorld* const that, int iActor) {
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
  // Declare a variable to memorize if there are transitions
  bool flagTransition = false;
  // Declare a variable to memorize the highest value among transitions
  float valBestTrans = 0.0;
  // Loop on transitions
  for (int iTrans = MFWorldGetNbTrans(that); iTrans--;) {
    // Declare a variable to memorize the transition
    const MFTransition* const trans = 
      MFWorldTransition(that, iTrans);
    // If this transitions has been expanded
    if (!MFTransitionIsExpandable(trans)) {
      // If it's not the first considered transition
      if (flagTransition) {
        // Get the value of the transition from the point of view of 
        // the requested actor
        float val = 
          MFTransitionGetPOVValue(trans, iActor);
        // If the value is better
        if (valBestTrans < val)
          valBestTrans = val;
      // Else it's the first considered transition
      } else {
        // Init the best value with the value of this transition
        valBestTrans = 
          MFTransitionGetPOVValue(trans, iActor);
        // Set the flag to memorize there are transitions
        flagTransition = true;
      }
    }
  }
  // Return the value for this world
  // If there are transitions
  if (flagTransition) {
    // Return the value of the best transition from the point of view 
    // of the requested actor
    return valBestTrans;
  // Else this world has no transitions
  } else {
    // Return the value of this world from the point of view of the 
    // requested actor
    return MFWorldGetPOVValue(that, iActor);
  }
}

// Get the best MFModelTransition for the 'iActor'-th actor in the 
// current MFWorld of the MiniFrame 'that'
// Return an undefined MFTransition if the curernt world has no 
// transition
const MFModelTransition* MFGetBestTransition(
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
  // Declare a variable to memorize the highest value among transitions
  float valBestTrans = 0.0;
  // Declare a variable to memorize the best transition
  const MFTransition* bestTrans = NULL;
  // Get the current world
  const MFWorld* const curWorld = MFCurWorld(that);
  // Loop on transitions
  for (int iTrans = MFWorldGetNbTrans(curWorld); iTrans--;) {
    // Declare a variable to memorize the transition
    const MFTransition* const trans = 
      MFWorldTransition(curWorld, iTrans);
    // If this transitions has been expanded
    if (!MFTransitionIsExpandable(trans)) {
      // If it's not the first considered transition
      if (bestTrans != NULL) {
        // Get the value of the transition from the point of view of 
        // the requested actor
        float val = MFTransitionGetPOVValue(trans, iActor);
        // If the value is better
        if (valBestTrans < val) {
          valBestTrans = val;
          bestTrans = trans;
        }
      // Else its the first considered transition
      } else {
        // Init the best value with the value of this transition
        valBestTrans = MFTransitionGetPOVValue(trans, iActor);
        // Init the index of the best transition
        bestTrans = trans;
      }
    }
  }
  // Return the best transition
  if (bestTrans != NULL)
    return (const MFModelTransition*)bestTrans;
  else {
    return NULL;
  }
}

// Update backward the forecast values for each transitions 
// leading to the MFWorld 'world' in the MiniFrame 'that'
// Use a penalty growing with each recursive call to 
// MFUpdateForecastValues to give priority to fastest convergence to 
// best solution and avoid infinite loop due to reuse of computed worlds
void MFUpdateForecastValues(MiniFrame* const that, 
  const MFWorld* const world, float delayPenalty) {
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
  // Increase the penalty 
  delayPenalty += PBMATH_EPSILON;
  // If the world has ancestors
  if (GSetNbElem(MFWorldSources(world)) > 0) {
    // Get the forecast values of the world for each actor
    float values[MF_NBMAXACTOR] = {0.0};
    for (int iAct = MF_NBMAXACTOR; iAct--;)
      values[iAct] = 
        MFWorldGetForecastValue(world, iAct) - delayPenalty;
    // For each source to the world
    GSetIterForward iter = 
      GSetIterForwardCreateStatic(MFWorldSources(world));
    do {
      // Get the transition
      MFTransition* const trans = GSetIterGet(&iter);
      // Update the transition's forecast value
      bool updated = MFTransitionUpdateValues(trans, values);
      // If the transition has been updated
      if (updated) {
        // Call recursively the MFUpdateForecastValues with the origin
        // of this transition
        MFUpdateForecastValues(that, MFTransitionFromWorld(trans),
          delayPenalty + PBMATH_EPSILON);
      }
    } while(GSetIterStep(&iter));
  }
}

// Update the values of the MFTransition 'that' for each actor with 
// the values 'values'
// Update only if the new value is higher than the current one
// Return true if at least one value has been updated, else false
bool MFTransitionUpdateValues(MFTransition* const that, 
  const float* const values) {
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
  // Declare a variable to memorize the returned flag
  bool updated = false;
  // For each actor
  for (int iAct = MF_NBMAXACTOR; iAct--;) {
    if (that->_values[iAct] < values[iAct]) {
      updated = true;
      that->_values[iAct] = values[iAct];
//printf("update ");MFTransitionPrint(that, stdout);printf("\n");
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
  printf("\n");
  for (int iTrans = 0; iTrans < MFWorldGetNbTrans(that); ++iTrans) {
    fprintf(stream, "  ");
    MFTransitionPrint(MFWorldTransition(that, iTrans), stream);
    fprintf(stream, "\n");
  }
}
  
// Set the current world of the MiniFrame 'that' to match the 
// MFModelStatus 'status'
// If the world is in computed worlds reuse it, else create a new one
// If we create a new one here and there are already has many computed
// worlds as the memory limit, free the current one to make room for
// the new one
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
  // Declare a flag to manage the deletion of element in the set of
  // computed worlds
  bool moved = false;
  // Loop on computed worlds
  GSetIterForward iter = GSetIterForwardCreateStatic(MFWorlds(that));
  do {
    MFWorld* world = GSetIterGet(&iter);
    moved = false;
    // If this is the current world
    if (MFModelStatusIsSame(MFWorldStatus(world), status)) {
      // Ensure that the status is exactly the same by copying the 
      // MFModelStatus struct, in case MFModelStatusIsSame refers only
      // to a subset of properties of the MFModelStatus
      memcpy(world, status, sizeof(MFModelStatus));
      // Update the curWorld in MiniFrame
      that->_curWorld = world;
      flagFound = true;
    // Else, if it's a disposable world
    } else if (MFModelStatusIsDisposable(MFWorldStatus(world), 
      MFWorldStatus(MFCurWorld(that)), MFGetNbComputedWorld(that))) {
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
      // Remove this world from the computed worlds
      moved = GSetIterRemoveElem(&iter);
      // Free memory
      MFWorldFree(&world);
    }
  } while (moved || GSetIterStep(&iter));
  // If we haven't found the searched status
  if (!flagFound) {
    // Create a new MFWorld with the current status
    MFWorld* world = MFWorldCreate(status);
    // Get the sente for the previous world
    int sente = MFModelStatusGetSente(MFWorldStatus(MFCurWorld(that)));
    // Add it to the computer worlds
    MFAddWorld(that, world, sente);
    // Update the current world
    that->_curWorld = world;
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
  // Loop on transitions
  if (GSetNbElem(MFWorldSources(that)) > 0) {
    bool moved = false;
    GSetIterForward iter = 
      GSetIterForwardCreateStatic(MFWorldSources(that));
    do {
      moved = false;
      MFTransition* trans = GSetIterGet(&iter);
      if (trans == source) {
        moved = GSetIterRemoveElem(&iter);
      }
    } while (moved || GSetIterStep(&iter));
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

