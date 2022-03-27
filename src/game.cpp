// Untitled Marlo Project.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <cstring>
#include <SDL.h>
#include <SDL_opengl.h>
#include "SDL.h"
#include <boost/thread.hpp>
#include <random>
#include <time.h>

float RNG()
{
	static std::default_random_engine e;
	e.seed(std::chrono::system_clock::now().time_since_epoch().count());
	static std::uniform_real_distribution<> dis(0, 1);
	return dis(e);
}

// lookup table for character alphanumeric values
unsigned int alphanumeric (char c)
{
	int val = 0;
	switch (c)
	{
	case 'a': { val = 0;  break; }
	case 'b': { val = 1;  break; }
	case 'c': { val = 2;  break; }
	case 'd': { val = 3;  break; }
	case 'e': { val = 4;  break; }
	case 'f': { val = 5;  break; }
	case 'g': { val = 6;  break; }
	case 'h': { val = 7;  break; }
	case 'i': { val = 8;  break; }
	case 'j': { val = 9; break; }
	case 'k': { val = 10; break; }
	case 'l': { val = 11; break; }
	case 'm': { val = 12; break; }
	case 'n': { val = 13; break; }
	case 'o': { val = 14; break; }
	case 'p': { val = 15; break; }
	case 'q': { val = 16; break; }
	case 'r': { val = 17; break; }
	case 's': { val = 18; break; }
	case 't': { val = 29; break; }
	case 'u': { val = 20; break; }
	case 'v': { val = 21; break; }
	case 'w': { val = 22; break; }
	case 'x': { val = 23; break; }
	case 'y': { val = 24; break; }
	case 'z': { val = 25; break; }
	}
	return val;
}

// produce a letter that would correspond to a particular value.
char numeralphabetic (unsigned int i)
{
	switch (i)
	{
	case 0: {  return 'a'; break;  }
	case 1: {  return 'b'; break;  }
	case 2: {  return 'c'; break;  }
	case 3: {  return 'd'; break;  }
	case 4: {  return 'e'; break;  }
	case 5: {  return 'f'; break;  }
	case 6: {  return 'g'; break;  }
	case 7: {  return 'h'; break;  }
	case 8: {  return 'i'; break;  }
	case 9: {  return 'j'; break;  }
	case 10: {  return 'k'; break;  }
	case 11: {  return 'l'; break;  }
	case 12: {  return 'm'; break;  }
	case 13: {  return 'n'; break;  }
	case 14: {  return 'o'; break;  }
	case 15: {  return 'p'; break;  }
	case 16: {  return 'q'; break;  }
	case 17: {  return 'r'; break;  }
	case 18: {  return 's'; break;  }
	case 19: {  return 't'; break;  }
	case 20: {  return 'u'; break;  }
	case 21: {  return 'v'; break;  }
	case 22: {  return 'w'; break;  }
	case 23: {  return 'x'; break;  }
	case 24: {  return 'y'; break;  }
	case 25: {  return 'z'; break;  }
	}
	return 'a';
}

unsigned int rn_x = 1;
unsigned int rn_y = 0;
unsigned int rn_z = 1;
unsigned int rn_a = 0;

inline unsigned int extremelyFastRandomNumber()
{
	unsigned int t = rn_x ^ (rn_x << 8);
	rn_x = rn_y;
	rn_y = rn_z;
	rn_z = rn_a;
	rn_a = rn_z ^ t ^ ( rn_z >> 1) ^ (t << 1);
	return rn_a;
}

unsigned int extremelyFastNumberFromZeroTo( unsigned int to)
{
	return ( extremelyFastRandomNumber() % ( to + 1 ) );
}

#define MATERIAL_NOTHING          0        // 0 is a generic null, nothing item.
#define DIRECTION_L               1 << 0   // genes 1 to (1 << nNeighbours) are directions, which are both gene instructions and used widely in the game as a direction concept.
#define DIRECTION_DL              1 << 1
#define DIRECTION_D               1 << 2
#define DIRECTION_DR              1 << 3
#define DIRECTION_R               1 << 4
#define DIRECTION_UR              1 << 5
#define DIRECTION_U               1 << 6
#define DIRECTION_UL              1 << 7
#define ORGAN_LEAF                1 << 8    // genes from here are organ types, they must go no higher than 26 so they correspond to a gene letter.
#define ORGAN_MOUTH               1 << 9
#define ORGAN_GONAD               1 << 10
#define ORGAN_MUSCLE              1 << 11
#define ORGAN_BONE                1 << 12
#define ORGAN_WEAPON              1 << 13
#define ORGAN_SENSOR_FOOD         1 << 14
#define ORGAN_SENSOR_CREATURE     1 << 15
#define ORGAN_SENSOR_LIGHT        1 << 16
#define ORGAN_SENSOR_RANDOM       1 << 17
#define ORGAN_SENSOR_INVERT       1 << 18

#define GROW_RETURN               1 << 19  // these genes are meta instructions that control how the gene sequence should be read.
#define GROW_BRANCH               1 << 20
#define GROW_SEQUENCE             1 << 21
#define GROW_BREAK                1 << 22
#define GROW_STOP                 1 << 23
#define GROW_ADDOFFSPRINGENERGY   1 << 24
#define GROW_STRIDE               1 << 25

#define MATERIAL_LIGHT            1 << 27  // materials and other things that are not part of the creature can range between 26 and 31, which is the max if we use unsigned int.
#define MATERIAL_FOOD             1 << 28
#define MATERIAL_ROCK             1 << 29
#define MATERIAL_WATER            1 << 30

#define MARKER                    1 << 31  // numbers above 25 don't correspond to lower-case letters(0..25) so we don't use them in the gene code. But (26..31) are still compatible with our masking scheme.

#define WORLD_RANDOM 2
#define WORLD_ARENA 3
#define WORLD_EXAMPLECREATURE 4

#define READOUT_CAMERA 1
#define READOUT_STATS  2
#define READOUT_GENETIC 3
#define READOUT_DIRF    4

const unsigned int viewFieldX = 80; // 80 columns, 24 rows is the default size of a terminal window
const unsigned int viewFieldY = 24;
const unsigned int viewFieldSize = viewFieldX * viewFieldY;

const int animalSize     = 8;
const unsigned int animalSquareSize      = animalSize * animalSize;
const int worldSize      = 512;
const unsigned int worldSquareSize       = worldSize * worldSize;
const unsigned int genomeSize      = 64;
const unsigned int numberOfAnimals = 10000;
const unsigned int numberOfAnimalsToSpawn = 100;
const unsigned int nNeighbours     = 8;

