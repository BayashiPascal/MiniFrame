#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include "pberr.h"
#include "pbmath.h"
#include "miniframe.h"

#define RANDOMSEED 0

void UnitTestMFTransitionCreateFree() {

  MFWorld world;
  MFModelTransition trans = {._move = 1};
  MFTransition act = MFTransitionCreateStatic(&world, &trans);
  if (act._fromWorld != &world ||
    act._toWorld != NULL ||
    memcmp(&(act._transition), &(trans), 
      sizeof(MFModelTransition)) != 0) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionCreateStatic failed");
    PBErrCatch(MiniFrameErr);
  }
  for (int iActor = MF_NBMAXACTOR; iActor--;)
    if (ISEQUALF(act._values[iActor], 0.0) == false) {
      MiniFrameErr->_type = PBErrTypeUnitTestFailed;
      sprintf(MiniFrameErr->_msg, "MFTransitionCreateStatic failed");
      PBErrCatch(MiniFrameErr);
    }
  MFTransitionFreeStatic(&act);

  printf("UnitTestMFTransitionCreateFree OK\n");
}

void UnitTestMFTransitionIsExpandable() {

  MFModelStatus status;
  MFWorld* world = MFWorldCreate(&status);
  MFModelTransition trans = {._move = 1};
  MFTransition act = MFTransitionCreateStatic(world, &trans);
  if (!MFTransitionIsExpandable(&act)) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionIsExpandable failed");
    PBErrCatch(MiniFrameErr);
  }
  act._toWorld = world;
  if (MFTransitionIsExpandable(&act)) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionIsExpandable failed");
    PBErrCatch(MiniFrameErr);
  }
  act._toWorld = NULL;
  world->_status._pos = world->_status._tgt;
  world->_transitions[0]._toWorld = world;
  if (MFTransitionIsExpandable(&act)) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionIsExpandable failed");
    PBErrCatch(MiniFrameErr);
  }
  world->_transitions[0]._toWorld = NULL;
  MFTransitionFreeStatic(&act);
  MFWorldFree(&world);

  printf("UnitTestMFTransitionIsExpandable OK\n");
}

void UnitTestMFTransitionIsExpanded() {

  MFModelStatus status;
  MFWorld* world = MFWorldCreate(&status);
  MFModelTransition trans = {._move = 1};
  MFTransition act = MFTransitionCreateStatic(world, &trans);
  if (MFTransitionIsExpanded(&act)) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionIsExpanded failed");
    PBErrCatch(MiniFrameErr);
  }
  act._toWorld = world;
  if (!MFTransitionIsExpanded(&act)) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionIsExpanded failed");
    PBErrCatch(MiniFrameErr);
  }
  MFTransitionFreeStatic(&act);
  MFWorldFree(&world);

  printf("UnitTestMFTransitionIsExpanded OK\n");
}

void UnitTestMFTransitionGetSet() {
  MFWorld worldFrom;
  MFWorld worldTo;
  MFModelTransition trans = {._move = 1};
  MFTransition act = MFTransitionCreateStatic(&worldFrom, &trans);
  act._toWorld = &worldTo;
  if (MFTransitionToWorld(&act) != &worldTo) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionToWorld failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFTransitionFromWorld(&act) != &worldFrom) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionFromWorld failed");
    PBErrCatch(MiniFrameErr);
  }
  MFTransitionSetValue(&act, 0, 1.0);
  if (ISEQUALF(act._values[0], 1.0) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionSetValue failed");
    PBErrCatch(MiniFrameErr);
  }
  if (ISEQUALF(MFTransitionGetValue(&act, 0), 1.0) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionGetValue failed");
    PBErrCatch(MiniFrameErr);
  }
  MFWorld worldB;
  MFTransitionSetToWorld(&act, &worldB);
  if (MFTransitionToWorld(&act) != &worldB) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFTransitionSetToWorld failed");
    PBErrCatch(MiniFrameErr);
  }
  MFTransitionFreeStatic(&act);

  printf("UnitTestMFTransitionGetSet OK\n");
}

