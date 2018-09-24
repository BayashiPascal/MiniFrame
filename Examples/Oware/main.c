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

void RunDemo(float expansionTime, bool useNN) {
  // Initial world
  MFModelStatus curWorld;
  MFModelStatusInit(&curWorld);
  // Display the current world
  MFModelStatusPrint(&curWorld, stdout);
  printf("\n");
  // Create the MiniFrame
  MiniFrame* mf = MiniFrameCreate(&curWorld);
  // If we use a NeuraNet as evaluation for player #0
  if (useNN) {
    // Try to load the NeuraNet from ./bestnn.txt
    FILE* stream = fopen("./bestnn.txt", "r");
    if (stream != NULL) {
      if (!NNLoad(curWorld._nn, stream)) {
        printf("Couldn't reload the NeuraNet from ./bestnn.txt\n");
        printf("Will use the default evaluation function\n");
      }
      fclose(stream);
    } else {
      printf("Couldn't reload the NeuraNet from ./bestnn.txt\n");
      printf("Will use the default evaluation function\n");
    }
  }
  // Set the expansion time
  MFSetMaxTimeExpansion(mf, expansionTime);
  // Set reusable worlds
  //MFSetWorldReusable(mf, true);
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
    //MFWorldTransPrintln(MFCurWorld(mf), stdout);
    printf("--- start of best story ---\n");
    MFWorldPrintBestStoryln(MFCurWorld(mf), 
      curWorld._curPlayer, stdout);
    printf("--- end of best story ---\n");
    // Display info about exansion
    printf("exp: %d ", MFGetNbWorldExpanded(mf));
    printf("unexp: %d ", MFGetNbWorldUnexpanded(mf));
    printf("comp: %d ", MFGetNbComputedWorld(mf));
    printf("removed: %d ", MFGetNbWorldRemoved(mf));
    printf("reused: %f ", MFGetPercWordReused(mf));
    printf("unused: %fms\n", MFGetTimeUnusedExpansion(mf));
    if (MFGetTimeUnusedExpansion(mf) < 0.0) {
      flagEnd = true;
      curWorld._score[curWorld._curPlayer] = -1;
    } else {
      // Get best transition
      const MFModelTransition* bestTrans = 
        MFGetBestTransition(mf, MFModelStatusGetSente(&curWorld));
      if (bestTrans != NULL) {
        // Display the transition's information 
        printf("sente: %d ", curWorld._curPlayer);
        MFModelTransitionPrint(bestTrans, stdout);
        printf(" forecast: %f", 
          MFTransitionGetForecastValue((MFTransition*)bestTrans, 
          curWorld._curPlayer));
        printf("\n");
        // Step with best transition
        curWorld = MFModelStatusStep(&curWorld, bestTrans);
      } else {
        flagEnd = true;
      }
      // Apply external forces to the world
      // curWorld. = ... ;
    }
    // Display the current world
    MFModelStatusPrint(&curWorld, stdout);
    printf("\n\n");
    fflush(stdout);
  }
  MFModelStatusFreeStatic(&curWorld);
  MiniFrameFree(&mf);
}

void Train(int nbEpoch, int sizePool, int nbElite, int nbGameEpoch,
  float expansionTime) {
  // Display parameters
  printf("Will train with following parameters:\n");
  printf("nbEpoch: %d\n", nbEpoch);
  printf("sizePool: %d\n", sizePool);
  printf("nbElite: %d\n", nbElite);
  printf("nbGameEpoch: %d\n", nbGameEpoch);
  printf("expansionTime: %fms\n", expansionTime);
  // Create the NeuraNet
  
  // Create the GenAlg
  
  // Reload the NeuraNet if possible
  
  // Create the ELORank
  
  // Declare a variable to memorize the current epoch
  int iEpoch = 0;
  // Loop on epochs
  while (iEpoch < nbEpoch) {
    // Declare a variable to memorize the current game
    int iGame = 0;
    // Loop on games
    while (iGame < nbGameEpoch) {
      // Set two randomly selected NeuraNet to players
      
      // Play the game
      
      // Update the ELORank with the result
      
      // Increment the current game
      ++iGame;
    }
    // Update the values of each NeuraNet in the GenAlg with their ELORank
    
    // Step the GenAlg
    
    // Save the best NeuraNet to ./bestnn.txt
    
    // Increment the current epoch
    ++iEpoch;
  }
}

int main(int argc, char** argv) {
  // Init the random generator
  srandom(time(NULL));
  // Declare a variable to memorize the mode
  // 0: demo (default)
  // 1: train mode
  // 2: demo with trained NeuraNet as player #0
  int mode = 0;
  // Declare a variable to memorize the expansion time (in millisec)
  float expansionTime = 100.0;
  // Declare a variable to memorize the number of epoch for training
  int nbEpoch = 10;
  // Declare variables to memorize the size of pool, number of elites,
  // number of game per epoch for training
  int nbElite = 5;
  int sizePool = 20;
  int nbGameEpoch = 100;
  // Process argument
  for (int iArg = 0; iArg < argc; ++iArg) {
    if (strcmp(argv[iArg], "-help") == 0) {
      printf("main [-demo] [-demoNN] [-train] [-nbEpoch <nbEpoch>] ");
      printf("[-nbElite <nbElite>] [-sizePool <sizePool>] ");
      printf("[-nbGameEpoch <nbGameEpoch>] [-expTime <expansionTime>]\n");
      exit(0);
    } else if (strcmp(argv[iArg], "-demo") == 0) {
      mode = 0;
    } else if (strcmp(argv[iArg], "-train") == 0) {
      mode = 1;
    } else if (strcmp(argv[iArg], "-demoNN") == 0) {
      mode = 2;
    } else if (strcmp(argv[iArg], "-nbEpoch") == 0 && iArg < argc - 1) {
      ++iArg;
      nbEpoch = atoi(argv[iArg]);
    } else if (strcmp(argv[iArg], "-nbElite") == 0 && iArg < argc - 1) {
      ++iArg;
      nbElite = atoi(argv[iArg]);
    } else if (strcmp(argv[iArg], "-sizePool") == 0 && iArg < argc - 1) {
      ++iArg;
      sizePool = atoi(argv[iArg]);
    } else if (strcmp(argv[iArg], "-nbGameEpoch") == 0 && iArg < argc - 1) {
      ++iArg;
      nbGameEpoch = atoi(argv[iArg]);
    } else if (strcmp(argv[iArg], "-expTime") == 0 && iArg < argc - 1) {
      ++iArg;
      expansionTime = atof(argv[iArg]);
    }
  }
  
  if (mode == 0) {
    RunDemo(expansionTime, false);
  } else if (mode == 1) {
    Train(nbEpoch, sizePool, nbElite, nbGameEpoch, expansionTime);
  } else if (mode == 2) {
    RunDemo(expansionTime, true);
  }

  // Return success code
  return 0;
}

