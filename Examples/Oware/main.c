#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include "pberr.h"
#include "pbmath.h"
#include "genalg.h"
#include "elorank.h"
#include "neuranet.h"
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
    //MFWorldTransPrintln(MFCurWorld(mf), stdout);
    /*printf("--- start of best story ---\n");
    MFWorldPrintBestStoryln(MFCurWorld(mf), 
      curWorld._curPlayer, stdout);
    printf("--- end of best story ---\n");*/
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
    printf("\n");
    fflush(stdout);
  }
  // Free memory
  for (int iPlayer = NBPLAYER; iPlayer--;) {
    if (curWorld._nn[iPlayer] != NULL)
      NeuraNetFree(curWorld._nn + iPlayer);
  }
  MiniFrameFree(&mf);
}

void TrainOneGame(float expansionTime, GenAlgAdn** adns, GSet* result) {
  // Initial world
  MFModelStatus curWorld;
  MFModelStatusInit(&curWorld);
  // Create the MiniFrame
  MiniFrame* mf = MiniFrameCreate(&curWorld);
  // Set the NeuraNet for each actor
  for (int iActor = 0; iActor < NBPLAYER; ++iActor) {
    NeuraNet* neuraNet = NeuraNetCreate(MF_MODEL_NN_NBINPUT,
      MF_MODEL_NN_NBOUTPUT, MF_MODEL_NN_NBHIDDEN, 
      MF_MODEL_NN_NBBASES, MF_MODEL_NN_NBLINKS);
    NNSetBases(neuraNet, GAAdnAdnF(adns[iActor]));
    NNSetLinks(neuraNet, GAAdnAdnI(adns[iActor]));
    curWorld._nn[iActor] = neuraNet;
  }
  // Set the expansion time
  MFSetMaxTimeExpansion(mf, expansionTime);
  // Set reusable worlds
  MFSetWorldReusable(mf, true);
  // Flag to end the game
  bool flagEnd = false;
  // Loop until end of game
  while (!MFModelStatusIsEnd(&curWorld) && !flagEnd) {
    // Set the start clock
    MFSetStartExpandClock(mf, clock());
    // Correct the current world in the MiniFrame
    MFSetCurWorld(mf, &curWorld);
    // Expand
    MFExpand(mf);
    if (MFGetTimeUnusedExpansion(mf) < 0.0) {
      flagEnd = true;
      curWorld._score[curWorld._curPlayer] = -1;
    } else {
      // Get best transition
      const MFModelTransition* bestTrans = 
        MFGetBestTransition(mf, MFModelStatusGetSente(&curWorld));
      if (bestTrans != NULL) {
        // Step with best transition
        curWorld = MFModelStatusStep(&curWorld, bestTrans);
      } else {
        flagEnd = true;
      }
    }
  }
  // Update result
  GSetFlush(result);
  for (int iActor = 0; iActor < NBPLAYER; ++iActor)
    GSetAddSort(result, adns[iActor], curWorld._score[iActor]);
  // Free memory
  for (int iPlayer = NBPLAYER; iPlayer--;) {
    if (curWorld._nn[iPlayer] != NULL)
      NeuraNetFree(curWorld._nn + iPlayer);
  }
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
  // Create a NeuraNet
  NeuraNet* neuraNet = NeuraNetCreate(MF_MODEL_NN_NBINPUT,
    MF_MODEL_NN_NBOUTPUT, MF_MODEL_NN_NBHIDDEN, 
    MF_MODEL_NN_NBBASES, MF_MODEL_NN_NBLINKS);
  // Create the GenAlg
  GenAlg* genAlg = GenAlgCreate(sizePool, nbElite, 
    NNGetGAAdnFloatLength(neuraNet), NNGetGAAdnIntLength(neuraNet)); 
  NNSetGABoundsBases(neuraNet, genAlg);
  NNSetGABoundsLinks(neuraNet, genAlg);
  GASetTypeNeuraNet(genAlg, MF_MODEL_NN_NBINPUT, 
    MF_MODEL_NN_NBHIDDEN, MF_MODEL_NN_NBOUTPUT);
  GAInit(genAlg);
  // Reload the GenAlg if possible
  
  // Declare a GSet to memorize the result
  GSet result = GSetCreateStatic();
  // Declare a variable to memorize the current epoch
  int iEpoch = 0;
  // Loop on epochs
  while (iEpoch < nbEpoch) {
    // Create the ELORank
    ELORank* eloRank = ELORankCreate();
    for (int iAdn = 0; iAdn < sizePool; ++iAdn)
      ELORankAdd(eloRank, GSetGet(GAAdns(genAlg), iAdn));
    // Declare a variable to memorize the current game
    int iGame = 0;
    // Loop on games
    while (iGame < nbGameEpoch) {
      printf("Epoch %03d/%03d Game %03d/%03d   \r", 
        iEpoch + 1, nbEpoch, iGame + 1, nbGameEpoch);
      fflush(stdout);
      // Select randomly two adns
      GenAlgAdn* adns[NBPLAYER] = {NULL};
      for (int iActor = 0; iActor < NBPLAYER; ++iActor) {
        int iAdn = (int)round(rnd() * (float)(sizePool - 1));
        adns[iActor] = GSetGet(GAAdns(genAlg), iAdn);
      }
      // Play the game
      TrainOneGame(expansionTime, adns, &result);
//printf("%p %f %p %f\n", GSetGet(&result, 0), GSetElemGetSortVal(GSetElement(&result, 0)),GSetGet(&result, 1), GSetElemGetSortVal(GSetElement(&result, 1)));
      // Update the ELORank with the result
      ELORankUpdate(eloRank, &result);
      // Increment the current game
      ++iGame;
    }
    printf("\n");
    fflush(stdout);
    // Update the values of each adn in the GenAlg with their ELORank
    for (int iAdn = 0; iAdn < sizePool; ++iAdn) {
      GenAlgAdn* adn = GSetGet(GAAdns(genAlg), iAdn);
      float elo = ELORankGetELO(eloRank, adn);
      GASetAdnValue(genAlg, adn, elo);
printf("%d %f\n", iAdn, elo);
    }
    // Step the GenAlg
    GAStep(genAlg);
    // Save the best NeuraNet to ./bestnn.txt
    
    // Save the GenAlg to ./bestga.txt
    
    // Increment the current epoch
    ++iEpoch;
    // Free memory
    ELORankFree(&eloRank);
  }
  // Free memory
  GSetFlush(&result);
  GenAlgFree(&genAlg);
  NeuraNetFree(&neuraNet);
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
  float expansionTime = 1.0; //100.0;
  // Declare a variable to memorize the number of epoch for training
  int nbEpoch = 100;
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
mode=1;  
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