void UnitTestMFTransition() {
  UnitTestMFTransitionCreateFree();
  UnitTestMFTransitionIsExpandable();
  UnitTestMFTransitionIsExpanded();
  UnitTestMFTransitionGetSet();
  printf("UnitTestMFTransition OK\n");
}

void UnitTestMFWorldCreateFree() {

  MFModelStatus modelWorld = {._step = 0, ._pos = 0, ._tgt = 1};
  MFWorld* world = MFWorldCreate(&modelWorld);
  if (world == NULL ||
    GSetNbElem(&(world->_sources)) != 0 ||
    world->_nbTransition != 3) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldCreate failed");
    PBErrCatch(MiniFrameErr);
  }
  float val[MF_NBMAXACTOR] = {0.0};
  val[0] = -1.0;
  for (int iActor = MF_NBMAXACTOR; iActor--;)
    if (ISEQUALF(world->_values[iActor], val[iActor]) == false) {
      MiniFrameErr->_type = PBErrTypeUnitTestFailed;
      sprintf(MiniFrameErr->_msg, "MFWorldCreate failed");
      PBErrCatch(MiniFrameErr);
    }
  MFWorldFree(&world);
  if (world != NULL) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldFree failed");
    PBErrCatch(MiniFrameErr);
  }

  printf("UnitTestMFWorldCreateFree OK\n");
}

void UnitTestMFWorldGetSet() {
  MFModelStatus modelWorld = {._step = 0, ._pos = 0, ._tgt = 1};
  MFWorld* world = MFWorldCreate(&modelWorld);
  if (MFWorldStatus(world) != &(world->_status)) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldStatus failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFWorldGetNbTrans(world) != 3) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldGetNbTrans failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFWorldSources(world) != &(world->_sources)) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldSources failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFWorldValues(world) != world->_values) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldValues failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFWorldTransition(world, 0) != world->_transitions ||
    MFWorldTransition(world, 1) != world->_transitions + 1 ||
    MFWorldTransition(world, 2) != world->_transitions + 2) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldTransition failed");
    PBErrCatch(MiniFrameErr);
  }
  world->_values[0] = 1.0;
  if (ISEQUALF(MFWorldGetValue(world, 0), 1.0) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldGetValue failed");
    PBErrCatch(MiniFrameErr);
  }
  MFWorldFree(&world);
  printf("UnitTestMFWorldGetSet OK\n");
}

void UnitTestMFWorldComputeTransition() {
  MFModelStatus modelWorld = {._step = 0, ._pos = 0, ._tgt = 1};
  MFWorld* world = MFWorldCreate(&modelWorld);
  MFModelStatus statusA = {._step = 1, ._pos = -1, ._tgt = 1};
  MFModelStatus statusB = {._step = 1, ._pos = 0, ._tgt = 1};
  MFModelStatus statusC = {._step = 1, ._pos = 1, ._tgt = 1};
  MFModelStatus status = MFWorldComputeTransition(world, 0);
  if (memcmp(&status, &statusA, sizeof(MFModelStatus)) != 0) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldComputeTransition failed");
    PBErrCatch(MiniFrameErr);
  }
  status = MFWorldComputeTransition(world, 1);
  if (memcmp(&status, &statusB, sizeof(MFModelStatus)) != 0) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldComputeTransition failed");
    PBErrCatch(MiniFrameErr);
  }
  status = MFWorldComputeTransition(world, 2);
  if (memcmp(&status, &statusC, sizeof(MFModelStatus)) != 0) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldComputeTransition failed");
    PBErrCatch(MiniFrameErr);
  }
  MFWorldFree(&world);
  printf("UnitTestMFWorldComputeTransition OK\n");
}

void UnitTestMFWorld() {
  UnitTestMFWorldCreateFree();
  UnitTestMFWorldGetSet();
  UnitTestMFWorldComputeTransition();
  printf("UnitTestMFWorld OK\n");
}