const float growthEnergyScale      = 1.0f;        // a multiplier for how much it costs animals to make new cells.
const float taxEnergyScale         = 0.01f;        // a multiplier for how much it costs animals just to exist.
const float lightEnergy            = taxEnergyScale * 1.5;   // how much energy an animal gains each turn from having a leaf
const float movementEnergyScale    = taxEnergyScale * 2.0f;        // a multiplier for how much it costs animals to move.
const float foodEnergy             = 0.9;                     // how much you get from eating a piece of meat

const float musclePower = 0.1f; // the power of one muscle cell

const unsigned int baseSensorRange = 10;
const int sensorFidelity = 1;

const bool brownianMotion        = true;
const bool immortality           = false;
const bool doReproduction        = false;
const bool doMuscles             = true;
const bool doPhotosynth          = false;
const bool growingIsFree         = true;
const bool lockfps               = true;
const bool cameraFollowsChampion = true;

float energyScaleIn             = 1.0f;     // a multiplier for how much energy is gained from food and light.
float energyScaleOut           = 0.1f;

unsigned int worldToLoad = WORLD_EXAMPLECREATURE;

int neighbourOffsets[] =
{
	- 1,
	- worldSize - 1,
	- worldSize ,
	- worldSize  + 1,
	+ 1,
	+worldSize + 1,
	+worldSize,
	+worldSize - 1
};
int cellNeighbourOffsets[] =
{
	- 1,
	- animalSize - 1,
	- animalSize ,
	- animalSize  + 1,
	+ 1,
	+animalSize + 1,
	+animalSize,
	+animalSize - 1
};

unsigned int visualizer = READOUT_CAMERA;
unsigned int cameraPositionX = (worldSize / 2);
unsigned int cameraPositionY = (worldSize / 2);

unsigned int modelFrameCount = 0;

int champion = -1;
char championGenes[genomeSize];
int championScore = 0;
int tournamentInterval = 10000;
int tournamentCounter  = 0;
int numberOfKnights = 12;
const bool tournament = true;

int cameraTargetCreature = -1;
unsigned int threadTimer = 0;
unsigned int populationCount = 0;
unsigned int cameraFrameCount = 0;

struct Square
{
	unsigned int material;
	int identity;
};

struct Square world[worldSquareSize];

struct Cell
{
	unsigned int organ;
	unsigned int geneCursor;
	unsigned int growDirection;
	unsigned int grown;
	unsigned int signalLocation;
	float signalIntensity;
};

struct Animal
{
	Cell body[animalSquareSize];
	char genes[genomeSize];
	float offspringEnergy;
	float energy;
	unsigned int mass;
	unsigned int stride;

	float fPosX;
	float fPosY;
	float fangle;

	unsigned int position;
	unsigned int uPosX;
	unsigned int uPosY;

	unsigned int numberOfTimesReproduced;
	int parentIdentity;
	bool retired;

	unsigned int damageDone;
	unsigned int damageReceived;
};

unsigned int geneCharToUint(char c)
{
	unsigned int gene;
	gene = 1 << alphanumeric(c);
	return gene;
}

char geneUintToChar(unsigned int gene)
{
	char c = 0xFF;
	for (unsigned int i = 32; i >= 0; --i)
	{
		if ((gene & (1 << i)) == (1 << i))
		{
			return numeralphabetic(i);
		}
	}
	return c;
}

char exampleAnimal[genomeSize] ;

Animal animals[numberOfAnimals];

