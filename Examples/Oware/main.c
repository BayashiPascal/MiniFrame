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
    // Display info
    printf("computed: %d, ", MFGetNbComputedWorlds(mf));
    printf("to expand: %d, ", MFGetNbWorldsToExpand(mf));
    printf("to free: %d, ", MFGetNbWorldsToFree(mf));
    printf("reused: %f, ", MFGetPercWordReused(mf));
    printf("unused: %fms\n", MFGetTimeUnusedExpansion(mf));
    // Free the disposable worlds
    MFFreeDisposableWorld(mf);
    // Expand
    MFExpand(mf);
    //MFWorldTransPrintln(MFCurWorld(mf), stdout);
    /*printf("--- start of best story ---\n");
    MFWorldPrintBestStoryln(MFCurWorld(mf), 
      curWorld._curPlayer, stdout);
    printf("--- end of best story ---\n");*/
    if (MFGetTimeUnusedExpansion(mf) < 0.0) {
      flagEnd = true;
      curWorld._score[curWorld._curPlayer] = -1;
    } else {
      // Get best transition
      const MFModelTransition* bestTrans = 
        MFBestTransition(mf, MFModelStatusGetSente(&curWorld));
      if (bestTrans != NULL) {
        // Display the transition's information 
        printf("sente: %d ", curWorld._curPlayer);
        MFModelTransitionPrint(bestTrans, stdout);
        printf(" forecast: %f", 
          MFTransitionGetValue((MFTransition*)bestTrans, 
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
    if (adns[iActor] != (void*)1) {
      NeuraNet* neuraNet = NeuraNetCreate(MF_MODEL_NN_NBINPUT,
        MF_MODEL_NN_NBOUTPUT, MF_MODEL_NN_NBHIDDEN, 
        MF_MODEL_NN_NBBASES, MF_MODEL_NN_NBLINKS);
      NNSetBases(neuraNet, GAAdnAdnF(adns[iActor]));
      NNSetLinks(neuraNet, GAAdnAdnI(adns[iActor]));
      curWorld._nn[iActor] = neuraNet;
    } else {
      curWorld._nn[iActor] = NULL;
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
    // Set the start clock
    MFSetStartExpandClock(mf, clock());
    // Correct the current world in the MiniFrame
    MFSetCurWorld(mf, &curWorld);
    // Free the disposable worlds
    MFFreeDisposableWorld(mf);
    // Expand
    MFExpand(mf);
    if (MFGetTimeUnusedExpansion(mf) < 0.0) {
      flagEnd = true;
      curWorld._score[curWorld._curPlayer] = -1;
    } else {
      // Get best transition
      const MFModelTransition* bestTrans = 
        MFBestTransition(mf, MFModelStatusGetSente(&curWorld));
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
  FILE* stream = fopen("./bestga.txt", "r");
  if (stream != NULL) {
    printf("Reload the previous GenAlg from ./bestga.txt\n");
    if (GALoad(&genAlg, stream)) {
      printf("Couldn't reload the GenAlg\n");
      exit(1);
    }
  }
  // Declare a stream to save results
  FILE* streamRes = fopen("./res.txt", "w");
  if (streamRes == NULL) {
    printf("Couldn't open ./res.txt\n");
    exit(1);
  }
  // Declare a GSet to memorize the result
  GSet result = GSetCreateStatic();
  // Create the ELORank
  ELORank* eloRank = ELORankCreate();
  for (int iAdn = 0; iAdn < sizePool; ++iAdn)
    ELORankAdd(eloRank, GSetGet(GAAdns(genAlg), iAdn));
  ELORankAdd(eloRank, (GenAlgAdn*)GABestAdn(genAlg));
  ELORankAdd(eloRank, (void*)1);
  // Declare a variable to memorize the current epoch
  int iEpoch = 0;
  
  int iAdnGlob = 0;
  
  // Loop on epochs
  while (iEpoch < nbEpoch) {
    // Declare a variable to memorize the current game
    int iGame = 0;
    // Loop on games
    while (iGame < nbGameEpoch) {
      float eloRef = ELORankGetELO(eloRank, (void*)1);
      float eloSoftRef = ELORankGetSoftELO(eloRank, (void*)1);
      float eloBest = 0.0;
      float eloSoftBest = 0.0;
      long int idBest = 0;
      if (ELORankGetRanked(eloRank, 0)->_data == (void*)1) {
        eloBest = ELORankGetELO(eloRank, 
          ELORankGetRanked(eloRank, 1)->_data);
        eloSoftBest = ELORankGetSoftELO(eloRank, 
          ELORankGetRanked(eloRank, 1)->_data);
        idBest = GAAdnGetId(ELORankGetRanked(eloRank, 1)->_data);
      } else {
        eloBest = ELORankGetELO(eloRank, 
          ELORankGetRanked(eloRank, 0)->_data);
        eloSoftBest = ELORankGetSoftELO(eloRank, 
          ELORankGetRanked(eloRank, 0)->_data);
        idBest = GAAdnGetId(ELORankGetRanked(eloRank, 0)->_data);
      }
      fprintf(stderr, "Epoch %05d/%05d Game %03d/%03d (ref %f[%f], bestelo(%ld) %f[%f])   \r", 
        iEpoch + 1, nbEpoch, iGame + 1, nbGameEpoch, eloRef, eloSoftRef, idBest, eloBest, eloSoftBest);
      fflush(stderr);
      // Select randomly one adn (including the best adn in GenAlg to play
      // against the reference, take care of who has the sente
      GenAlgAdn* adns[NBPLAYER] = {NULL};
      int iAdn = (int)round(rnd() * (float)(sizePool) - 1.0);
      
      iAdn = iAdnGlob;
      ++iAdnGlob;
      if (iAdnGlob == sizePool) iAdnGlob = -1;
      
      if (rnd() < 0.5) {
        adns[0] = (void*)1;
        if (iAdn == -1)
          adns[1] = (GenAlgAdn*)GABestAdn(genAlg);
        else 
          adns[1] = GSetGet(GAAdns(genAlg), iAdn);
      } else {
        adns[1] = (void*)1;
        if (iAdn == -1)
          adns[0] = (GenAlgAdn*)GABestAdn(genAlg);
        else 
          adns[0] = GSetGet(GAAdns(genAlg), iAdn);
      }
      // Select randomly two adns
      /*GenAlgAdn* adns[NBPLAYER] = {NULL};
      GSet setPlayers = GSetCreateStatic();
      GSetAddSort(&setPlayers, (void*)1, rnd());
      for (int iAdn = sizePool; iAdn--;)
        GSetAddSort(&setPlayers, GSetGet(GAAdns(genAlg), iAdn), rnd());
      while (GSetNbElem(&setPlayers) > 2)
        (void)GSetDrop(&setPlayers);
      adns[0] = GSetGet(&setPlayers, 0);
      adns[1] = GSetGet(&setPlayers, 1);
      GSetFlush(&setPlayers);*/
      // Force the ref to play only with the elite
      /*if (adns[0] == (void*)1) {
        int iElite = sizePool - (int)round(rnd() * (nbElite - 2)) - 1;
        adns[1] = GSetGet(GAAdns(genAlg), iElite);
      } else if (adns[1] == (void*)1) {
        int iElite = sizePool - (int)round(rnd() * (nbElite - 2)) - 1;
        adns[0] = GSetGet(GAAdns(genAlg), iElite);
      }*/
      // Play the game
      TrainOneGame(expansionTime, adns, &result);
      // Update the ELORank with the result
      ELORankUpdate(eloRank, &result);
      // Increment the current game
      ++iGame;
    }
    fprintf(stderr, "\n");
    fflush(stderr);
    // Update the values of each adn in the GenAlg with their ELORank
    for (int iAdn = 0; iAdn < sizePool; ++iAdn) {
      GenAlgAdn* adn = GSetGet(GAAdns(genAlg), iAdn);
      float elo = ELORankGetSoftELO(eloRank, adn);
      GASetAdnValue(genAlg, adn, elo);
    }
    // Update the value of the best adn too
    GenAlgAdn* bestAdn = (GenAlgAdn*)GABestAdn(genAlg);
    bestAdn->_val = ELORankGetSoftELO(eloRank, bestAdn);
    // Step the GenAlg
    GAStep(genAlg);
    // Display the elo of the best and the reference
    float eloSoftBest = GAAdnGetVal(bestAdn);
    float eloSoftRef = ELORankGetSoftELO(eloRank, (void*)1);
    printf("best(%ld): [%f](age %ld) ref: [%f](rank %d)\n", 
      GAAdnGetId(bestAdn), eloSoftBest, 
      GAAdnGetAge(bestAdn), eloSoftRef, ELORankGetRank(eloRank, 
      (void*)1));
    fflush(stdout);
    // Save the result
    fprintf(streamRes, "%ld %f %f %d\n", 
      GAGetCurEpoch(genAlg), eloSoftBest,
      eloSoftRef, ELORankGetRank(eloRank, (void*)1));
    fflush(streamRes);
    // Save the best NeuraNet to ./bestnn.txt
    NNSetBases(neuraNet, GAAdnAdnF(bestAdn));
    NNSetLinks(neuraNet, GAAdnAdnI(bestAdn));
    stream = fopen("./bestnn.txt", "w");
    if (stream == NULL) {
      printf("Couldn't open ./bestnn.txt");
      exit(1);
    }
    if (!NNSave(neuraNet, stream, true)) {
      printf("Couldn't open ./bestnn.txt");
      exit(1);
    }
    fclose(stream);
    // Save the GenAlg to ./bestga.txt
    stream = fopen("./bestga.txt", "w");
    if (stream == NULL) {
      printf("Couldn't open ./bestga.txt");
      exit(1);
    }
    if (!GASave(genAlg, stream, true)) {
      printf("Couldn't save ./bestga.txt");
      exit(1);
    }
    fclose(stream);
    // Reset the ELO of the non elite adn
    for (int iAdn = 0; iAdn < sizePool; ++iAdn) {
      GenAlgAdn* adn = GSetGet(GAAdns(genAlg), iAdn);
      int rank = ELORankGetRank(eloRank, adn);
      if (rank >= nbElite)
        ELORankResetELO(eloRank, adn);
    }
    // Increment the current epoch
    ++iEpoch;
  }
  // Free memory
  ELORankFree(&eloRank);
  // Free memory
  fclose(streamRes);
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
  float expansionTime = 100.0;
  // Declare a variable to memorize the number of epoch for training
  int nbEpoch = 50;
  // Declare variables to memorize the size of pool, number of elites,
  // number of game per epoch for training
  int nbElite = 5;
  int sizePool = nbElite * 4;
  int nbGameEpoch = sizePool * sizePool;
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

