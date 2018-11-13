// ============ MINIFRAME_INLINE.C ================

// ================ Functions implementation ====================

// Get the time limit for expansion of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
float MFGetMaxTimeExpansion(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_maxTimeExpansion;
}

// Get the time unused during last expansion of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
float MFGetTimeUnusedExpansion(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_timeUnusedExpansion;
}

// Get the nb of world To expande of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
int MFGetNbWorldsToExpand(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return GSetNbElem(MFWorldsToExpand(that));
}

// Get the clock considered has start during expansion
#if BUILDMODE != 0
inline
#endif
clock_t MFGetStartExpandClock(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_startExpandClock;
}

// Set the clock considered has start during expansion to 'c'
#if BUILDMODE != 0
inline
#endif
void MFSetStartExpandClock(MiniFrame* const that, clock_t c) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  that->_startExpandClock = c;
}

// Set the time limit for expansion of the MiniFrame 'that' to 
// 'timeLimit', in millisecond
// The time is measured with the function clock(), see "man clock"
// for details
#if BUILDMODE != 0
inline
#endif
void MFSetMaxTimeExpansion(MiniFrame* const that, const float timeLimit) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  that->_maxTimeExpansion = timeLimit;
}

// Get the current MFWorld of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
const MFWorld* MFCurWorld(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_curWorld;
}

// Get the GSet of computed MFWorlds of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
const GSet* MFWorlds(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return &(that->_worlds);
}

// Get the GSet of worlds to expand of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
const GSet* MFWorldsToExpand(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return &(that->_worldsToExpand);
}

// Get the GSet of worlds on hold of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
const GSet* MFWorldsOnHold(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return &(that->_worldsOnHold);
}

// Get the GSet of worlds to free of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
const GSet* MFWorldsToFree(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return &(that->_worldsToFree);
}

// Add the MFWorld 'world' to the computed MFWorlds of the 
// MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
void MFAddWorld(MiniFrame* const that, const MFWorld* const world) {
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
  GSetAppend(&(that->_worlds), world);  
}

// Add the MFWorld 'world' to the world to be expanded of the 
// MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
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

// Add the MFWorld 'world' to the worlds on hold of the 
// MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
void MFAddWorldToOnHold(MiniFrame* const that, \
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
  GSetPush(&(that->_worldsOnHold), world);  
}

// Return the MFModelStatus of the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
const MFModelStatus* MFWorldStatus(const MFWorld* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return (const MFModelStatus*)that;  
}

// Get the number of transition for the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
int MFWorldGetNbTrans(const MFWorld* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_nbTransition;
}

// Get the percentage of resued world of the MiniFrame 'that' during 
// the last MFEpxand()
#if BUILDMODE != 0
inline
#endif
float MFGetPercWordReused(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_percWorldReused;
}

// Get the 'iTrans' MFTransition of the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
const MFTransition* MFWorldTransition(const MFWorld* const that, 
  const int iTrans) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (iTrans <0 || iTrans >= that->_nbTransition) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'iTrans' is invalid (0<=%d<%d)",
      iTrans, that->_nbTransition);
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_transitions + iTrans;
}

// Compute the MFModelStatus resulting from the 'iTrans' MFTransition 
// of the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
MFModelStatus MFWorldComputeTransition(const MFWorld* const that, 
  const int iTrans) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (iTrans <0 || iTrans >= that->_nbTransition) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'iTrans' is invalid (0<=%d<%d)",
      iTrans, that->_nbTransition);
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Return the resulting MFModelStatus
  return MFModelStatusStep((const MFModelStatus* const)that, 
    (const MFModelTransition* const)MFWorldTransition(that, iTrans));
}

// Return true if the expansion algorithm looks in previously 
// computed worlds for same world to reuse, else false
#if BUILDMODE != 0
inline
#endif
bool MFIsWorldReusable(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_reuseWorld;
}

// Set the flag controling if the expansion algorithm looks in 
// previously computed worlds for same world to reuse to 'reuse'
#if BUILDMODE != 0
inline
#endif
void MFSetWorldReusable(MiniFrame* const that, const bool reuse) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  that->_reuseWorld = reuse;
}

// Get the MFWorld which the MFTransition 'that' is leading to
#if BUILDMODE != 0
inline
#endif
const MFWorld* MFTransitionToWorld(const MFTransition* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_toWorld;
}

