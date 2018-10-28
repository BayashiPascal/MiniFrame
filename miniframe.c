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

// Get the set of worlds to be expanded for the MiniFrame 'that'
// Prune worlds which have a value lower than a threhsold compare to 
// their best brother 
// Stop searching for world if clock() >= clockLimit
// Will return at least one world even if clockLimit == current clock
// The MiniFrame must have at least one world in its set of computed 
// worlds
// Force the current world to the end of the returned set to ensure
// it will be the first to be expanded
GSet MFGetWorldsToExpand(MiniFrame* const that, 
  const clock_t clockLimit);
void MFGetWorldsToExpandRec(MiniFrame* const that, 
  MFWorld* const world, GSet* set, const clock_t clockLimit, 
  int depth, GSet* setVisited);
  
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

// Update backward the forecast values for actor 'iActor' for each 
// transitions leading to the MFWorld 'world' in the MiniFrame 'that'
// Use a penalty growing with each recursive call to 
// MFUpdateForecastValues to give priority to fastest convergence to 
// best solution
// Avoid infinite loop due to reuse of computed worlds
void MFUpdateForecastValues(MiniFrame* const that, 
  const MFWorld* const world, int delayPenalty, GSet* const setWorld,
  int iActor);

// Update the values of the MFTransition 'that' for actor 'iActor' with 
// 'val'
// Return true if the value has been updated, else false
bool MFTransitionUpdateValue(MFTransition* const that, const int iActor,
  const float val);

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

