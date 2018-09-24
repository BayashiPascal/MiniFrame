// ============ MINIFRAME_MODEL.C ================

// ================= Include =================

#include "miniframe-model.h"
#if BUILDMODE == 0
#include "miniframe-inline-model.c"
#endif

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
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
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
  bool ret = true;
  if (that->_curPlayer != tho->_curPlayer ||
    that->_end != tho->_end)
    ret = false;
  for (int iPlayer = NBPLAYER; iPlayer-- && ret;)
    if (that->_score[iPlayer] != tho->_score[iPlayer])
      ret = false;
  for (int iHole = NBHOLE; iHole-- && ret;)
    if (that->_nbStone[iHole] != tho->_nbStone[iHole])
      ret = false;
  return ret;
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
  return that->_curPlayer;
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
  // Incorrect if NBPLAYER > 2
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
  *nbTrans = 0;
  for (int iHole = that->_curPlayer * NBHOLEPLAYER; 
    iHole < (that->_curPlayer + 1) * NBHOLEPLAYER; 
    ++iHole) {
    if (that->_nbStone[iHole] > 0) {
      transitions[*nbTrans]._iHole = iHole;
      ++(*nbTrans);
    }  
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
  VecFloat* input = VecFloatCreate(MF_MODEL_NN_NBINPUT);
  VecFloat* output = VecFloatCreate(MF_MODEL_NN_NBOUTPUT);
  for (int iPlayer = NBPLAYER; iPlayer--;) {
    if (that->_nn[iPlayer] == NULL) {
      values[iPlayer] = that->_score[iPlayer];
    } else {
      for (int iHole = NBHOLE; iHole--;) {
        int jHole = iHole + iPlayer * NBHOLEPLAYER;
        if (jHole >= NBHOLE)
          jHole -= NBHOLE;
        VecSet(input, iHole, that->_nbStone[jHole]);
      }
      NNEval(that->_nn[iPlayer], input, output);
      float valMax = VecGetMaxVal(output);
      values[iPlayer] = MAX(valMax, that->_score[iPlayer]);
    }
    if (values[iPlayer] * 2 > NBSTONE)
      values[iPlayer] = NBSTONE;
  }
  VecFree(&input);
  VecFree(&output);
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

  MFModelStatusCopy(that, &status);
  int nbStone = status._nbStone[trans->_iHole];
  // Remove stones from starting hole
  status._nbStone[trans->_iHole] = 0;
  // Distribute stones 
  int jHole = trans->_iHole;
  while (nbStone > 0) {
    ++jHole;
    if (jHole == NBHOLE) jHole = 0;
    // Jump over starting hole
    if (jHole == trans->_iHole) ++jHole;
    if (jHole == NBHOLE) jHole = 0;
    ++(status._nbStone[jHole]);
    --nbStone;
  }
  // Check for captured stones
  char flagCaptured = 0;
  while ((jHole < status._curPlayer * NBHOLEPLAYER || 
    jHole >= (status._curPlayer + 1) * NBHOLEPLAYER) &&
    (status._nbStone[jHole] == 2 ||
    status._nbStone[jHole] == 3)) {
    status._score[status._curPlayer] += status._nbStone[jHole];
    status._nbStone[jHole] = 0;
    flagCaptured = 1;
    --jHole;
  }
  // Check for special end conditions
  // First, check that the opponent is not starving
  int nbStoneOpp = 0;
  for (int iHole = 0; iHole < NBHOLE; ++iHole) {
    if (iHole < status._curPlayer * NBHOLEPLAYER || 
      iHole >= (status._curPlayer + 1) * NBHOLEPLAYER)
      nbStoneOpp += status._nbStone[iHole];
  }
  // If the opponent is starving
  if (nbStoneOpp == 0) {
    if (flagCaptured == 1) {
      // If there has been captured stones, it means the current
      // player has starved the opponent. The current player looses.
      status._end = 1;
      status._score[status._curPlayer] = 0.0;
    } else {
      // If there was no captured stones, it means the opponent
      // starved itself. The current player catches all his own stones.
      status._end = 1;
      for (int iHole = 0; iHole < NBHOLE; ++iHole) {
        if (iHole >= status._curPlayer * NBHOLEPLAYER && 
          iHole < (status._curPlayer + 1) * NBHOLEPLAYER)
          status._score[status._curPlayer] += 
            status._nbStone[iHole];
      }
    }
  }
  // Step the current player
  ++(status._curPlayer);
  if (status._curPlayer == NBPLAYER)
    status._curPlayer = 0;
  // Increment the nb of turn
  ++(status._nbTurn);


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
  fprintf(stream, "#%d: ", that->_nbTurn);
  for (int iHole = 0; iHole < NBHOLE; ++iHole)
    fprintf(stream, "%d ", that->_nbStone[iHole]);
  fprintf(stream, " score: ");
  for (int iPlayer = 0; iPlayer < NBPLAYER; ++iPlayer) {
    if (iPlayer == MFModelStatusGetSente(that))
      fprintf(stream, "^");
    fprintf(stream, "%d", that->_score[iPlayer]);
    if (iPlayer < NBPLAYER - 1)
      fprintf(stream, ":");
  }
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
  fprintf(stream, "move:%d", that->_iHole);
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
  (void)nbStatus;
  int nbRemainStoneCurStatus = 0;
  for (int iHole = NBHOLE; iHole--;)
    nbRemainStoneCurStatus += curStatus->_nbStone[iHole];
  int nbRemainStone = 0;
  for (int iHole = NBHOLE; iHole--;)
    nbRemainStone += that->_nbStone[iHole];
  if (nbRemainStone > nbRemainStoneCurStatus)
    return true;
  else
    return false;
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

  /*if (that->_score[0] > 0 || that->_score[1] > 0)
    return true;
  else
    return false;*/

  if (that->_end == 1 ||
    that->_nbTurn == NBMAXTURN)
    return true;
  bool ret = false;
  for (int iPlayer = NBPLAYER; iPlayer--;) {
    // Incorrect if NBPLAYER > 2
    if (that->_score[iPlayer] * 2 > NBSTONE)
      ret = true;
  }
  // For the case NBPLAYER > 2
  /*if (ret == false) {
    int nbRemainStone = 0;
    for (int iHole = NBHOLE; iHole-- && ret == false;)
      nbRemainStone += that->_nbStone[iHole];
    if (nbRemainStone == 0)
      ret = true;
  }*/
  return ret;
}

// Init the board
void MFModelStatusInit(MFModelStatus* const that) {
#if BUILDMODE == 0
  if (that == NULL) {
    MiniFrameErr->_type = PBErrTypeNullPointer;
    sprintf(MiniFrameErr->_msg, "'that' is null");
    PBErrCatch(MiniFrameErr);
  }
#endif
  that->_end = 0;
  for (int iPlayer = NBPLAYER; iPlayer--;) {
    that->_score[iPlayer] = 0;
    that->_nn[iPlayer] = NULL;
  }
  for (int iHole = NBHOLE; iHole--;)
    that->_nbStone[iHole] = NBINITSTONEPERHOLE;
  that->_curPlayer = 0;
  that->_nbTurn = 0;
}