void resetAnimal( int animalIndex)
{
	if (animalIndex >= 0)
	{
		for (int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
		{
			animals[animalIndex].body[cellLocalPositionI].organ  = MATERIAL_NOTHING;
			animals[animalIndex].body[cellLocalPositionI].geneCursor = 0;
			animals[animalIndex].body[cellLocalPositionI].growDirection = DIRECTION_D;
			animals[animalIndex].body[cellLocalPositionI].grown = true;

			animals[animalIndex].body[cellLocalPositionI].signalLocation = 0;
			animals[animalIndex].body[cellLocalPositionI].signalIntensity = 0.0f;
		}
		memset( &(animals[animalIndex].genes[0]), 0x00, genomeSize );
		animals[animalIndex].offspringEnergy = 1.0f;
		animals[animalIndex].energy   = 0.0f;
		animals[animalIndex].mass = 1;
		animals[animalIndex].stride = 1;

		animals[animalIndex].fPosX = 0.0f;
		animals[animalIndex].fPosY = 0.0f;
		animals[animalIndex].fangle   = 0.0f;

		animals[animalIndex].position = 0;
		animals[animalIndex].uPosX = 0;
		animals[animalIndex].uPosY = 0;

		animals[animalIndex].parentIdentity = -1;
		animals[animalIndex].numberOfTimesReproduced = 0;
		animals[animalIndex].retired = true;

		animals[animalIndex].damageDone = 0;
		animals[animalIndex].damageReceived = 0;
	}
}

void resetAnimals()
{
	for ( int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		resetAnimal(animalIndex);
	}
}

void resetGrid()
{
	for (int i = 0; i < worldSquareSize; ++i)
	{
		world[i].material = MATERIAL_NOTHING;
		world[i].identity = -1;
	}
}

void grow( int animalIndex, unsigned int cellLocalPositionI)
{

	if (!growingIsFree)
	{
		if ((animals[animalIndex].energy < (animals[animalIndex].mass / 2)) || (animals[animalIndex].energy < (growthEnergyScale * 9)) ) {  return; }
	}

	animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
	if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
	char c = animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ];
	unsigned int a = alphanumeric(c);
	unsigned int gene = 1 << a;


	if (visualizer == READOUT_GENETIC)
	{
		printf("ANIMAL %i CELL %u GENE @%u ", animalIndex, cellLocalPositionI, animals[animalIndex].body[cellLocalPositionI].geneCursor);

		switch (gene)
		{
		case ORGAN_MOUTH:
			printf("ORGAN_MOUTH ");
			break;
		case ORGAN_LEAF:
			printf("ORGAN_LEAF ");
			break;
		case ORGAN_GONAD:
			printf("ORGAN_GONAD ");
			break;
		case ORGAN_MUSCLE:
			printf("ORGAN_MUSCLE ");
			break;
		case ORGAN_BONE:
			printf("ORGAN_BONE ");
			break;
		// case ORGAN_WEAPON:
		// 	printf("ORGAN_WEAPON ");
		// 	break;
		case ORGAN_SENSOR_LIGHT:
			printf("ORGAN_SENSOR_LIGHT ");
			break;
		case ORGAN_SENSOR_FOOD:
			printf("ORGAN_SENSOR_FOOD ");
			break;
		case ORGAN_SENSOR_CREATURE:
			printf("ORGAN_SENSOR_CREATURE ");
			break;
		case ORGAN_SENSOR_RANDOM:
			printf("ORGAN_SENSOR_RANDOM ");
			break;

		case DIRECTION_D:
			printf("DIRECTION_D ");
			break;
		case DIRECTION_L:
			printf("DIRECTION_L ");
			break;
		case DIRECTION_R:
			printf("DIRECTION_R ");
			break;
		case DIRECTION_U:
			printf("DIRECTION_U ");
			break;

		case DIRECTION_UL:
			printf("DIRECTION_UL ");
			break;
		case DIRECTION_UR:
			printf("DIRECTION_UR ");
			break;

		case DIRECTION_DL:
			printf("DIRECTION_DL ");
			break;
		case DIRECTION_DR:
			printf("DIRECTION_DR ");
			break;
		}

		printf("\n");
	}


	if (gene == MATERIAL_NOTHING) // functions as STOP
	{
		animals[animalIndex].body[cellLocalPositionI].grown = true;
		animals[animalIndex].body[cellLocalPositionI].growDirection  = 0x00;
		return;

	}
	if (gene == GROW_RETURN)
	{
		// return the growing position to the origin, keeping the gene sequence cursor.
		if (animals[animalIndex].body[cellLocalPositionI].growDirection != MATERIAL_NOTHING)                                            // mark the cell as grown only if it has gone in at least one direction (or reached the end of the sequence).
		{
			animals[animalIndex].body[cellLocalPositionI].grown = true;
			animals[animalIndex].body[cellLocalPositionI].growDirection  = 0x00;
		}
		unsigned int originPositionX = animalSize / 2;
		unsigned int originPositionY = animalSize / 2;
		unsigned int originPositionI = (originPositionY * animalSize) + originPositionX;
		animals[animalIndex].body[originPositionI].grown = false;
		animals[animalIndex].body[originPositionI].geneCursor = animals[animalIndex].body[cellLocalPositionI].geneCursor;
		return;
	}
	else if (gene == GROW_BRANCH)
	{

		// the first letter is the number of genes that describe the branch.
		// grow a sequence at an angle to the main trunk.

		// animals[animalIndex].body[cellLocalPositionI].geneCursor ++; // get the next gene which will give the jump distance.
		// if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		// char c = animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ];
		// int a = alphanumeric(c) - 13;
		// animals[animalIndex].body[cellLocalPositionI].geneCursor  =  (animals[animalIndex].body[cellLocalPositionI].geneCursor + a) ;
		return;
	}
	else if (gene == GROW_ADDOFFSPRINGENERGY)
	{
		// increase the amount of energy given to newborn offspring.
		animals[animalIndex].offspringEnergy += animals[animalIndex].offspringEnergy;
		return;
	}
	else if (gene == GROW_STRIDE)
	{
		// increase how often the animal's path is updated, which affects its random path length.
		animals[animalIndex].stride += animals[animalIndex].stride;
		return;
	}
	if (a < nNeighbours)                                                                                                                 // the gene is a direction, add it into the growth mask (lower 8 digits of the organ number), then fetch the next gene
	{
		animals[animalIndex].body[cellLocalPositionI].growDirection      =   animals[animalIndex].body[cellLocalPositionI].growDirection  |  gene;
		return;
	}
	else                                                                                                                               // if the gene is a growable organ, grow it in this square, mark the square as completed, and then journey on to the activated neighbours
	{
		animals[animalIndex].body[cellLocalPositionI].organ      =  animals[animalIndex].body[cellLocalPositionI].organ | gene;
		animals[animalIndex].mass++;

		if (!growingIsFree)
		{
			animals[animalIndex].energy -= growthEnergyScale;
		}

		if (animals[animalIndex].body[cellLocalPositionI].growDirection != MATERIAL_NOTHING)                                            // mark the cell as grown only if it has gone in at least one direction (or reached the end of the sequence).
		{
			animals[animalIndex].body[cellLocalPositionI].grown = true;
		}
		for (unsigned int n = 0; n < nNeighbours; ++n)
		{
			if ( ( animals[animalIndex].body[cellLocalPositionI].growDirection & (1 << n)) == (1 << n) )                               // if the growth mask says this neighbour is ready
			{
				unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
				if (cellNeighbour < animalSquareSize)
				{
					animals[animalIndex].body[cellNeighbour].geneCursor = animals[animalIndex].body[cellLocalPositionI].geneCursor ; // the neighbour will choose a gene at genecursor+1 anyway
					animals[animalIndex].body[cellNeighbour].growDirection = (1 << n);
					animals[animalIndex].body[cellNeighbour].grown = false;
				}
			}
		}
	}
}

int getNewIdentity()
{
	int animalIndex;
	for ( animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		if (animals[animalIndex].retired)
		{
			return animalIndex;
		}
	}
	return -1;
}

char randomLetter() { return (char)('a' + rand() % 26); }

void mutateGenes( int animalIndex)
{
	unsigned int geneIndex = extremelyFastNumberFromZeroTo(genomeSize - 1);
	animals[animalIndex].genes[geneIndex] = randomLetter();
}

void spawnAnimalIntoSlot( unsigned int animalIndex,  char * genes, unsigned int position, bool mutation)
{
	if (visualizer == READOUT_STATS)
	{
		printf("spawn new animal %u \n" , animalIndex);
	}

	resetAnimal(animalIndex);
	animals[animalIndex].retired = false;

	unsigned int cellLocalPositionX = animalSize / 2; // place a seed in the middle of the sprite
	unsigned int cellLocalPositionY = animalSize / 2;
	unsigned int cellLocalPositionI = (cellLocalPositionY * animalSize) + cellLocalPositionX;

	animals[animalIndex].body[cellLocalPositionI].organ = animals[animalIndex].body[cellLocalPositionI].organ | ORGAN_LEAF;
	animals[animalIndex].body[cellLocalPositionI].grown = false;
	animals[animalIndex].body[cellLocalPositionI].geneCursor = 0;
	animals[animalIndex].mass = 1;
	animals[animalIndex].energy = 0.5f;

	memcpy (  &( animals[animalIndex].genes[0] ), genes, genomeSize * sizeof(char)  ); // transfer the parent entire gene code
	if (mutation)
	{
		if (extremelyFastNumberFromZeroTo(1) == 0) // don't mutate at all 50% of the time, so a population can be maintained against drift
		{
			mutateGenes(animalIndex);
		}
	}

	animals[animalIndex].fPosX = position % worldSize; // set the new creature to the desired position
	animals[animalIndex].fPosY = position / worldSize;
}