void UnitTestMiniFrameCreateFree() {

  MFModelStatus initStatus = {._step = 0, ._pos = 0, ._tgt = 1};
  MiniFrame* mf = MiniFrameCreate(&initStatus);
  if (mf == NULL ||
    mf->_nbStep != 0 ||
    ISEQUALF(mf->_maxTimeExpansion, MF_DEFAULTTIMEEXPANSION) == false ||
    MFModelStatusIsSame(&initStatus, &(MFCurWorld(mf)->_status)) == false ||
    MFCurWorld(mf) != GSetGet(MFWorlds(mf), 0) ||
    GSetNbElem(MFWorlds(mf)) != 1 ||
    ISEQUALF(mf->_timeUnusedExpansion, 0.0) == false ||
    ISEQUALF(mf->_percWorldReused, 0.0) == false ||
    mf->_nbWorldExpanded != 0 ||
    mf->_nbWorldUnexpanded != 0 ||
    mf->_nbRemovedWorld != 0 ||
    mf->_timeEndExpansion <= 0.0 ||
    mf->_maxDepthExp != -1 ||
    mf->_expansionType != MFExpansionTypeValue ||
    mf->_nbTransMonteCarlo != MF_NBTRANSMONTECARLO ||
    mf->_pruningDeltaVal != MF_PRUNINGDELTAVAL ||
    mf->_reuseWorld != false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MiniFrameCreate failed");
    PBErrCatch(MiniFrameErr);
  }
  MiniFrameFree(&mf);
  if (mf != NULL) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MiniFrameFree failed");
    PBErrCatch(MiniFrameErr);
  }

  printf("UnitTestMiniFrameCreateFree OK\n");
}

