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
  MFModelStatus status = {._step = 0, ._pos = 0, ._tgt = 1};
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

  MFModelStatus status = {._step = 0, ._pos = 0, ._tgt = 1};
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
    GSetNbElem(MFWorldsToExpand(mf)) != 1 ||
    MFCurWorld(mf) != GSetGet(MFWorldsToExpand(mf), 0) ||
    ISEQUALF(mf->_timeUnusedExpansion, 0.0) == false ||
    ISEQUALF(mf->_percWorldReused, 0.0) == false ||
    mf->_maxDepthExp != MF_DEFAULTMAXDEPTHEXP ||
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
  float t = MF_DEFAULTTIMEEXPANSION + 1.0;
  MFSetMaxTimeExpansion(mf, t);
  if (ISEQUALF(MFGetMaxTimeExpansion(mf), t) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFSetMaxTimeExpansion failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFCurWorld(mf) != mf->_curWorld) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFCurWorld failed");
    PBErrCatch(MiniFrameErr);
  }
  if (MFWorldsComputed(mf) != &(mf->_worldsComputed)) {
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
  if (ISEQUALF(MFGetPercWorldReused(mf), 1.0) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetPercWorldReused failed");
    PBErrCatch(MiniFrameErr);
  }
  MFModelStatus modelWorld = {._step = 0, ._pos = 0, ._tgt = 1};
  MFWorld* world = MFWorldCreate(&modelWorld);
  MFAddWorldToComputed(mf, world);
  if (GSetNbElem(MFWorldsComputed(mf)) != 1 ||
    MFModelStatusIsSame(MFWorldStatus(world),
      (MFModelStatus*)GSetGet(MFWorldsComputed(mf), 0)) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFAddWorldToComputed failed");
    PBErrCatch(MiniFrameErr);
  }
  MFWorld* worldToExpand = MFWorldCreate(&modelWorld);
  MFAddWorldToExpand(mf, worldToExpand);
  if (GSetNbElem(MFWorldsToExpand(mf)) != 2) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFAddWorldToExpand failed");
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
  if (ISEQUALF(MFGetPercWorldReused(mf), 
    mf->_percWorldReused) == false) {
    MiniFrameErr->_type = PBErrTypeUnitTestFailed;
    sprintf(MiniFrameErr->_msg, "MFGetPercWorldReused failed");
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
  printf("Nb computed worlds: %d\n", MFGetNbComputedWorlds(mf));
  printf("Nb worlds to expand: %d\n", MFGetNbWorldsToExpand(mf));
  printf("Perc world reused: %f\n", MFGetPercWorldReused(mf));
  printf("Computed worlds:\n");
  GSetIterForward iter = 
    GSetIterForwardCreateStatic(MFWorldsComputed(mf));
  do {
    MFWorld* world = GSetIterGet(&iter);
    MFWorldTransPrintln(world, stdout);
  } while (GSetIterStep(&iter));
  if (mf->_timeUnusedExpansion < 0.0 ||
    MFGetNbComputedWorlds(mf) != 13 ||
    MFGetNbWorldsToExpand(mf) != 0 ||
    ISEQUALF(MFGetPercWorldReused(mf), 0.666667) == false) {
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
  if (MFCurWorld(mf) != GSetGet(MFWorldsComputed(mf), 1)) {
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
  MFSetWorldReusable(mf, false);
  // Loop until end of game
  int tgt[7] = {2,2,-1,-1,-1,-1,-1};
  while (!MFModelStatusIsEnd(&curWorld)) {
    // Set the start clock
    MFSetStartExpandClock(mf, clock());
    // Correct the current world in the MiniFrame
    MFSetCurWorld(mf, &curWorld);
    // Expand
    MFExpand(mf);
    // Display the current world
    printf("mf(");
    MFModelStatusPrint(MFWorldStatus(MFCurWorld(mf)), stdout);
    printf(") real(");
    MFModelStatusPrint(&curWorld, stdout);
    printf(")\n");
    MFWorldTransPrintln(MFCurWorld(mf), stdout);  
    /*printf("--- start of best story ---\n");
    MFWorldPrintBestStoryln(MFCurWorld(mf), 0, stdout);
    printf("--- end of best story ---\n");*/
    printf("\n");
    // Get best transition
    const MFModelTransition* bestTrans = MFBestTransition(mf, 0);
    if (bestTrans != NULL) {
      // Step with best transition
      curWorld = MFModelStatusStep(&curWorld, bestTrans);
    }
    // Apply external forces to the world
    curWorld._tgt = tgt[curWorld._step];
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