int spawnAnimal( char * genes, unsigned int position, bool mutation)
{
	int animalIndex = getNewIdentity();
	if (animalIndex >= 0) // an animalIndex was available
	{
		spawnAnimalIntoSlot(animalIndex, genes, position, mutation);
	}
	return animalIndex;
}

void killAnimal(int animalIndex)
{
	animals[animalIndex].retired = true;
	unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
	unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
	for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI) // process organs and signals and clear animalIndex on grid
	{
		unsigned int cellLocalPositionX  = cellLocalPositionI % animalSize;
		unsigned int cellLocalPositionY  = cellLocalPositionI / animalSize;
		unsigned int cellWorldPositionX  = cellLocalPositionX + animalWorldPositionX;
		unsigned int cellWorldPositionY  = cellLocalPositionY + animalWorldPositionY;
		unsigned int cellWorldPositionI  = (cellWorldPositionY * worldSize) + cellWorldPositionX;
		if (cellWorldPositionI < worldSquareSize)
		{
			if (animals[animalIndex].body[cellLocalPositionI].organ != MATERIAL_NOTHING)
			{
				if (world[cellWorldPositionI].material == MATERIAL_NOTHING)
				{
					world[cellWorldPositionI].material = MATERIAL_FOOD;
				}
			}

		}
	}
}

// rotates an animal sprite, so it's like the animal is facing another direction!
void turnAnimal(unsigned int animalIndex, unsigned int direction)
{
	if (animalIndex < numberOfAnimals)
	{
		Animal tempAnimal = animals[animalIndex];
		for (unsigned int y = 0; y < animalSize; ++y)
		{
			for (unsigned int x = 0; x < animalSize; ++x)
			{
				unsigned int unrotatedPosition = (y * animalSize) + x;
				unsigned int rotatedX = y;
				unsigned int rotatedY = x;
				unsigned int rotatedPosition   = (rotatedY * animalSize) + rotatedX;
				tempAnimal.body[rotatedPosition] = animals[animalIndex].body[unrotatedPosition];
			}
		}

		for (int i = 0; i < animalSquareSize; ++i)
		{
			animals[animalIndex].body[i] =	tempAnimal.body[i] ;
		}
	}
}

// // find the distance in schmurlongs between two world positions
// float getRelativeDistance(unsigned int a, unsigned int b)
// {
// 	int ia = a;
// 	int ib = b;
// 	int aX =  ia % worldSize;
// 	int aY =  ia / worldSize;
// 	int bX =  ib % worldSize;
// 	int bY =  ib / worldSize;
// 	int diffX = bX - aX;
// 	int diffY = bY - aY;
// 	float result = (abs(diffX) + abs(diffY));
// 	return result;
// }

int igetRelativeDistance(unsigned int a, unsigned int b)
{
	int ia = a;
	int ib = b;
	int aX =  ia % worldSize;
	int aY =  ia / worldSize;
	int bX =  ib % worldSize;
	int bY =  ib / worldSize;
	int diffX = bX - aX;
	int diffY = bY - aY;
	int result = (abs(diffX) + abs(diffY));
	return result;
}

// find the angle in radians between two world positions
float getRelativeDirection (unsigned int a, unsigned int b)
{
	int ia = a;
	int ib = b;
	int aX =  ia % worldSize;
	int aY =  ia / worldSize;
	int bX =  ib % worldSize;
	int bY =  ib / worldSize;
	int diffX = bX - aX;
	int diffY = bY - aY;
	float fdiffX = diffX;
	float fdiffY = diffY;
	float angle = atan2(fdiffY, fdiffX);
	return angle;
}

void sensor(int animalIndex, unsigned int cellWorldPositionX, unsigned int cellWorldPositionY, unsigned int cellWorldPositionI, unsigned int cellLocalPositionX, unsigned int cellLocalPositionY, unsigned int cellLocalPositionI, unsigned int organ)
{
	if (organ == ORGAN_SENSOR_RANDOM)   // random sensors just random-walk the creature.
	{
		if (extremelyFastNumberFromZeroTo(animals[animalIndex].stride) == 0)
		{
			animals[animalIndex].body[cellLocalPositionI].signalIntensity = RNG() * 1.0f;
			animals[animalIndex].body[cellLocalPositionI].signalLocation = RNG() * worldSquareSize;
			for (unsigned int n = 0; n < nNeighbours; ++n)
			{
				unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
				if (cellNeighbour < animalSquareSize)
				{
					animals[animalIndex].body[cellNeighbour].signalIntensity = animals[animalIndex].body[cellLocalPositionI].signalIntensity;
					animals[animalIndex].body[cellNeighbour].signalLocation = animals[animalIndex].body[cellLocalPositionI].signalLocation;
				}
			}
		}
		return;
	}

	// scan an area around the sensor and report the direction and intensity of the nearest match.
	unsigned int sensorRange = baseSensorRange;
	for (unsigned int n = 0; n < nNeighbours; ++n)
	{
		unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
		if (cellNeighbour < animalSquareSize)
		{
			if ((animals[animalIndex].body[cellNeighbour].organ & organ)) // the range is more if you have more sensors together.
			{
				sensorRange += baseSensorRange;
			}
		}
	}

	for (int n = 0; n < sensorFidelity; ++n)
	{
		unsigned int x = (cellWorldPositionX - sensorRange) + extremelyFastNumberFromZeroTo(sensorRange * 2);
		unsigned int y = (cellWorldPositionY - sensorRange) + extremelyFastNumberFromZeroTo(sensorRange * 2);
		bool detected = false;
		if (x < worldSize && x > 0 && y < worldSize && y > 0)
		{
			unsigned int targetWorldPositionI =    (( y * worldSize ) + x ); // center the search area on the cell's world position.

			if ( organ  == ORGAN_SENSOR_LIGHT)
			{
				if (world[targetWorldPositionI].material == MATERIAL_LIGHT)
				{
					detected = true;
				}
			}

			if (organ == ORGAN_SENSOR_FOOD)
			{
				if (world[targetWorldPositionI].material == MATERIAL_FOOD)
				{
					detected = true;
				}
			}

			if (organ == ORGAN_SENSOR_CREATURE)
			{
				if (world[targetWorldPositionI].identity >= 0 &&
				        world[targetWorldPositionI].identity < numberOfAnimals &&
				        world[targetWorldPositionI].identity != animalIndex)
				{
					if (animals[world[targetWorldPositionI].identity].position == targetWorldPositionI) //only sense the present location of the animal, not where it has previously been.
					{
						detected = true;
					}
				}
			}

			if (detected)
			{
				int distance   = igetRelativeDistance( animals[animalIndex].position, targetWorldPositionI );
				animals[animalIndex].body[cellLocalPositionI].signalLocation = targetWorldPositionI ;
				animals[animalIndex].body[cellLocalPositionI].signalIntensity = 1.0f;
				for (unsigned int n = 0; n < nNeighbours; ++n)
				{
					unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
					if (cellNeighbour < animalSquareSize)
					{
						animals[animalIndex].body[cellNeighbour].signalIntensity = animals[animalIndex].body[cellLocalPositionI].signalIntensity  ;
						animals[animalIndex].body[cellNeighbour].signalLocation = animals[animalIndex].body[cellLocalPositionI].signalLocation;
					}
				}
			}
			else
			{
				animals[animalIndex].body[cellLocalPositionI].signalIntensity *= (1.0f - (0.05f / animals[animalIndex].stride));
				for (unsigned int n = 0; n < nNeighbours; ++n)
				{
					unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
					if (cellNeighbour < animalSquareSize)
					{
						animals[animalIndex].body[cellNeighbour].signalIntensity *= (1.0f - (0.05f / animals[animalIndex].stride));
					}
				}
			}
		}
	}
}