void UnitTestMiniFrameGetSet() {
  MFModelStatus initWorld = {._step = 0, ._pos = 0, ._tgt = 1};
  MiniFrame* mf = MiniFrameCreate(&initWorld);
  if (ISEQUALF(MFGetMaxTimeExpansion(mf),
    mf->_maxTimeExpansion) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetMaxTimeExpansion failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFGetNbComputedWorld(mf) != 1) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetNbComputedWorld failed");
    PBErrCatch(MiniFrameErr);
  }
  float t = MF_DEFAULTTIMEEXPANSION + 1.0;
  MFSetMaxTimeExpansion(mf, t);
  if (ISEQUALF(MFGetMaxTimeExpansion(mf), t) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFSetMaxTimeExpansion failed");
    PBErrCatch(MiniFrameErr);
  }
  if (ISEQUALF(MFGetTimeEndExpansion(mf),
    mf->_timeEndExpansion) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetTimeEndExpansion failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFCurWorld(mf) != mf->_curWorld) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFCurWorld failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFWorlds(mf) != &(mf->_worlds)) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorlds failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFIsWorldReusable(mf) != mf->_reuseWorld) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFIsWorldReusable failed");
    PBErrCatch(MiniFrameErr);
  }
  bool reuse = !MFIsWorldReusable(mf);
  MFSetWorldReusable(mf, reuse);
  if (MFIsWorldReusable(mf) != reuse) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFSetWorldReusable failed");
    PBErrCatch(MiniFrameErr);
  }
  mf->_percWorldReused = 1.0;
  if (ISEQUALF(MFGetPercWordReused(mf), 1.0) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetPercWordReused failed");
    PBErrCatch(MiniFrameErr);
  }
  MFModelStatus modelWorld = {._step = 0, ._pos = 0, ._tgt = 1};
  MFWorld* world = MFWorldCreate(&modelWorld);
  MFAddWorld(mf, world, 0);
  if (GSetNbElem(MFWorlds(mf)) != 2 ||
    MFModelStatusIsSame(MFWorldStatus(world),
      (MFModelStatus*)GSetGet(MFWorlds(mf), 1)) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFAddWorld failed");
    PBErrCatch(MiniFrameErr);
  }
  mf->_nbWorldExpanded = 1;
  if (MFGetNbWorldExpanded(mf) != mf->_nbWorldExpanded) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetNbWorldExpanded failed");
    PBErrCatch(MiniFrameErr);
  }
  mf->_nbWorldUnexpanded = 1;
  if (MFGetNbWorldUnexpanded(mf) != mf->_nbWorldUnexpanded) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetNbWorldUnexpanded failed");
    PBErrCatch(MiniFrameErr);
  }
  mf->_timeSearchWorld = 2.0;
  if (ISEQUALF(MFGetTimeSearchWorld(mf), 
    mf->_timeSearchWorld) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetTimeSearchWorld failed");
    PBErrCatch(MiniFrameErr);
  }
  mf->_timeUnusedExpansion = 3.0;
  if (ISEQUALF(MFGetTimeUnusedExpansion(mf), 
    mf->_timeUnusedExpansion) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetTimeUnusedExpansion failed");
    PBErrCatch(MiniFrameErr);
  }
  mf->_percWorldReused = 4.0;
  if (ISEQUALF(MFGetPercWordReused(mf), 
    mf->_percWorldReused) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetPercWordReused failed");
    PBErrCatch(MiniFrameErr);
  }
  clock_t now = clock();
  MFSetStartExpandClock(mf, now);
  if (mf->_startExpandClock != now) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetStartExpandClock failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFGetStartExpandClock(mf) != now) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetStartExpandClock failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFGetMaxDepthExp(mf) != mf->_maxDepthExp) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetMaxDepthExp failed");
    PBErrCatch(MiniFrameErr);
  }
  MFSetMaxDepthExp(mf, 3);
  if (MFGetMaxDepthExp(mf) != 3) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFSetMaxDepthExp failed");
    PBErrCatch(MiniFrameErr);
  }
  MFSetMaxDepthExp(mf, -2);
  if (MFGetMaxDepthExp(mf) != -1) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFSetMaxDepthExp failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFGetExpansionType(mf) != mf->_expansionType) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetExpansionType failed");
    PBErrCatch(MiniFrameErr);
  }
  MFSetExpansionType(mf, MFExpansionTypeWidth);
  if (MFGetExpansionType(mf) != MFExpansionTypeWidth) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFSetExpansionType failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFGetNbTransMonteCarlo(mf) != mf->_nbTransMonteCarlo) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetNbTransMonteCarlo failed");
    PBErrCatch(MiniFrameErr);
  }
  MFSetNbTransMonteCarlo(mf, 10);
  if (MFGetNbTransMonteCarlo(mf) != 10) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFSetNbTransMonteCarlo failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFGetPruningDeltaVal(mf) != mf->_pruningDeltaVal) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetPruningDeltaVal failed");
    PBErrCatch(MiniFrameErr);
  }
  MFSetPruningDeltaVal(mf, 10.0);
  if (!ISEQUALF(MFGetPruningDeltaVal(mf), 10.0)) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFSetPruningDeltaVal failed");
    PBErrCatch(MiniFrameErr);
  }
  MiniFrameFree(&mf);
  printf("UnitTestMiniFrameGetSet OK\n");
}