// Free the memory used by the disposable worlds in the computed worlds
// of the MinFrame 'that'
void MFFreeDisposableWorld(MiniFrame* const that);

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
  MFAddWorld(that, MFCurWorld(that), MFModelStatusGetSente(initStatus));
  that->_timeSearchWorld = MF_DEFAULTTIMEEXPANSION;
  that->_nbWorldExpanded = 0;
  that->_nbWorldUnexpanded = 0;
  that->_nbRemovedWorld = 0;
  that->_timeUnusedExpansion = 0.0;
  that->_reuseWorld = false;
  that->_percWorldReused = 0.0;
  that->_startExpandClock = 0;
  that->_maxDepthExp = -1;
  that->_expansionType = MFExpansionTypeValue;
  that->_nbTransMonteCarlo = MF_NBTRANSMONTECARLO;
  that->_pruningDeltaVal = MF_PRUNINGDELTAVAL;
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
  float values[MF_NBMAXACTOR] = {0.0};
  MFModelStatusGetValues(status, values);
  MFWorldSetValues(that, values);
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
  // Free the disposable worlds
  MFFreeDisposableWorld(that);
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
  // Declare a variable to memorie the number of worlds to expand added
  // to the original set
  int nbWorldToExpandPost = 0;
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
    // Get the number of expandable transition
    int nbTransExpandable = MFWorldGetNbTransExpandable(worldToExpand);
    // Get the threhsold for expannsion to activate montecarlo when 
    // there are too many transitions
    float thresholdMonteCarlo = 
      (float)MFGetNbTransMonteCarlo(that) / (float)nbTransExpandable;
    // For each transitions from the expanded world and until we have
    // time available
    // Take care of clock() wrapping around
    for (int iTrans = 0; iTrans < MFWorldGetNbTrans(worldToExpand) &&
      timeUsed >= 0.0 &&
      timeUsed + maxTimeOneStep + 
      MFGetTimeEndExpansion(that) * GSetNbElem(&worldsToExpand) < 
      MFGetMaxTimeExpansion(that); 
      ++iTrans) {
      // If this transition has not been computed
      const MFTransition* const trans = 
        MFWorldTransition(worldToExpand, iTrans);
      if (MFTransitionIsExpandable(trans) && 
        rnd() < thresholdMonteCarlo) {
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
          // Set the expanded world as the result of the transition
          MFWorldSetTransitionToWorld(
            worldToExpand, iTrans, expandedWorld);
          // If it's not an end status world and we haven't reached
          // the expansion limit
          if ((that->_maxDepthExp < 0 || 
            worldToExpand->_depth < that->_maxDepthExp) &&
            !MFModelStatusIsEnd(MFWorldStatus(expandedWorld))) {
            // Add the world to the set of worlds to expand
            ++nbWorldToExpand;
            expandedWorld->_depth = worldToExpand->_depth + 1;
            if (MFGetExpansionType(that) == MFExpansionTypeValue) {
              float value = MFWorldGetValue(expandedWorld, sente);
              GSetAddSort(&worldsToExpand, expandedWorld, value);
            } else if (MFGetExpansionType(that) == MFExpansionTypeWidth) {
              GSetPush(&worldsToExpand, expandedWorld);
            }
            ++nbWorldToExpandPost;
          }
        } else {
          // Increment the number of reused world
          ++nbReusedWorld;
          // Set the already computed one as the result of the 
          // transition 
          MFWorldSetTransitionToWorld(worldToExpand, iTrans, sameWorld);
        }
      }
      // Update the total time used from beginning of expansion 
      timeUsed = 
        ((double)(clock() - clockStart)) / MF_MILLISECTOCLOCKS;
    }
    // For each actor
    for (int iActor = 
      MFModelStatusGetNbActor(MFWorldStatus(worldToExpand)); 
      iActor--;) {
      // Update backward the forecast values for each transitions 
      // leading to the expanded world according to its new transitions
      GSet setWorld = GSetCreateStatic();
      MFUpdateForecastValues(that, worldToExpand, 0, &setWorld, iActor);
      GSetFlush(&setWorld);
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
  // Memorize the remaining number of worlds to expand
  int nbRemainingWorldToExpand = 
    MAX(0, GSetNbElem(&worldsToExpand) - nbWorldToExpandPost);
  // Update the total time used from beginning of expansion 
  timeUsed = ((double)(clock() - clockStart)) / MF_MILLISECTOCLOCKS;
  // Update the percentage of time allocated to searching for worlds
  // to expand
  // If there was worlds to expand
  if (nbWorldToExpand > 0) {
    // If we could expand all the worlds
    if (nbRemainingWorldToExpand == 0) {
      that->_timeSearchWorld *= 
        MFGetMaxTimeExpansion(that) / timeUsed;
      if (that->_timeSearchWorld > MFGetMaxTimeExpansion(that))
        that->_timeSearchWorld = MFGetMaxTimeExpansion(that);
    // Else, we had not enough time to expand all the worlds
    } else {
      that->_timeSearchWorld *= 
        (float)nbRemainingWorldToExpand / (float)nbWorldToExpand;
    }
  } else {
    that->_timeSearchWorld = 
      MAX(0, MFGetMaxTimeExpansion(that) - timeUsed);
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
  that->_nbWorldUnexpanded = nbRemainingWorldToExpand;
  if (that->_nbWorldExpanded > 0)
    that->_percWorldReused = 
      (float)nbReusedWorld / (float)(that->_nbWorldExpanded);
  else
    that->_percWorldReused = 0.0;
}

// Get the set of worlds to be expanded (having at least one transition
// whose _toWorld is null) for the MiniFrame 'that'
// Stop searching for world if clock() >= clockLimit
// Will return at least one world even if clockLimit == current clock
// The MiniFrame must have at least one world in its set of computed 
// worlds
// Force the current world to the end of the returned set to ensure
// it will be the first to be expanded
GSet MFGetWorldsToExpandOld(MiniFrame* const that, 
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
  // Free the disposabe worlds
  MFFreeDisposableWorld(that);
  // Declare the set to memorize worlds to expand
  GSet set = GSetCreateStatic();
  // Loop through the computed worlds
  GSetIterForward iter = GSetIterForwardCreateStatic(MFWorlds(that));
  do {
    MFWorld* world = GSetIterGet(&iter);
    // If this world has transition to expand
    if (world != MFCurWorld(that) && MFWorldIsExpandable(world)) {
      // Add this world to the result set ordered by the value
      world->_depth = 0;
      if (MFGetExpansionType(that) == MFExpansionTypeValue) {
        int sente = MFModelStatusGetSente(MFWorldStatus(world));
        float value = MFWorldGetForecastValue(world, sente);
        GSetAddSort(&set, world, value);
      } else if (MFGetExpansionType(that) == MFExpansionTypeWidth) {
        GSetPush(&set, world);
      }
    }
  } while (GSetIterStep(&iter) && clock() < clockLimit);
  // Add the current world
  if (MFWorldIsExpandable(MFCurWorld(that))) {
    that->_curWorld->_depth = 0;
    GSetAppend(&set, MFCurWorld(that));
  }
  // Return the set of worlds to expand
  return set;
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
  if (MFTransitionToWorld(trans) != world) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, 
      "The transition doesn't reach the world");
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
  int sente = MFModelStatusGetSente(MFWorldStatus(fatherWorld));
  // Declare a variable to memorize the maximum value of brothers world
  float max = 0.0;
  const MFWorld* bestBrother = NULL;
  // Loop on transitions from the father world
  for (int iTrans = MFWorldGetNbTrans(fatherWorld); iTrans--;) {
    // Get the origin of the current transition
    const MFWorld* const brother = 
      MFTransitionToWorld(MFWorldTransition(fatherWorld, iTrans));
    if (brother != that && brother != NULL) {
      // Get the value of the brother
      const float val = MFWorldGetForecastValue(brother, sente);
      // Update the maximum if necessary
      if (bestBrother == NULL || max < val) {
        bestBrother = brother;
        max = val;
      }
    }
  }
  // If there is at least one brother
  if (bestBrother != NULL) {
    // Get the value of the world in argument
    float val = MFWorldGetForecastValue(that, sente);
    // If the pruning constraint is true
    if (val < max - MFGetPruningDeltaVal(mf))
      // Update the result
      ret = true;
  }
  // Return the result
  return ret;
}
  
