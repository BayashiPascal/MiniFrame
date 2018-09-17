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

void RunGame() {
  // Initial world
  MFModelStatus curWorld;
  MFModelStatusInit(&curWorld);
  // Display the current world
  MFModelStatusPrint(&curWorld, stdout);
  printf("\n");
  // Create the MiniFrame
  MiniFrame* mf = MiniFrameCreate(&curWorld);
  // Set the expansion time
  MFSetMaxTimeExpansion(mf, 100.0);
  // Set reusable worlds
  MFSetWorldReusable(mf, true);
  // Flag to end the game
  bool flagEnd = false;
  // Loop until end of game
  while (!MFModelStatusIsEnd(&curWorld) && !flagEnd) {
    printf("-----------\n");
    // Set the start clock
    MFSetStartExpandClock(mf, clock());
    // Correct the current world in the MiniFrame
    MFSetCurWorld(mf, &curWorld);
    // Expand
    MFExpand(mf);
    // Display info about exansion
    printf("exp: %d ", MFGetNbWorldExpanded(mf));
    printf("comp: %d ", MFGetNbComputedWorld(mf));
    printf("unused: %fms\n", MFGetTimeUnusedExpansion(mf));
    if (MFGetTimeUnusedExpansion(mf) < 0.0)
      flagEnd = true;
    // Get best transition
    const MFModelTransition* bestTrans = 
      MFGetBestTransition(mf, MFModelStatusGetSente(&curWorld));
    if (bestTrans != NULL) {
      // Display the transition
      MFModelTransitionPrint(bestTrans, stdout);
      printf("\n");
      // Step with best transition
      curWorld = MFModelStatusStep(&curWorld, bestTrans);
    } else {
      flagEnd = true;
    }
    // Apply external forces to the world
    // curWorld. = ... ;
    // Display the current world
    MFModelStatusPrint(&curWorld, stdout);
    printf("\n");
    fflush(stdout);
  }
  MiniFrameFree(&mf);
}

int main() {
  RunGame();
  // Return success code
  return 0;
}