void UnitTestMiniFrameExpandSetCurWorld() {
  MFModelStatus initWorld = {._step = 0, ._pos = 0, ._tgt = 2};
  MiniFrame* mf = MiniFrameCreate(&initWorld);
  MFSetStartExpandClock(mf, clock());
  MFSetWorldReusable(mf, true);
  MFExpand(mf);
  printf("Time unused by MFExpand: %f\n", MFGetTimeUnusedExpansion(mf));
  printf("Time search world to expand: %f\n", MFGetTimeSearchWorld(mf));
  printf("Nb world expanded: %d\n", MFGetNbWorldExpanded(mf));
  printf("Nb world unexpanded: %d\n", MFGetNbWorldUnexpanded(mf));
  printf("Nb world removed: %d\n", MFGetNbWorldRemoved(mf));
  printf("Perc world reused: %f\n", MFGetPercWordReused(mf));
  printf("Computed worlds:\n");
  GSetIterForward iter = GSetIterForwardCreateStatic(MFWorlds(mf));
  do {
    MFWorld* world = GSetIterGet(&iter);
    MFWorldTransPrintln(world, stdout);
  } while (GSetIterStep(&iter));
  if (mf->_timeUnusedExpansion < 0.0 ||
    MFGetNbWorldExpanded(mf) != 15 ||
    MFGetNbWorldUnexpanded(mf) != 0 ||
    MFGetNbWorldRemoved(mf) != 0 ||
    ISEQUALF(MFGetPercWordReused(mf), 0.666667) == false ||
    ISEQUALF(MFGetTimeSearchWorld(mf), 100.0) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFExpand failed");
    PBErrCatch(MiniFrameErr);
  }
  const MFModelTransition* bestTrans = MFBestTransition(mf, 0);
  printf("Best action: %d\n", bestTrans->_move);
  if (bestTrans->_move != 1) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetBestTransition failed");
    PBErrCatch(MiniFrameErr);
  }
  if (ISEQUALF(MFWorldGetForecastValue(MFCurWorld(mf), 0), 0.0) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldGetPOVValue failed");
    PBErrCatch(MiniFrameErr);
  }
  if (ISEQUALF(
    MFWorldGetForecastValue(MFCurWorld(mf), 0), 0.0) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFWorldGetForecastValue failed");
    PBErrCatch(MiniFrameErr);
  }
  MFModelStatus nextWorld = {._pos = -1, ._tgt = 2};
  MFSetCurWorld(mf, &nextWorld);
  if (MFCurWorld(mf) != GSetGet(MFWorlds(mf), 2) ||
    MFGetNbComputedWorld(mf) != 6) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFSetCurWorld failed");
    PBErrCatch(MiniFrameErr);
  }
  MiniFrameFree(&mf);
  printf("UnitTestMiniFrameExpandSetCurWorld OK\n");
}

void UnitTestMiniFrameFullExample() {
  // Initial world
  MFModelStatus curWorld = {._step = 0, ._pos = 0, ._tgt = 2};
  // Create the MiniFrame
  MiniFrame* mf = MiniFrameCreate(&curWorld);
  // Set reusable worlds
  MFSetWorldReusable(mf, true);
  // Loop until end of game
  int tgt[7] = {2,2,-1,-1,-1,-1,-1};
  while (!MFModelStatusIsEnd(&curWorld)) {
    // Set the start clock
    MFSetStartExpandClock(mf, clock());
    // Correct the current world in the MiniFrame
    MFSetCurWorld(mf, &curWorld);
    // Expand
    MFExpand(mf);
    // Get best transition
    const MFModelTransition* bestTrans = MFBestTransition(mf, 0);
    if (bestTrans != NULL) {
      // Step with best transition
      curWorld = MFModelStatusStep(&curWorld, bestTrans);
    }
    // Apply external forces to the world
    curWorld._tgt = tgt[curWorld._step];
    // Display the current world
    printf("mf(");
    MFModelStatusPrint(MFWorldStatus(MFCurWorld(mf)), stdout);
    printf(") real(");
    MFModelStatusPrint(&curWorld, stdout);
    printf(")\n");
    /*MFWorldTransPrintln(MFCurWorld(mf), stdout);  
    printf("--- start of best story ---\n");
    MFWorldPrintBestStoryln(MFCurWorld(mf), 0, stdout);
    printf("--- end of best story ---\n");
    printf("\n");*/
  }
  MiniFrameFree(&mf);
  printf("UnitTestMiniFrameFullExample OK\n");
}

void UnitTestMiniFrame() {
  UnitTestMiniFrameCreateFree();
  UnitTestMiniFrameGetSet();
  UnitTestMiniFrameExpandSetCurWorld();
  UnitTestMiniFrameFullExample();
  printf("UnitTestMiniFrame OK\n");
}

void UnitTestAll() {
  UnitTestMFTransition();
  UnitTestMFWorld();
  UnitTestMiniFrame();
  printf("UnitTestAll OK\n");
}

int main() {
  UnitTestAll();
  // Return success code
  return 0;
}

