// ============ MINIFRAME_MODEL.C ================

// As an example the code below implements a world where one actor
// moves along a discrete axis by step of one unit to reach a fixed 
// target position
// Status of the world is defined by the current actor position and 
// the target position
// Available actions are -1, 0, +1 (next position = current position 
// + action) if the actor hasn't reached the target, else no actions 
// The position of the actor is bounded to -5, 5
// The value of the world is given by -abs(position-target)

// ================= Include =================

#include "miniframe-model.h"

// ================ Functions implementation ====================

// Copy the properties of the MFModelStatus 'that' into the 
// MFModelStatus 'tho'
// Dynamically allocated properties must be cloned
void MFModelStatusCopy(const MFModelStatus* const that,
  MFModelStatus* const tho) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (tho == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'tho' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  (void)memcpy(tho, that, sizeof(MFModelStatus));
}

// Free memory used by the properties of the MFModelStatus 'that'
// The memory used by the MFModelStatus itself is managed by MiniFrame
void MFModelStatusFreeStatic(MFModelStatus* that) {
  (void)that;
  
}

// Free memory used by the properties of the MFModelTransition 'that'
// The memory used by the MFModelTransition itself is managed by 
// MiniFrame
void MFModelTransitionFreeStatic(MFModelTransition* that) {
  (void)that;
  
}

// Return true if 'that' and 'tho' are to be considered as the same
// by MiniFrame when trying to reuse previously computed status, 
// else false
bool MFModelStatusIsSame(const MFModelStatus* const that,
  const MFModelStatus* const tho) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (tho == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'tho' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  if (that->_pos == tho->_pos &&
    that->_tgt == tho->_tgt)
    return true;
  else
    return false;
}

// Return the index of the actor who has preemption in the MFModelStatus
// 'that'
// If no actor has preemption (all the actor act simultaneously)
// return -1
int MFModelStatusGetSente(const MFModelStatus* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  (void)that;
  return 0;
}

// Return true if the actor 'iActor' is active given the MFModelStatus 
// 'that'
bool MFModelStatusIsActorActive(const MFModelStatus* const that, const int iActor) {
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
  (void)that;(void)iActor;

  return true;
}

// Get the possible transitions from the MFModelStatus 'that' and 
// memorize them in the array of MFModelTransition 'transitions', and 
// memorize the number of transitions in 'nbTrans'
// 'transitions' as MF_NBMAXTRANSITION size, got MFModelTransition are 
// expected in transitions[0~(nbTrans-1)]
void MFModelStatusGetTrans(const MFModelStatus* const that, 
  MFModelTransition* const transitions, int* const nbTrans) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (transitions == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'transitions' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (nbTrans == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'nbTrans' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  if (that->_pos == that->_tgt) {
    *nbTrans = 0;
  } else {
    *nbTrans = 3;
    transitions[0]._move = -1;
    transitions[1]._move = 0;
    transitions[2]._move = 1;
  }
}

// Get the values of the MFModelStatus 'that' from the point of view 
// of each actor and  memorize them in the array of float 'values'
// 'values' as MF_NBMAXACTOR size, all values are set to 0.0 before 
// calling this function
void MFModelStatusGetValues(const MFModelStatus* const that, 
  float* const values) {
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
  values[0] = -1.0 * fabs(that->_tgt - that->_pos);
}

// Return the MFModelStatus resulting from applying the 
// MFModelTransition 'trans' to the MFModelStatus 'that' 
MFModelStatus MFModelStatusStep(const MFModelStatus* const that, 
  const MFModelTransition* const trans) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (trans == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'trans' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  // Declare a variable to memorize the resulting status
  MFModelStatus status;
  // Apply the transition
  status._step = that->_step + 1;
  status._tgt = that->_tgt;
  status._pos = that->_pos + trans->_move;
  int limit = 3;
  if (status._pos < -limit) status._pos = -limit;
  if (status._pos > limit) status._pos = limit;
  // Return the status
  return status;
}

// Print the MFModelStatus 'that' on the stream 'stream' 
void MFModelStatusPrint(const MFModelStatus* const that, 
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
  fprintf(stream, "step:%d pos:%d tgt:%d", that->_step, 
    that->_pos, that->_tgt);
}

// Print the MFModelTransition 'that' on the stream 'stream' 
void MFModelTransitionPrint(const MFModelTransition* const that, 
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
  fprintf(stream, "move:%d", that->_move);
}

// Return true if the MFStatus 'that' is disposable (its memory can be 
// freed) given the current status 'curStatus' and the number of 
// world instances in memory, else false
// As many as possible should be kept in memory, especially if worlds
// are reusable, but its up to the user to decide which and when whould
// be discarded to fit the physical memory available
// Having too many world instances in memory also slow down the 
// exploration of worlds during expansion
bool MFModelStatusIsDisposable(const MFModelStatus* const that, 
  const MFModelStatus* const curStatus, const int nbStatus) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
  if (curStatus == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'curStatus' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  if (nbStatus > 0) {
    if (abs(that->_pos - curStatus->_pos) > 2)
      return true;
    else
      return false;
  } else {
    return false;
  }
}

// Return true if the MFModelStatus 'that' is the end of the 
// game/simulation, else false
bool MFModelStatusIsEnd(const MFModelStatus* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  if (that->_step >= 6 || that->_pos == that->_tgt) {
    return true;
  } else {
    return false;
  }
}