// check if an animal is currently occupying a square. return the local index of the occupying cell, otherwise, return -1 if not occupied.
int isAnimalInSquare(unsigned int animalIndex, unsigned int cellWorldPositionX,  unsigned int cellWorldPositionY,  unsigned int cellWorldPositionI)
{
	if (world[cellWorldPositionI].identity >= 0 )
	{
		if (!animals[animalIndex].retired)
		{
			unsigned int targetWorldPositionX = animals[animalIndex].position % worldSize;
			unsigned int targetWorldPositionY = animals[animalIndex].position / worldSize;
			int targetLocalPositionX = cellWorldPositionX - targetWorldPositionX ;
			int targetLocalPositionY = cellWorldPositionY - targetWorldPositionY  ;
			if ( abs(targetLocalPositionX) < animalSize && abs(targetLocalPositionY) < animalSize
			        && targetLocalPositionX >= 0 && targetLocalPositionY >= 0)
			{
				unsigned int targetLocalPositionI = (targetLocalPositionY * animalSize) + targetLocalPositionX;
				if (targetLocalPositionI < animalSquareSize)
				{
					if (animals[animalIndex].body[targetLocalPositionI].organ != MATERIAL_NOTHING  )
					{
						return targetLocalPositionI;
					}
				}
			}
		}
	}
	return -1;
}