// Get the set of worlds to be expanded for the MiniFrame 'that'
// Prune worlds which have a value lower than a threhsold compare to 
// their best brother 
// Stop searching for world if clock() >= clockLimit
// Will return at least one world even if clockLimit == current clock
// The MiniFrame must have at least one world in its set of computed 
// worlds
// Force the current world to the end of the returned set to ensure
// it will be the first to be expanded
GSet MFGetWorldsToExpand(MiniFrame* const that, 
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
  // Create the result set
  GSet set = GSetCreateStatic();
  // Create the visited set
  GSet setVisited = GSetCreateStatic();
  // Get the current world
  MFWorld* const world = (MFWorld*)MFCurWorld(that);
  // Start the recursion
  MFGetWorldsToExpandRec(that, world, &set, clockLimit, 0, &setVisited);
  // Add the current world of the MiniFrame at the end of the set
  if (MFWorldIsExpandable(world)) {
    world->_depth = 0;
    GSetAppend(&set, world);
  }
  // Free memory
  GSetFlush(&setVisited);
  // Return the set
  return set;
}
void MFGetWorldsToExpandRec(MiniFrame* const that, 
  MFWorld* const world, GSet* set, const clock_t clockLimit, 
  int depth, GSet* setVisited) {
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
  // Avoid inifinite loop when reusing world
  if (MFIsWorldReusable(that) && 
    GSetFirstElem(setVisited, world) != NULL)
    return;
  else
    GSetAppend(setVisited, world);
  // If we are not at the root and the current world is expandable
  if (depth != 0 && MFWorldIsExpandable(world)) {
    // Add the world to the result set ordered by depth or value
    world->_depth = 0;
    if (MFGetExpansionType(that) == MFExpansionTypeValue) {
      int sente = MFModelStatusGetSente(MFWorldStatus(world));
      float value = MFWorldGetForecastValue(world, sente);
      GSetAddSort(set, world, value);
    } else if (MFGetExpansionType(that) == MFExpansionTypeWidth) {
      float value = -1.0 * (float)depth;
      GSetAddSort(set, world, value);
    }
  }
  // Loop on the transitions of the current world
  for (int iTrans = MFWorldGetNbTrans(world); 
    iTrans-- && clock() < clockLimit;) {
    // Get the transition
    const MFTransition* const trans = MFWorldTransition(world, iTrans);
    // Get the world reached through this transition
    const MFWorld* const toWorld = MFTransitionToWorld(trans);
    // If this transition is expanded
    if (toWorld != NULL) {
      // If the pruning condition is false for the world reached 
      // through this transition
      if (!MFWorldIsPrunedDuringExpansion(toWorld, that, trans)) {
        // Continue searching for worlds to expand from the world 
        // reached through this transition
        MFGetWorldsToExpandRec(that, (MFWorld*)toWorld, set, 
          clockLimit, depth + 1, setVisited);
      }
    }
  }
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
      if (MFTransitionIsExpandable(MFWorldTransition(that, iTrans)))
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
  // Should never reach here, but just in case...
  return true;
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
  // Declare a variable to memorize the highest value among transitions
  float valBestTrans = 0.0;
  // Declare a variable to memorize the transition with highest value
  const MFTransition* bestTrans = NULL;
  // Loop on transitions
  for (int iTrans = MFWorldGetNbTrans(that); iTrans--;) {
    // Declare a variable to memorize the transition
    const MFTransition* const trans = 
      MFWorldTransition(that, iTrans);
    // If this transitions has been expanded
    if (!MFTransitionIsExpandable(trans)) {
      // Get the value of the transition from the point of view of 
      // the sente
      float val = MFTransitionGetValue(trans, iActor);
      // If it's not the first considered transition
      if (bestTrans != NULL) {
        // If the value is better
        if (valBestTrans < val) {
          valBestTrans = val;
          bestTrans = trans;
        }
      // Else it's the first considered transition
      } else {
        // Init the best value with the value of this transition
        valBestTrans = val;
        // Init the best transition
        bestTrans = trans;
      }
    }
  }
  // Return the value for this world
  // If there are expanded transitions
  if (bestTrans != NULL) {
    // Return the value of the best transition from the point of view 
    // of the requested actor
    return MFTransitionGetValue(bestTrans, iActor);
  // Else this world has no transitions
  } else {
    // Return the value of this world from the point of view of the 
    // requested actor
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
  // Declare a variable to memorize the best transition
  const MFTransition* bestTrans = NULL;
  // Loop on transitions
  for (int iTrans = MFWorldGetNbTrans(that); iTrans--;) {
    // Declare a variable to memorize the transition
    const MFTransition* const trans = MFWorldTransition(that, iTrans);
    // If this transitions has been expanded
    if (MFTransitionIsExpanded(trans)) {
      // Get the value of the transition from the point of view of 
      // the requested actor
      float val = MFTransitionGetValue(trans, iActor);
      // Add some random perturbation to avoid always picking
      // the same transitions between those with equal values
      val += rnd() * PBMATH_EPSILON;
      // If it's not the first considered transition
      if (bestTrans != NULL) {
        // If the value is better
        if (valBestTrans < val) {
          // Update the best value and best transition
          valBestTrans = val;
          bestTrans = trans;
        }
      // Else it's the first considered transition
      } else {
        // Init the best value with the value of this transition
        valBestTrans = val;
        // Init the best transition
        bestTrans = trans;
      }
    }
  }
  // If the bestTrans is null here it means that none of the transitions
  // for the current world were expanded yet
  // By default choose a random one 
  if (bestTrans == NULL && MFWorldGetNbTrans(that) > 0) {
    bestTrans = MFWorldTransition(that, 
      (int)floor(MIN(rnd(), 0.9999) * (float)MFWorldGetNbTrans(that)));
  }
  // Return the best transition
  return (const MFModelTransition*)bestTrans;
}

// Update backward the forecast values for actor 'iActor' for each 
// transitions leading to the MFWorld 'world' in the MiniFrame 'that'
// Use a penalty growing with each recursive call to 
// MFUpdateForecastValues to give priority to fastest convergence to 
// best solution
// Avoid infinite loop due to reuse of computed worlds
void MFUpdateForecastValues(MiniFrame* const that, 
  const MFWorld* const world, int delayPenalty, GSet* const setWorld,
  int iActor) {
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
  // Avoid infinite loop
  if (GSetFirstElem(setWorld, world) == NULL)
    GSetAppend(setWorld, (void*)world);
  else
    return;
  // If the world has ancestors
  if (GSetNbElem(MFWorldSources(world)) > 0) {
    // Get the forecast value of the world
    float forecastVal = MFWorldGetForecastValue(world, iActor);
    // Declare a variable to memorize when the transition is updated
    bool updated = false;
    // For each transition to the world
    GSetIterForward iter = 
      GSetIterForwardCreateStatic(MFWorldSources(world));
    do {
      // Get the transition
      MFTransition* const trans = GSetIterGet(&iter);
      // If we are at the first level of recursion
      if (delayPenalty == 0) {
        // Initialize the value of the transition
        MFTransitionSetValue(trans, iActor, forecastVal);
        updated = true;
      } else {
        // Update the value of the transition
        updated = MFTransitionUpdateValue(trans, iActor, 
          forecastVal - (float)delayPenalty * PBMATH_EPSILON);
      }
      // If the value has been updated
      if (updated) {
        // Propagate the update from the source world
        MFUpdateForecastValues(that, MFTransitionFromWorld(trans),
          delayPenalty + 1, setWorld, iActor);
      }
    } while (GSetIterStep(&iter));
  }
}

// Update the values of the MFTransition 'that' for actor 'iActor' with 
// 'val'
// Return true if the value has been updated, else false
bool MFTransitionUpdateValue(MFTransition* const that, const int iActor,
  const float val) {
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
  // Declare a variable to memorize the returned flag
  bool updated = false;
#if MF_NBMAXACTOR == 1
  if (that->_values[iActor] < val) {
    updated = true;
    that->_values[iActor] = val;
  }
#else
#if MF_SIMULTANEOUS_PLAY

#else
  MFWorld* fromWorld = MFTransitionFromWorld(that);
  int sente = MFModelStatusGetSente(MFWorldStatus(fromWorld));
  if (sente == -1 || sente == iActor) {
    if (that->_values[iActor] < val) {
      updated = true;
      that->_values[iActor] = val;
    }
  } else {
    if (that->_values[iActor] > val) {
      updated = true;
      that->_values[iActor] = val;
    }
  }
#endif
#endif
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
  // Loop on computed worlds
  GSetIterForward iter = GSetIterForwardCreateStatic(MFWorlds(that));
  do {
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
  } while (!flagFound && GSetIterStep(&iter));
  // If we haven't found the searched status
  if (!flagFound) {
    // Create a new MFWorld with the current status
    MFWorld* world = MFWorldCreate(status);
    // Get the sente for the previous world
    int sente = MFModelStatusGetSente(MFWorldStatus(MFCurWorld(that)));
    // Add it to the computed worlds
    MFAddWorld(that, world, sente);
    // Update the current world
    that->_curWorld = world;
  }
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
  // Declare a flag to memorize if we have found a disposable world
  bool flag = false;
  // Declare a flag to manage the deletion of element in the set of
  // computed worlds
  bool moved = false;
  //Declare a variable to memorize the number of removed world
  int nbRemovedWorld = 0;
  // Loop until we haven't found any disposable world
  do {
    // Reset the flag to memorize if we have found disposable world
    flag = false;
    // Loop on computed worlds
    GSetIterForward iter = GSetIterForwardCreateStatic(MFWorlds(that));
    do {
      MFWorld* world = GSetIterGet(&iter);
      moved = false;
      // If it's a disposable world
      if (that->_curWorld != world && 
        (GSetNbElem(MFWorldSources(world)) == 0 || 
        MFModelStatusIsDisposable(MFWorldStatus(world), 
        MFWorldStatus(MFCurWorld(that)), MFGetNbComputedWorld(that)))) {
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
        // Increment the number of removed world
        ++nbRemovedWorld;
        // Memorize we have found a disposable world
        flag = true;
      }
    } while (moved || GSetIterStep(&iter));
  } while (flag == true);
  // Update the number of removed world
  that->_nbRemovedWorld = nbRemovedWorld;
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

// Print the best forecasted story from the MFWorld 'that' for the 
// actor 'iActor' on the stream 'stream'
void MFWorldPrintBestStoryln(const MFWorld* const that, const int iActor, 
  FILE* const stream) {
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
  for (int iActor = MF_NBMAXACTOR; iActor--;) {
    that->_values[iActor] = 0.0;
    for (int jActor = MF_NBMAXACTOR; jActor--;) {
      if (iActor == jActor)
        that->_values[iActor] += values[jActor];
      else
        that->_values[iActor] -= values[jActor];
    }
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