// Set the MFWorld to which the MFTransition 'that' is leading to 
// 'world'
#if BUILDMODE != 0
inline
#endif
void MFTransitionSetToWorld(MFTransition* const that, 
  MFWorld* const world) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  that->_toWorld = world;
}

// Get the MFWorld which the MFTransition 'that' is coming from
#if BUILDMODE != 0
inline
#endif
const MFWorld* MFTransitionFromWorld(const MFTransition* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_fromWorld;
}

// Set the value of the MFTransition 'that' for the actor 'iActor' to 
// 'val'
#if BUILDMODE != 0
inline
#endif
void MFTransitionSetValue(MFTransition* const that, const int iActor,
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
  that->_values[iActor] = val;
}

// Return the number of computed worlds in the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
int MFGetNbComputedWorlds(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return GSetNbElem(&(that->_worlds));
}

// Get the nb of worlds to remove of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
int MFGetNbWorldsToFree(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return GSetNbElem(MFWorldsToFree(that));
}

// Get the nb of worlds on hold of the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
int MFGetNbWorldsOnHold(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return GSetNbElem(MFWorldsOnHold(that));
}


// Return the value of the MFWorld 'that' for the 
// actor 'iActor'.
#if BUILDMODE != 0
inline
#endif
float MFWorldGetValue(const MFWorld* const that, const int iActor) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (iActor < 0 || iActor >= MF_NBMAXACTOR) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'iActor' is invalid (0<=%d<%d)", \
      iActor, MF_NBMAXACTOR);
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_values[iActor];
}

// Return the value of the MFTransition 'that' for the 
// actor 'iActor'.
#if BUILDMODE != 0
inline
#endif
float MFTransitionGetValue(const MFTransition* const that, 
  const int iActor) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (iActor < 0 || iActor >= MF_NBMAXACTOR) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'iActor' is invalid (0<=%d<%d)", \
      iActor, MF_NBMAXACTOR);
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_values[iActor];
}



// Get the set of MFTransition reaching the MFWorld 'that'
#if BUILDMODE != 0
inline
#endif
const GSet* MFWorldSources(const MFWorld* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return &(that->_sources);
}

// Return the array of values of the MFWorld 'that' for each actor
#if BUILDMODE != 0
inline
#endif
const float* MFWorldValues(const MFWorld* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_values;
}

// Return the max depth during expansion for the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
int MFGetMaxDepthExp(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_maxDepthExp;
}

// Set the max depth during expansion for the MiniFrame 'that' to 'depth'
// If depth is less than -1 it is converted to -1
// If the expansion type is not by width the max expansion depth is 
// ignored during expansion
#if BUILDMODE != 0
inline
#endif
void MFSetMaxDepthExp(MiniFrame* const that, const int depth) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  that->_maxDepthExp = MAX(-1, depth);
}

// Return the type of expansion for the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
MFExpansionType MFGetExpansionType(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_expansionType;
}

// Set the type expansion for the MiniFrame 'that' to 'type'
#if BUILDMODE != 0
inline
#endif
void MFSetExpansionType(MiniFrame* const that, const MFExpansionType type) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  that->_expansionType = type;
}

// Set the nb of transitions to activate MonteCarlo during expansion
// for the MiniFrame 'that' to 'nb'
#if BUILDMODE != 0
inline
#endif
void MFSetNbTransMonteCarlo(MiniFrame* const that, const int nb) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (nb <= 0) {
    MiniFrameErr->_type = PBErrTypeInvalidArg;
    sprintf(MiniFrameErr->_msg, "'nb' is invalid (%d>0)", nb);
    PBErrCatch(MiniFrameErr);
  }
#endif
  that->_nbTransMonteCarlo = nb;
}

// Get the nb of transitions to activate MonteCarlo during expansion
// for the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
int MFGetNbTransMonteCarlo(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_nbTransMonteCarlo;
}

// Return true if the MFTransition is expanded, false else
#if BUILDMODE != 0
inline
#endif
bool MFTransitionIsExpanded(const MFTransition* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return (that->_toWorld != NULL);
}

// Set the pruning threshold during expansion for the MiniFrame 'that' 
// to 'val'
#if BUILDMODE != 0
inline
#endif
void MFSetPruningDeltaVal(MiniFrame* const that, const float val) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  that->_pruningDeltaVal = val;
}

// Get the pruning threshold during expansion for the MiniFrame 'that'
#if BUILDMODE != 0
inline
#endif
float MFGetPruningDeltaVal(const MiniFrame* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  return that->_pruningDeltaVal;
}