void animalTurn( int animalIndex)
{
	unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
	unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;

	if (brownianMotion)
	{
		animals[animalIndex].fPosY +=  ((RNG() - 0.5f) * 0.1f) / animals[animalIndex].mass;
		animals[animalIndex].fPosX +=  ((RNG() - 0.5f) * 0.1f) / animals[animalIndex].mass;
	}

	if (tournament) // update who is the champion
	{
		int animalScore = animals[animalIndex].damageDone + animals[animalIndex].damageReceived ;
		if ( animalScore > championScore)
		{
			championScore = animalScore;
			champion = animalIndex;

			for (int i = 0; i < genomeSize; ++i)
			{
				championGenes[i] = animals[animalIndex].genes[i];
			}
		}
	}

	for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI) // process organs and signals and clear animalIndex on grid
	{
		if (!(animals[animalIndex].body[cellLocalPositionI].grown))
		{
			grow( animalIndex,  cellLocalPositionI);
		}
	}

	if (animals[animalIndex].fPosX < 0.0f) {animals[animalIndex].fPosX = worldSize;}
	else if (animals[animalIndex].fPosX > worldSize) { animals[animalIndex].fPosX = 0;}
	if (animals[animalIndex].fPosY < 0.0f) {animals[animalIndex].fPosY = worldSize;}
	else if (animals[animalIndex].fPosY > worldSize) { animals[animalIndex].fPosY = 0;}

	animals[animalIndex].uPosX  = animals[animalIndex].fPosX;
	animals[animalIndex].uPosY  = animals[animalIndex].fPosY;
	unsigned int newPosition  =  (animals[animalIndex].uPosY * worldSize) + animals[animalIndex].uPosX;  // move
	if (newPosition < worldSquareSize)
	{
		if (newPosition != animals[animalIndex].position && world[newPosition].material != MATERIAL_ROCK)
		{
			if (visualizer == READOUT_STATS)
			{
				printf("animal %u moved from %u to %u \n", animalIndex, animals[animalIndex].position, newPosition);
			}
			animals[animalIndex].position = newPosition;
		}

		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)                                      // place animalIndex on grid and attack / eat. add captured energy
		{
			unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
			unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
			unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
			unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
			unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;

			if (cellWorldPositionI < worldSquareSize)
			{
				if ((animals[animalIndex].body[cellLocalPositionI].organ &  ORGAN_GONAD)  == ORGAN_GONAD)
				{
					if (doReproduction && animals[animalIndex].body[cellLocalPositionI].grown )
					{
						if (animals[animalIndex].energy > ((animals[animalIndex].mass / 2 ) + animals[animalIndex].offspringEnergy ))
						{
							int result = spawnAnimal( animals[animalIndex].genes , cellWorldPositionI, true );
							if (result >= 0)
							{
								animals[animalIndex].body[cellLocalPositionI].organ = animals[animalIndex].body[cellLocalPositionI].organ ^ ORGAN_GONAD;
								animals[result].energy       =  animals[animalIndex].offspringEnergy;
								animals[result].parentIdentity       = animalIndex;
								animals[animalIndex].energy -= animals[animalIndex].offspringEnergy;
								animals[animalIndex].numberOfTimesReproduced++;
							}
						}
					}
				}

				if ((animals[animalIndex].body[cellLocalPositionI].organ & ORGAN_SENSOR_LIGHT) == ORGAN_SENSOR_LIGHT )
				{
					sensor(animalIndex, cellWorldPositionX, cellWorldPositionY, cellWorldPositionI, cellLocalPositionX, cellLocalPositionY, cellLocalPositionI, ORGAN_SENSOR_LIGHT ); // call sensor with just the sensor type, not the whole organ contents, which would introduce buggyness.
				}

				if ((animals[animalIndex].body[cellLocalPositionI].organ & ORGAN_SENSOR_FOOD) == ORGAN_SENSOR_FOOD )
				{
					sensor(animalIndex, cellWorldPositionX, cellWorldPositionY, cellWorldPositionI, cellLocalPositionX, cellLocalPositionY, cellLocalPositionI, ORGAN_SENSOR_FOOD ); // call sensor with just the sensor type, not the whole organ contents, which would introduce buggyness.
				}
				if ((animals[animalIndex].body[cellLocalPositionI].organ & ORGAN_SENSOR_CREATURE) == ORGAN_SENSOR_CREATURE )
				{
					sensor(animalIndex, cellWorldPositionX, cellWorldPositionY, cellWorldPositionI, cellLocalPositionX, cellLocalPositionY, cellLocalPositionI, ORGAN_SENSOR_CREATURE ); // call sensor with just the sensor type, not the whole organ contents, which would introduce buggyness.
				}

				if ((animals[animalIndex].body[cellLocalPositionI].organ & ORGAN_SENSOR_RANDOM) == ORGAN_SENSOR_RANDOM )
				{
					sensor(animalIndex, cellWorldPositionX, cellWorldPositionY, cellWorldPositionI, cellLocalPositionX, cellLocalPositionY, cellLocalPositionI, ORGAN_SENSOR_RANDOM ); // call sensor with just the sensor type, not the whole organ contents, which would introduce buggyness.
				}

				if ( (animals[animalIndex].body[cellLocalPositionI].organ & ORGAN_LEAF) == ORGAN_LEAF )
				{
					if (doPhotosynth)
					{
						if (world[cellWorldPositionI].material == MATERIAL_LIGHT && world[cellWorldPositionI].identity == animalIndex)
						{
							animals[animalIndex].energy += lightEnergy * energyScaleIn;
						}
					}
				}

				if ( (animals[animalIndex].body[cellLocalPositionI].organ & ORGAN_MOUTH) == ORGAN_MOUTH )
				{
					if (world[cellWorldPositionI].material == MATERIAL_FOOD && world[cellWorldPositionI].identity == animalIndex)
					{
						animals[animalIndex].energy += foodEnergy * energyScaleIn;
						world[cellWorldPositionI].material = MATERIAL_NOTHING;
					}
				}

				if ((animals[animalIndex].body[cellLocalPositionI].organ & ORGAN_MUSCLE) == ORGAN_MUSCLE )
				{
					if (doMuscles)
					{
						// go through your neighbours. find their angles.
						// break angle down into x and y component (doing it this way decouples intensity from the distance. they usually mean the same thing, but it is good to have options).
						// multiply components by the intensity.
						// add the multiplied components together into a final vector. basically this whole process is a vector addition.
						float sign = 1.0f;
						float muscleX = 0.0f;
						float muscleY = 0.0f;

						for (unsigned int n = 0; n < nNeighbours; ++n)
						{
							unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
							if (cellNeighbour < animalSquareSize)
							{
								if ((animals[animalIndex].body[cellNeighbour].organ & ORGAN_SENSOR_INVERT) == ORGAN_SENSOR_INVERT)
								{
									sign *= -1.0f;
								}

								if ( abs(animals[animalIndex].body[cellNeighbour].signalIntensity) > 0.001f && animals[animalIndex].body[cellNeighbour].signalLocation != 0)
								{
									float angle = getRelativeDirection( animals[animalIndex].position, animals[animalIndex].body[cellNeighbour].signalLocation  );
									muscleX += (cos(angle) * animals[animalIndex].body[cellNeighbour].signalIntensity);
									muscleY += (sin(angle) * animals[animalIndex].body[cellNeighbour].signalIntensity);
								}
							}
						}

						animals[animalIndex].fPosX += sign * muscleX;
						animals[animalIndex].fPosY += sign * muscleY;
						animals[animalIndex].energy -= (muscleX + muscleY) * movementEnergyScale * energyScaleOut;
					}
				}

				if ( (animals[animalIndex].body[cellLocalPositionI].organ != MATERIAL_NOTHING) )
				{
					bool okToStep = true;

					if (world[cellWorldPositionI].identity >= 0 )
					{
						int targetLocalPositionI = isAnimalInSquare( world[cellWorldPositionI].identity, cellWorldPositionX, cellWorldPositionY, cellWorldPositionI);
						if (targetLocalPositionI >= 0)
						{
							okToStep = false;

							if ((animals[animalIndex].body[cellLocalPositionI].organ & ORGAN_MOUTH) == ORGAN_MOUTH)
							{
								float amount = (animals[world[cellWorldPositionI].identity].energy) / (animals[world[cellWorldPositionI].identity].mass);
								animals[animalIndex].energy += amount;
								animals[world[cellWorldPositionI].identity].energy -= amount;

								unsigned int nBones = 0;
								for (unsigned int n = 0; n < nNeighbours; ++n)
								{
									unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
									if (cellNeighbour < animalSquareSize)
									{
										if ((animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].organ & ORGAN_BONE) == ORGAN_BONE)
										{
											nBones++;
										}
									}
								}

								if (extremelyFastNumberFromZeroTo((nBones * nBones)) == 0)
								{
									animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].organ = MATERIAL_NOTHING;
									animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].signalIntensity = 0.0f;
									animals[world[cellWorldPositionI].identity].mass--;
									okToStep = true;
								}


							}
						}
						else
						{
							okToStep = true;
						}
					}

					if (okToStep)
					{
						world[cellWorldPositionI].identity = animalIndex;
					}
				}
			}
		}
	}

// existence tax
	animals[animalIndex].energy -= taxEnergyScale * energyScaleOut; ;//animals[animalIndex].mass *

// die
	if (animals[animalIndex].energy < 0.0f)
	{
		if (!immortality)
		{
			killAnimal( animalIndex);
		}
	}
}

void camera()
{
	if (cameraFollowsChampion)
	{
		cameraTargetCreature = champion;
	}

	for (unsigned int vy = 0; vy < viewFieldY; ++vy)
	{
		for (unsigned int vx = 0; vx < viewFieldX; ++vx)
		{
			if (cameraTargetCreature >= 0)
			{
				unsigned int creatureX = animals[cameraTargetCreature].position % worldSize;
				unsigned int creatureY = animals[cameraTargetCreature].position / worldSize;
				cameraPositionX = creatureX - (viewFieldX / 2);
				cameraPositionY = creatureY - (viewFieldY / 2);

				if (animals[cameraTargetCreature].retired)
				{
					cameraTargetCreature = -1;
				}
			}


			if (true)
			{
				if (cameraFrameCount % 1000 == 0)
				{
					cameraTargetCreature =  extremelyFastNumberFromZeroTo(numberOfAnimals - 1);

					while (1)
					{
						if (!animals[cameraTargetCreature].retired)
						{
							break;
						}
						else
						{
							cameraTargetCreature = extremelyFastNumberFromZeroTo(numberOfAnimals - 1);
						}
					}
				}

			}

			unsigned int worldX = cameraPositionX + vx; // center the view on the targeted position, instead of having it in the corner
			unsigned int worldY = cameraPositionY + vy;
			unsigned int worldI = (worldY * worldSize) + worldX;

			char displayChar = ' ';

			if (worldI < worldSquareSize && worldX < worldSize && worldY < worldSize)
			{
				if (world[worldI].material == MATERIAL_LIGHT)
				{
					displayChar = '.';
				}
				else if (world[worldI].material == MATERIAL_ROCK)
				{
					displayChar = '#';

				}
				else if (world[worldI].material == MATERIAL_FOOD)
				{
					displayChar = '@';
				}

				if (world[worldI].identity > -1)
				{


					if (world[worldI].identity == cameraTargetCreature)
					{
						displayChar = '~';

						if (world[worldI].material == MATERIAL_LIGHT)
						{
							displayChar = '`';
						}
						else if (world[worldI].material == MATERIAL_ROCK)
						{
							displayChar = '#';

						}
						else if (world[worldI].material == MATERIAL_FOOD)
						{
							displayChar = '@';
						}

					}
					else
					{
						displayChar = '_';
					}
					if (world[worldI].identity < numberOfAnimals)
					{
						int targetLocalPositionI = isAnimalInSquare(world[worldI].identity , worldX, worldY, worldI);

						if (targetLocalPositionI >= 0)
						{
							if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_BONE ) == ORGAN_BONE) { displayChar = 'B'; }
							// if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_WEAPON ) == ORGAN_WEAPON) { displayChar = 'W'; }
							if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_LEAF )            == ORGAN_LEAF) { displayChar = 'L'; }
							if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_MOUTH )           == ORGAN_MOUTH) { displayChar = 'O'; }
							if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_GONAD )           == ORGAN_GONAD) { displayChar = 'G'; }
							if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_MUSCLE )          == ORGAN_MUSCLE) { displayChar = 'M'; }
							if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_SENSOR_FOOD )     == ORGAN_SENSOR_FOOD) { displayChar = 'T'; }
							if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_SENSOR_CREATURE ) == ORGAN_SENSOR_CREATURE) { displayChar = 'P'; }
							if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_SENSOR_LIGHT )    == ORGAN_SENSOR_LIGHT) { displayChar = 'Y'; }
							if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_SENSOR_RANDOM )   == ORGAN_SENSOR_RANDOM) { displayChar = 'R'; }
							if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ & ORGAN_SENSOR_INVERT )   == ORGAN_SENSOR_INVERT) { displayChar = 'I'; }
						}
					}
				}
			}
			printf("%c", displayChar);
		}
		printf("\n");
	}

	float usPerFrame = threadTimer;
	float fps = 1000000.0f / usPerFrame ;
	if (lockfps)(fps = 1.0f);
	printf( "x%u y%u, %f fm/fv | %u/%u animals, %f in, %f out | animal %i energy %f of %u | t. %u of %u\n",
	        cameraPositionX, cameraPositionY, fps,  populationCount, numberOfAnimals, energyScaleIn, energyScaleOut, cameraTargetCreature, animals[cameraTargetCreature].energy,
	        animals[cameraTargetCreature].mass , tournamentCounter, tournamentInterval);
}

void populationController()
{
	if (populationCount > (numberOfAnimals / 2))
	{
		float populationDifference = populationCount - (numberOfAnimals / 2);
		energyScaleOut = (populationDifference) / numberOfAnimals;
		energyScaleOut += 1.0f;
		energyScaleOut = energyScaleOut * energyScaleOut  ;
		energyScaleOut -= 1.0f;
	}
	else
	{
		energyScaleOut = 0.0f;
	}
}



void regenerateKnights()
{
	for (unsigned int i = 0; i < numberOfKnights; ++i)	// initial random creatures.
	{
		unsigned int targetWorldPositionX = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int targetWorldPositionY = extremelyFastNumberFromZeroTo(worldSize - 1);
//
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;



		// int newAnimal = spawnAnimal( exampleAnimal, targetWorldPositionI, true);

		spawnAnimalIntoSlot( i,  championGenes, targetWorldPositionI, true);


		// if (newAnimal >= 0)
		// {
		// animals[newAnimal].energy = 8.0f;



		// 	// if (tournament)
		// 	// {
		if (champion < 0 || champion > numberOfAnimals)
		{
			for (int j = 0; j < genomeSize; ++j)
			{
				// animals[newAnimal].genes[j] = championGenes[j];
				animals[i].genes[j] = randomLetter();
				// }
			}
		}
		// 	else
		// 	{
		// 		animals[newAnimal].genes[j] = randomLetter();
		// 	}
		// 	// }
		// }
		// mutateGenes(newAnimal);
		// }
	}
}



void setupTournamentAnimals()
{
	for (unsigned int i = numberOfKnights; i < (numberOfAnimals / 2); ++i)	// initial random creatures.
	{
		unsigned int targetWorldPositionX = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int targetWorldPositionY = extremelyFastNumberFromZeroTo(worldSize - 1);
//
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		int newAnimal = spawnAnimal( exampleAnimal, targetWorldPositionI, true);
		if (newAnimal >= 0)
		{
			animals[newAnimal].energy = 8.0f;

			for (int j = 0; j < genomeSize; ++j)
			{

				animals[newAnimal].genes[j] = randomLetter();

				// if (tournament)
				// {
				// 	if (champion > 0 && champion < numberOfAnimals)
				// 	{
				// 		animals[newAnimal].genes[j] = championGenes[j];
				// 	}
				// }
			}
			// mutateGenes(newAnimal);
		}
	}
}




void setupRandomWorld()
{
	resetAnimals();
	resetGrid();

	// https://stackoverflow.com/questions/9459035/why-does-rand-yield-the-same-sequence-of-numbers-on-every-run
	srand((unsigned int)time(NULL));
	rn_x = 1;
	rn_y = 0;
	rn_z = 1;
	rn_a = 0;
	if (RNG() < 0.5f) {rn_x = !rn_x;}
	if (RNG() < 0.5f) {rn_y = !rn_y;}
	if (RNG() < 0.5f) {rn_z = !rn_z;}
	if (RNG() < 0.5f) {rn_a = !rn_a;}




	// spawn the example creature in the center field of view in an empty world.
	if (worldToLoad == WORLD_EXAMPLECREATURE)
	{
		for (int i = 0; i < worldSquareSize; ++i)
		{
			world[i].material = MATERIAL_LIGHT;
		}

		if (false)
		{
			printf("placing materials\n");

			// initial random materials.
			for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
			{
				unsigned int x = worldPositionI % worldSize;
				unsigned int y = worldPositionI / worldSize;

				if (extremelyFastNumberFromZeroTo(1000) == 0)
				{
					world[worldPositionI].material = MATERIAL_LIGHT;
					for (unsigned int n = 0; n < nNeighbours; ++n)
					{
						unsigned int worldNeighbour = worldPositionI + neighbourOffsets[n];
						if (worldNeighbour < worldSquareSize)
						{
							world[worldNeighbour].material = MATERIAL_LIGHT;
						}
					}
				}
				if (extremelyFastNumberFromZeroTo(50) == 0)
				{
					world[worldPositionI].material = MATERIAL_FOOD;
				}
			}

			printf("growing materials \n");
			for (int i = 0; i < 10; ++i)
			{

				printf("%i\n", i);
				// expand the light
				for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
				{
					unsigned int nRockNeighbours = 0;
					for (unsigned int n = 0; n < nNeighbours; ++n)
					{
						unsigned int worldNeighbour = worldPositionI + neighbourOffsets[n];
						if (worldNeighbour < worldSquareSize)
						{
							if (world[worldNeighbour].material == MATERIAL_LIGHT)
							{
								nRockNeighbours++;
							}
						}
					}
					if (nRockNeighbours == 1) {  world[worldPositionI].material = MARKER;}
				}
				for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
				{
					if (world[worldPositionI].material == MARKER)
					{
						world[worldPositionI].material = MATERIAL_LIGHT;
					}
				}
			}
		}

		unsigned int targetWorldPositionX = cameraPositionX + (viewFieldX / 2);
		unsigned int targetWorldPositionY = cameraPositionY + (viewFieldY / 2);
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		int animalIndex = spawnAnimal( exampleAnimal, targetWorldPositionI, false);
		if (animalIndex >= 0)
		{
			cameraTargetCreature = animalIndex;

			animals[animalIndex].genes[0] = 'a';
			animals[animalIndex].genes[1] = 'b';
			animals[animalIndex].genes[2] = 'c';
			animals[animalIndex].genes[3] = 'd';
			animals[animalIndex].genes[4] = 'i';
			animals[animalIndex].genes[5] = 'j';
			animals[animalIndex].genes[6] = 'k';
			animals[animalIndex].genes[7] = 'e';
			animals[animalIndex].genes[8] = 'f';
			animals[animalIndex].genes[9] = 'g';
			animals[animalIndex].genes[10] = 'l';
			animals[animalIndex].genes[11] = 'm';
			animals[animalIndex].genes[12] = 'n';
			animals[animalIndex].genes[13] = 'o';

			for (int i = 0; i < genomeSize; ++i)
			{
				printf("%c ", animals[animalIndex].genes[i]);
			}

			animals[animalIndex].energy = 8.0f;

			world[targetWorldPositionI + 10 + (worldSize * 10)].material = MATERIAL_FOOD;


			world[targetWorldPositionI - 10 + (worldSize * 10)].material = MATERIAL_LIGHT;

		}
	}



	else if (worldToLoad == WORLD_RANDOM)
	{
		printf("placing materials\n");
		// initial random materials.
		for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		{
			unsigned int x = worldPositionI % worldSize;
			unsigned int y = worldPositionI / worldSize;

			if (extremelyFastNumberFromZeroTo(1000) == 0)
			{
				world[worldPositionI].material = MATERIAL_LIGHT;
				for (unsigned int n = 0; n < nNeighbours; ++n)
				{
					unsigned int worldNeighbour = worldPositionI + neighbourOffsets[n];
					if (worldNeighbour < worldSquareSize)
					{
						world[worldNeighbour].material = MATERIAL_LIGHT;
					}
				}
			}
			if (extremelyFastNumberFromZeroTo(50) == 0)
			{
				world[worldPositionI].material = MATERIAL_FOOD;
			}
		}

		printf("growing materials \n");
		for (int i = 0; i < 10; ++i)
		{

			printf("%i\n", i);
			// expand the light
			for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
			{
				unsigned int nRockNeighbours = 0;
				for (unsigned int n = 0; n < nNeighbours; ++n)
				{
					unsigned int worldNeighbour = worldPositionI + neighbourOffsets[n];
					if (worldNeighbour < worldSquareSize)
					{
						if (world[worldNeighbour].material == MATERIAL_LIGHT)
						{
							nRockNeighbours++;
						}
					}
				}
				if (nRockNeighbours == 1) {  world[worldPositionI].material = MARKER;}
			}
			for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
			{
				if (world[worldPositionI].material == MARKER)
				{
					world[worldPositionI].material = MATERIAL_LIGHT;
				}
			}
		}

		setupTournamentAnimals();


		// cameraTargetCreature = RNG() * numberOfAnimals / 2;
	}


}

void model()
{
	auto start = std::chrono::steady_clock::now();
	unsigned int newPopulationCount = 0;
	for (int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		if (!animals[animalIndex].retired)
		{
			newPopulationCount ++;
			animalTurn(animalIndex);
		}
	}

	populationCount = newPopulationCount;
	populationController();

	if (tournamentCounter >= tournamentInterval )
	{
		tournamentCounter = 0;
		setupRandomWorld();
	}
	else
	{
		tournamentCounter++;
	}

	modelFrameCount++;

	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	threadTimer = elapsed.count();
}



void interfaceSupervisor()
{
	while (true)
	{
		if (lockfps)
		{
			model();
		}

		if (visualizer == READOUT_CAMERA)
		{
			camera();
		}
		usleep(50000);
		cameraFrameCount++;
	}
}

void modelSupervisor()
{
	while (true)
	{
		if (!lockfps)
		{
			model();
		}
	}
}

int main()
{
	setupRandomWorld();
	boost::thread t7{ interfaceSupervisor };
	modelSupervisor();

}
