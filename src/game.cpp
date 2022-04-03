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

#include <Tracy.hpp>

float RNG()
{
	ZoneScoped;
	static std::default_random_engine e;
	e.seed(std::chrono::system_clock::now().time_since_epoch().count());
	static std::uniform_real_distribution<> dis(0, 1);
	return dis(e);
}
unsigned int rn_x = 1;
unsigned int rn_y = 0;
unsigned int rn_z = 1;
unsigned int rn_a = 0;

inline unsigned int extremelyFastRandomNumber()
{
	ZoneScoped;
	unsigned int t = rn_x ^ (rn_x << 8);
	rn_x = rn_y;
	rn_y = rn_z;
	rn_z = rn_a;
	rn_a = rn_z ^ t ^ ( rn_z >> 1) ^ (t << 1);
	return rn_a;
}

inline unsigned int extremelyFastNumberFromZeroTo( unsigned int to)
{
	ZoneScoped;
	return ( extremelyFastRandomNumber() % ( to + 1 ) );
}

#define DIRECTION_L               0  // A  // genes 1 to (1 << nNeighbours) are directions, which are both gene instructions and used widely in the game as a direction concept.
#define DIRECTION_DL              1  // B 
#define DIRECTION_D               2  // C 
#define DIRECTION_DR              3  // D 
#define DIRECTION_R               4  // E 
#define DIRECTION_UR              5  // F 
#define DIRECTION_U               6  // G 
#define DIRECTION_UL              7  // H 
#define ORGAN_LEAF                8  // I   // genes from here are organ types, they must go no higher than 26 so they correspond to a gene letter.
#define ORGAN_MOUTH               9  // J        
#define ORGAN_GONAD               10 // K           
#define ORGAN_MUSCLE              11 // L              
#define ORGAN_BONE                12 // M          
#define ORGAN_WEAPON              13 // N           
#define ORGAN_LIVER               14 // O        
#define ORGAN_SENSOR_FOOD         15 // P        
#define ORGAN_SENSOR_CREATURE     16 // Q          
#define ORGAN_SENSOR_LIGHT        17 // R      
#define ORGAN_SENSOR_RANDOM       18 // S      
#define ORGAN_SENSOR_INVERT       19 // T   
#define ORGAN_SENSOR_PARENT       20 // U   
#define ORGAN_SENSOR_HOME         21 // V   
#define GROW_BRANCH               22 // W   // these genes are meta instructions that control how the gene sequence should be read.
#define GROW_END                  23 // X   
#define GROW_ADDOFFSPRINGENERGY   24 // Y    
#define GROW_STRIDE               25 // Z    
#define GROW_SEQUENCE             26 // [     
#define MATERIAL_NOTHING          27 //     // materials and other things that are not part of the creature can range between 26 and 31, which is the max if we use unsigned int.
#define MATERIAL_FOOD             28 //           
#define MATERIAL_ROCK             29 //           
#define MATERIAL_WATER            30 //           
#define MARKER                    31 //      // numbers above 25 don't correspond to lower-case letters(0..25) so we don't use them in the gene code. But (26..31) are still compatible with our masking scheme.

#define WORLD_RANDOM 1
#define WORLD_EXAMPLECREATURE 2



const bool brownianMotion        = false;
const bool immortality           = false;
const bool doReproduction        = true;
const bool doMuscles             = true;
const bool doPhotosynth          = true;
const bool growingCostsEnergy    = true;
const bool lockfps               = false;
const bool tournament            = true;
const bool taxIsByMass           = true;



const bool cameraFollowsChampion = true;
// const bool cameraShowsSignal= false;

unsigned int worldToLoad = WORLD_RANDOM;






const unsigned int viewFieldX = 203; // 80 columns, 24 rows is the default size of a terminal window
const unsigned int viewFieldY = 55 - 3;  // 203 columns, 55 rows is the max size i can make one on my pc.
const unsigned int viewFieldSize = viewFieldX * viewFieldY;

const int animalSize     = 32;
const unsigned int animalSquareSize      = animalSize * animalSize;
const int worldSize      = 256;
const unsigned int worldSquareSize       = worldSize * worldSize;
const unsigned int genomeSize      = 32;
const unsigned int numberOfAnimals = 10000;
const unsigned int numberOfAnimalsToSpawn = 100;
const unsigned int nNeighbours     = 8;
const unsigned int numberOfCharacters = 31;

const float growthEnergyScale      = 1.0f;        // a multiplier for how much it costs animals to make new cells.
const float taxEnergyScale         = 0.1f;        // a multiplier for how much it costs animals just to exist.
const float lightEnergy            = 0.0f;//.0100001f;   // how much energy an animal gains each turn from having a leaf. if tax is by mass, must be higher than taxEnergyScale to be worth having leaves at all.
const float movementEnergyScale    = 1.0f;        // a multiplier for how much it costs animals to move.
const float foodEnergy             = 0.5f;                     // how much you get from eating a piece of meat. should be less than 1 to avoid meat tornado
const float liverStorage = 10.0f;
const float musclePower = 0.1f; // the power of one muscle cell


const int excitationRange = 2;
const unsigned int baseSensorRange = 10;
const int sensorFidelity = 1;




float energyScaleIn             = 1.0f;     // a multiplier for how much energy is gained from food and light.
float minimumEntropy = 0.1f;
float energyScaleOut           = minimumEntropy;


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


unsigned int cameraPositionX = (worldSize / 2);
unsigned int cameraPositionY = (worldSize / 2);
unsigned int modelFrameCount = 0;

int champion = -1;
char championGenes[genomeSize];
int championScore = 0;
int tournamentInterval = 10000;
int tournamentCounter  = 0;
int numberOfKnights = 12;

int cameraTargetCreature = -1;
unsigned int threadTimer = 0;
unsigned int populationCount = 0;
unsigned int cameraFrameCount = 0;

struct Square
{
	unsigned int material;
	int identity;
	int height;
	float light;
};

struct Square world[worldSquareSize];

struct Cell
{
	unsigned int organ;
	unsigned int geneCursor;
	unsigned int growDirection;
	unsigned int growthMask;
	unsigned int grown;
	unsigned int origin;
	unsigned int sequenceNumber;
	int sign;
	int sensorRange;
};

struct Animal
{
	Cell body[animalSquareSize];
	char genes[genomeSize];

	unsigned int mass;
	unsigned int stride;
	unsigned int position;
	unsigned int uPosX;
	unsigned int uPosY;
	unsigned int numberOfTimesReproduced;
	unsigned int damageDone;
	unsigned int damageReceived;
	unsigned int birthLocation;
	int parentIdentity;
	bool retired;
	float fPosX;
	float fPosY;
	float fangle;
	float offspringEnergy;
	float energy;
	float maxEnergy;
	float energyDebt;


	// unsigned int signalLocation;
	int destinationX;
	int destinationY;

	// float signalIntensity;
};

float organGrowthCost(unsigned int organ)
{
	ZoneScoped;
	float growthCost = 0.0f;
	if (growingCostsEnergy)
	{
		growthCost = 1.0f;
		switch (organ)
		{
		case ORGAN_LEAF:
			growthCost *= 1.0f;
			break;
		case ORGAN_MUSCLE:
			growthCost *= 1.0f;
			break;
		case ORGAN_BONE:
			growthCost *= 1.0f;
			break;
		case ORGAN_WEAPON:
			growthCost *= 2.0f;
			break;
		case ORGAN_SENSOR_FOOD:
			growthCost *= 2.0f;
			break;
		case ORGAN_SENSOR_LIGHT:
			growthCost *= 2.0f;
			break;
		case ORGAN_SENSOR_CREATURE:
			growthCost *= 2.0f;
			break;
		case ORGAN_GONAD:
			growthCost *= 5.0f;
			break;
		case ORGAN_MOUTH:
			growthCost *= 5.0f;
			break;
		}
	}
	return growthCost;
}

float organUpkeepCost(unsigned int organ)
{
	ZoneScoped;
	float upkeepCost = 1.0f;
	switch (organ)
	{
	case ORGAN_LEAF:
		upkeepCost *= 0.0f;
		break;
	case ORGAN_BONE:
		upkeepCost *= 0.0f;
		break;
	case ORGAN_WEAPON:
		upkeepCost *= 0.0f;
		break;
	case ORGAN_MOUTH:
		upkeepCost *= 0.0f;
		break;
	case ORGAN_MUSCLE:
		upkeepCost *= 1.0f;
		break;
	case ORGAN_GONAD:
		upkeepCost *= 1.0f;
		break;
	case ORGAN_LIVER:
		upkeepCost *= 1.0f;
		break;
	case ORGAN_SENSOR_FOOD:
		upkeepCost *= 2.0f;
		break;
	case ORGAN_SENSOR_LIGHT:
		upkeepCost *= 2.0f;
		break;
	case ORGAN_SENSOR_CREATURE:
		upkeepCost *= 2.0f;
		break;
	}
	return upkeepCost;
}

char exampleAnimal[genomeSize] ;
Animal animals[numberOfAnimals];

void resetAnimal(unsigned int animalIndex)
{
	ZoneScoped;
	if (animalIndex >= 0)
	{
		for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
		{
			animals[animalIndex].body[cellLocalPositionI].organ  = MATERIAL_NOTHING;
			animals[animalIndex].body[cellLocalPositionI].geneCursor = 0;
			animals[animalIndex].body[cellLocalPositionI].growDirection = 0x00;
			animals[animalIndex].body[cellLocalPositionI].grown = true;
			animals[animalIndex].body[cellLocalPositionI].origin = 0;
		}
		memset( &(animals[animalIndex].genes[0]), 0x00, genomeSize );
		animals[animalIndex].offspringEnergy = 1.0f;
		animals[animalIndex].energy   = 0.0f;
		animals[animalIndex].energyDebt   = 0.0f;
		animals[animalIndex].mass = 0;
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
		animals[animalIndex].birthLocation = 0;

		// animals[animalIndex].signalLocation = 0;
		animals[animalIndex].destinationX = 0;
		animals[animalIndex].destinationY = 0;


		// animals[animalIndex].signalIntensity = 0.0f;
	}
}

void resetAnimals()
{
	ZoneScoped;
	for ( int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		resetAnimal(animalIndex);
	}
}

void resetGrid()
{
	ZoneScoped;
	for (int i = 0; i < worldSquareSize; ++i)
	{
		world[i].material = MATERIAL_NOTHING;
		world[i].identity = -1;
		world[i].light = 1.0f;
	}
}

char geneCodeToChar( unsigned int n )
{
	ZoneScoped;
	return (char)('A' + n);
}

unsigned int charToGeneCode( char c )
{
	ZoneScoped;
	return (unsigned int)(c - 'A');
}

void examplePlant (unsigned int animalIndex)
{
	ZoneScoped;
	for (int i = 0; i < genomeSize; ++i)
	{
		animals[animalIndex].genes[i] = geneCodeToChar( MATERIAL_NOTHING);
	}
	animals[animalIndex].genes[1]  = geneCodeToChar(DIRECTION_L ) ;
	animals[animalIndex].genes[2]  = geneCodeToChar(GROW_BRANCH ) ;
	animals[animalIndex].genes[3]  = geneCodeToChar(ORGAN_GONAD ) ;
	animals[animalIndex].genes[4]  = geneCodeToChar(ORGAN_GONAD ) ;
	animals[animalIndex].genes[5]  = geneCodeToChar(GROW_END ) ;
	// animals[animalIndex].energy = 8.0f;
}

void setupExampleAnimal(unsigned int animalIndex)
{
	ZoneScoped;
	for (int i = 0; i < genomeSize; ++i)
	{
		animals[animalIndex].genes[i] = geneCodeToChar( MATERIAL_NOTHING);
	}


	animals[animalIndex].genes[1]  = geneCodeToChar(DIRECTION_D ) ;
	animals[animalIndex].genes[2]  = geneCodeToChar(GROW_BRANCH ) ;
	animals[animalIndex].genes[3]  = geneCodeToChar(ORGAN_GONAD ) ;
	animals[animalIndex].genes[4]  = geneCodeToChar(ORGAN_GONAD ) ;
	animals[animalIndex].genes[5]  = geneCodeToChar(DIRECTION_DR ) ;
	animals[animalIndex].genes[6]  = geneCodeToChar(GROW_BRANCH ) ;
	animals[animalIndex].genes[7]  = geneCodeToChar(ORGAN_MUSCLE ) ;
	animals[animalIndex].genes[8]  = geneCodeToChar(ORGAN_SENSOR_CREATURE ) ;
	animals[animalIndex].genes[9]  = geneCodeToChar(ORGAN_MOUTH ) ;
	animals[animalIndex].genes[10]  = geneCodeToChar(GROW_END ) ;
	animals[animalIndex].genes[11]  = geneCodeToChar(DIRECTION_DL ) ;
	animals[animalIndex].genes[12]  = geneCodeToChar(GROW_BRANCH ) ;
	animals[animalIndex].genes[13]  = geneCodeToChar(ORGAN_MUSCLE ) ;
	animals[animalIndex].genes[14]  = geneCodeToChar(ORGAN_SENSOR_FOOD ) ;
	animals[animalIndex].genes[15]  = geneCodeToChar(ORGAN_MOUTH ) ;
	animals[animalIndex].genes[16]  = geneCodeToChar(GROW_END ) ;
	animals[animalIndex].genes[17]  = geneCodeToChar(GROW_END ) ;


	// // animals[animalIndex].genes[0]  = geneCodeToChar(DIRECTION_UL ) ;
	// // animals[animalIndex].genes[1]  = geneCodeToChar(DIRECTION_UL ) ;
	// // animals[animalIndex].genes[2]  = geneCodeToChar(DIRECTION_U ) ;
	// // animals[animalIndex].genes[3]  = geneCodeToChar(DIRECTION_UR ) ;
	// animals[animalIndex].genes[1]  = geneCodeToChar(DIRECTION_L ) ;
	// animals[animalIndex].genes[2]  = geneCodeToChar(DIRECTION_R ) ;
	// // animals[animalIndex].genes[6]  = geneCodeToChar(DIRECTION_DL ) ;
	// // animals[animalIndex].genes[7]  = geneCodeToChar(DIRECTION_D ) ;
	// // animals[animalIndex].genes[8]  = geneCodeToChar(DIRECTION_DR ) ;
	// animals[animalIndex].genes[3]  = geneCodeToChar(GROW_BRANCH ) ;
	// animals[animalIndex].genes[4]  = geneCodeToChar(ORGAN_BONE ) ;
	// animals[animalIndex].genes[5]  = geneCodeToChar(ORGAN_BONE ) ;
	// animals[animalIndex].genes[6]  = geneCodeToChar(ORGAN_BONE ) ;
}


// #define INT_BITS 32
// int leftRotate(int n, unsigned int d)
// {
// 	/*
//     https://www.geeksforgeeks.org/rotate-bits-of-an-integer/
// 	In n<<d, last d bits are 0. To
// 	 put first 3 bits of n at
// 	last, do bitwise or of n<<d
// 	with n >>(INT_BITS - d) */
// 	return (n << d) | (n >> (INT_BITS - d));
// }

// Function to right rotate n by d bits
// int rightRotate(int n, unsigned int d)
// {
// 	/*
// 	https://www.geeksforgeeks.org/rotate-bits-of-an-integer/
// 	In n>>d, first d bits are 0.
// 	To put last 3 bits of at
// 	first, do bitwise or of n>>d
// 	with n <<(INT_BITS - d) */
// 	return (n >> d) | (n << (INT_BITS - d));
// }



void grow( int animalIndex, unsigned int cellLocalPositionI)
{
	ZoneScoped;
	animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
	if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
	unsigned int gene =  charToGeneCode( (animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ]) );
	if (gene == GROW_BRANCH)
	{


		for (unsigned int n = 0; n < nNeighbours; ++n)
		{
			if ( ( animals[animalIndex].body[cellLocalPositionI].growthMask & (1U << n)) == (1U << n) )                               // if the growth mask says this neighbour is ready
			{

				// unsigned int rotatedN = (n + animals[animalIndex].body[cellLocalPositionI].growDirection) % nNeighbours;
				unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
				if (cellNeighbour < animalSquareSize)
				{
					animals[animalIndex].body[cellNeighbour].origin     = cellLocalPositionI;
					animals[animalIndex].body[cellNeighbour].geneCursor = animals[animalIndex].body[cellLocalPositionI].geneCursor ; // the neighbour will choose a gene at genecursor+1 anyway
					animals[animalIndex].body[cellNeighbour].growDirection =  n;
					animals[animalIndex].body[cellNeighbour].growthMask = 0x00;
					animals[animalIndex].body[cellNeighbour].grown = false;
				}
			}
		}
		animals[animalIndex].body[cellLocalPositionI].grown = true;
		animals[animalIndex].body[cellLocalPositionI].growthMask = 0x00;
		return;
	}
	else if (gene == GROW_END)
	{
		unsigned int origin =   animals[animalIndex].body[cellLocalPositionI].origin  ;
		int sequenceNumber = 	animals[animalIndex].body[   origin    ].sequenceNumber;
		if (sequenceNumber == 0)
		{
			animals[animalIndex].body[cellLocalPositionI].grown = true;
			animals[animalIndex].body[    origin      ].grown = false;   // if sequence number == 0, this is a branch, so finish working on this branch and return to the cell it originated from.
			animals[animalIndex].body[    origin      ].geneCursor = animals[animalIndex].body[cellLocalPositionI].geneCursor;
		}
		else  if (sequenceNumber > 0)                                                             /// if sequence number > 0, you are in a sequence, start the sequence over again.
		{
			animals[animalIndex].body[  origin     ].sequenceNumber--;
			for (unsigned int n = 0; n < nNeighbours; ++n)
			{
				if ( animals[animalIndex].body[cellLocalPositionI].growDirection == n ) //& (1U << n)) == (1U << n) )                               // if the growth mask says this neighbour is ready
				{
					unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
					if (cellNeighbour < animalSquareSize)
					{
						animals[animalIndex].body[cellNeighbour] = animals[animalIndex].body[    origin    ]; // copy the origin cell to the neighbour and begin the sequence again.
						animals[animalIndex].body[cellNeighbour].origin  = origin ;// except, the new cell's origin is still set to this sequence's origin, not the origin's origin.
						animals[animalIndex].body[cellNeighbour].grown = false;
					}
				}
			}
		}
		return;
	}
	else if (gene == GROW_ADDOFFSPRINGENERGY)	// increase the amount of energy given to newborn offspring.
	{
		animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
		if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		animals[animalIndex].offspringEnergy  = charToGeneCode( animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ] );
		return;
	}
	else if (gene == GROW_STRIDE) // increase how often the animal's path is updated, which affects its random path length.
	{
		animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
		if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		animals[animalIndex].stride  = charToGeneCode( animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ] );
		return;
	}
	else if (gene == GROW_SEQUENCE) // repeat a motif a number of times in a row.
	{
		// whenever a break is encountered at a sequence number higher than 0, the new growing cell's properties are copied from the origin, and the sequence number is decremented.
		animals[animalIndex].body[cellLocalPositionI].geneCursor ++;
		if (animals[animalIndex].body[cellLocalPositionI].geneCursor >= genomeSize) { animals[animalIndex].body[cellLocalPositionI].grown = true; return; }
		animals[animalIndex].body[cellLocalPositionI].sequenceNumber  = charToGeneCode( animals[animalIndex].genes[ animals[animalIndex].body[cellLocalPositionI].geneCursor ] );
		for (unsigned int n = 0; n < nNeighbours; ++n)
		{
			if ( ( animals[animalIndex].body[cellLocalPositionI].growthMask & (1U << n)) == (1U << n) )                               // if the growth mask says this neighbour is ready
			{
				unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
				if (cellNeighbour < animalSquareSize)
				{
					animals[animalIndex].body[cellNeighbour].origin     = cellLocalPositionI;
					animals[animalIndex].body[cellNeighbour].geneCursor = animals[animalIndex].body[cellLocalPositionI].geneCursor ; // the neighbour will choose a gene at genecursor+1 anyway
					animals[animalIndex].body[cellNeighbour].growDirection = n;// 1U << n);
					animals[animalIndex].body[cellNeighbour].growthMask = 0x00;
					animals[animalIndex].body[cellNeighbour].grown = false;
				}
			}
		}
		animals[animalIndex].body[cellLocalPositionI].grown = true;
		return;
	}
	else if ( gene == MATERIAL_NOTHING) // terminate the sequence completely.// certain characters mark the end of the entire sequence.
	{
		animals[animalIndex].body[cellLocalPositionI].geneCursor = genomeSize;
		animals[animalIndex].body[cellLocalPositionI].grown = true; return;
	}

	if (gene < nNeighbours)                                                                                                                 // the gene is a direction, add it into the growth mask (lower 8 digits of the organ number), then fetch the next gene
	{
		animals[animalIndex].body[cellLocalPositionI].growthMask  ^= (1 << gene)  ; // toggle a bit in the growth mask
		// printf("animals[animalIndex].body[cellLocalPositionI].growthMask %u\n", animals[animalIndex].body[cellLocalPositionI].growthMask);
		return;
	}
	else                                                                                                                               // if the gene is a growable organ, grow it in this square, mark the square as completed, and then journey on to the activated neighbours
	{
		// if the current cell does not have that organ type, add it in
		// if it does have it, do not add it in, and mark the neighbour to grow, SETTING THE GENE CURSOR BACK ONE PLACE so the neighbour gets the gene instead.
		// if (!animals[animalIndex].body[cellLocalPositionI].grown && ((animals[animalIndex].body[cellLocalPositionI].organ  & gene ) != gene))
		if (animals[animalIndex].body[cellLocalPositionI].organ == MATERIAL_NOTHING)
		{
			animals[animalIndex].body[cellLocalPositionI].organ = gene;






			if ( gene == ORGAN_SENSOR_FOOD ||
			        gene == ORGAN_SENSOR_LIGHT ||
			        gene == ORGAN_SENSOR_CREATURE ||
			        gene == ORGAN_SENSOR_RANDOM ||
			        gene == ORGAN_SENSOR_INVERT ||
			        gene == ORGAN_SENSOR_HOME ||
			        gene == ORGAN_SENSOR_PARENT	)
			{
				animals[animalIndex].body[cellLocalPositionI].sensorRange = baseSensorRange;
				for (unsigned int n = 0; n < nNeighbours; ++n)
				{

					unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
					if (cellNeighbour < animalSquareSize)
					{
						if (animals[animalIndex].body[cellNeighbour].organ == gene)
						{
							animals[animalIndex].body[cellLocalPositionI].sensorRange += baseSensorRange;
							animals[animalIndex].body[cellNeighbour].sensorRange = animals[animalIndex].body[cellLocalPositionI].sensorRange ;
						}

					}

				}

			}




		}
		else
		{
			for (unsigned int n = 0; n < nNeighbours; ++n)
			{
				if (  animals[animalIndex].body[cellLocalPositionI].growDirection == n)                               // if the growth mask says this neighbour is ready
				{
					unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
					if (cellNeighbour < animalSquareSize)
					{
						animals[animalIndex].body[cellNeighbour].origin     = animals[animalIndex].body[cellLocalPositionI].origin;
						animals[animalIndex].body[cellNeighbour].geneCursor = animals[animalIndex].body[cellLocalPositionI].geneCursor - 1; // the neighbour will choose a gene at genecursor+1 anyway, so its like it gets the same gene as you just had.
						animals[animalIndex].body[cellNeighbour].growDirection = n;//(1U << n);
						animals[animalIndex].body[cellNeighbour].growthMask = animals[animalIndex].body[cellLocalPositionI].growthMask ;
						animals[animalIndex].body[cellNeighbour].grown = false;
					}
				}
			}
			animals[animalIndex].body[cellLocalPositionI].grown = true;
		}
	}
}

int getNewIdentity()
{
	ZoneScoped;
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

char randomLetter()
{
	ZoneScoped;
	return geneCodeToChar (extremelyFastNumberFromZeroTo(31));
}

void mutateGenes( int animalIndex)
{
	ZoneScoped;
	unsigned int mutationType = extremelyFastNumberFromZeroTo(3);
	if (mutationType == 0)  // randomise a letter
	{
		unsigned int geneIndex = 1 + extremelyFastNumberFromZeroTo(genomeSize - 1);
		animals[animalIndex].genes[geneIndex] = randomLetter();
	}
	else if (mutationType == 1)  // insert a random letter at a random place, pushing subsequent letters back
	{
		unsigned int geneIndex = 1 + extremelyFastNumberFromZeroTo(genomeSize - 1);
		for (unsigned int i = genomeSize; i > geneIndex; --i)
		{
			animals[animalIndex].genes[i] = animals[animalIndex].genes[i - 1];
		}
		animals[animalIndex].genes[geneIndex] = randomLetter();
	}
	else if (mutationType == 2)  // delete a letter at a random place, pulling subsequent letters forward
	{
		unsigned int geneIndex = 1 +  extremelyFastNumberFromZeroTo(genomeSize - 2);
		for (unsigned int i = (geneIndex + 1); i < genomeSize; ++i)
		{
			animals[animalIndex].genes[i - 1] = animals[animalIndex].genes[i];
		}
	}
	else if (mutationType == 3)  // swap the order of two adjacent letters
	{

		unsigned int geneIndex = 1 +  extremelyFastNumberFromZeroTo(genomeSize - 2);
		char temp = animals[animalIndex].genes[geneIndex];
		animals[animalIndex].genes[geneIndex] = animals[animalIndex].genes[geneIndex + 1];
		animals[animalIndex].genes[geneIndex + 1] = temp;

	}
}

void initially_place_animal(unsigned int animalIndex)
{
	unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
	unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
	for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)                                      // place animalIndex on grid and attack / eat. add captured energy
	{
		if ( (animals[animalIndex].body[cellLocalPositionI].organ != MATERIAL_NOTHING) )
		{
			unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
			unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
			unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
			unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
			unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;
			world[cellWorldPositionI].identity = animalIndex;
		}
	}
}

void spawnAnimalIntoSlot( unsigned int animalIndex,  char * genes, unsigned int position, bool mutation)
{
	ZoneScoped;
	resetAnimal(animalIndex);
	animals[animalIndex].retired = false;
	unsigned int cellLocalPositionX = animalSize / 2; // place a seed in the middle of the sprite
	unsigned int cellLocalPositionY = animalSize / 2;
	unsigned int cellLocalPositionI = (cellLocalPositionY * animalSize) + cellLocalPositionX;
	animals[animalIndex].body[cellLocalPositionI].origin = cellLocalPositionI;
	animals[animalIndex].body[cellLocalPositionI].organ = animals[animalIndex].body[cellLocalPositionI].organ = ORGAN_LEAF;
	animals[animalIndex].body[cellLocalPositionI].grown = false;
	animals[animalIndex].body[cellLocalPositionI].geneCursor = 0;
	animals[animalIndex].mass = 1;
	// animals[animalIndex].energy = 0.5f;
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
	animals[animalIndex].birthLocation = position;
	// world[position].identity= animalIndex;
	initially_place_animal( animalIndex);
}

int spawnAnimal( char * genes, unsigned int position, bool mutation)
{
	ZoneScoped;
	int animalIndex = getNewIdentity();
	if (animalIndex >= 0) // an animalIndex was available
	{
		spawnAnimalIntoSlot(animalIndex, genes, position, mutation);
	}
	return animalIndex;
}

void killAnimal(int animalIndex)
{
	ZoneScoped;
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
	ZoneScoped;
	Animal tempAnimal = Animal();
	for (int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)  // you cannot add stuff back into the animal while you are working on it, so create a copy to hold your updates.
	{
		int originalXDiff = (cellLocalPositionI % animalSize) - (animalSize / 2);
		int originalYDiff = (cellLocalPositionI / animalSize) - (animalSize / 2);
		int rotatedXDiff = originalYDiff;
		int rotatedYDiff = originalXDiff;
		if (direction == DIRECTION_R)
		{
			int rotatedXDiff = originalYDiff * -1;
			int rotatedYDiff = originalXDiff * -1;
		}
		int rotatedX = rotatedXDiff + (animalSize / 2);
		int rotatedY = rotatedYDiff + (animalSize / 2);
		int rotatedI = (rotatedY * animalSize) + rotatedX;
		tempAnimal.body[cellLocalPositionI] = animals[animalIndex].body[rotatedI];
	}
	for (int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)
	{
		animals[animalIndex].body[cellLocalPositionI] =	tempAnimal.body[cellLocalPositionI];
	}
}

int igetRelativeDistance(unsigned int a, unsigned int b)
{
	ZoneScoped;
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
	ZoneScoped;
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


// check if an animal is currently occupying a square. return the local index of the occupying cell, otherwise, return -1 if not occupied.
int isAnimalInSquare(unsigned int animalIndex, unsigned int cellWorldPositionX,  unsigned int cellWorldPositionY,  unsigned int cellWorldPositionI)
{

	ZoneScoped;
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

void sensor(int animalIndex, unsigned int cellLocalPositionI)
{


	unsigned int organ = animals[animalIndex].body[cellLocalPositionI].organ;

	if (organ == ORGAN_SENSOR_RANDOM)   // random sensors just random-walk the creature.
	{
		if (extremelyFastNumberFromZeroTo(animals[animalIndex].stride) == 0)
		{
			// animals[animalIndex].signalIntensity = RNG() * 1.0f;

			animals[animalIndex].destinationX += (RNG() - 0.5f) * 0.1f;
			animals[animalIndex].destinationY += (RNG() - 0.5f) * 0.1f;

		}
		return;
	}


	unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
	unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
	unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
	unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
	unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
	unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
	unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;

	if (organ == ORGAN_SENSOR_PARENT)
	{
		if (extremelyFastNumberFromZeroTo(animals[animalIndex].stride) == 0)
		{
			if (animals[animalIndex].parentIdentity > 0 && animals[animalIndex].parentIdentity < numberOfAnimals)
			{
				if (! animals[animals[animalIndex].parentIdentity].retired)
				{
					// animals[animalIndex].body[cellLocalPositionI].signalIntensity = 1;//RNG() * 1.0f;
					// animals[animalIndex].body[cellLocalPositionI].signalLocation =  animals[animals[animalIndex].parentIdentity].position;//RNG() * worldSquareSize;

					unsigned int parentPos = animals[animals[animalIndex].parentIdentity].position;
					unsigned int parentPosX = parentPos % worldSize;
					unsigned int parentPosY = parentPos / worldSize;

					animals[animalIndex].destinationX += (parentPosX - animals[animalIndex].destinationX) * 0.1f;
					animals[animalIndex].destinationY += (parentPosY - animals[animalIndex].destinationY) * 0.1f;

					// for (unsigned int n = 0; n < nNeighbours; ++n)
					// {
					// 	unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
					// 	if (cellNeighbour < animalSquareSize)
					// 	{
					// 		animals[animalIndex].body[cellNeighbour].signalIntensity = animals[animalIndex].body[cellLocalPositionI].signalIntensity;
					// 		animals[animalIndex].body[cellNeighbour].signalLocation = animals[animalIndex].body[cellLocalPositionI].signalLocation;
					// 	}
					// }
				}
			}
		}
		return;
	}

	if (organ == ORGAN_SENSOR_HOME)
	{
		if (extremelyFastNumberFromZeroTo(animals[animalIndex].stride) == 0)
		{
			// animals[animalIndex].body[cellLocalPositionI].signalIntensity = 1.0f;//RNG() * 1.0f;
			// animals[animalIndex].body[cellLocalPositionI].signalLocation = animals[animalIndex].birthLocation;//RNG() * worldSquareSize;

			unsigned int homePos = animals[animalIndex].birthLocation;//animals[animals[animalIndex].parentIdentity].position;
			unsigned int homePosX = homePos % worldSize;
			unsigned int homePosY = homePos / worldSize;

			animals[animalIndex].destinationX += (homePosX - animals[animalIndex].destinationX) * 0.1f;
			animals[animalIndex].destinationY += (homePosY - animals[animalIndex].destinationY) * 0.1f;


			// for (unsigned int n = 0; n < nNeighbours; ++n)
			// {
			// 	unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
			// 	if (cellNeighbour < animalSquareSize)
			// 	{
			// 		animals[animalIndex].body[cellNeighbour].signalIntensity = animals[animalIndex].body[cellLocalPositionI].signalIntensity;
			// 		animals[animalIndex].body[cellNeighbour].signalLocation = animals[animalIndex].body[cellLocalPositionI].signalLocation;
			// 	}
			// }
		}
		return;
	}

	// for (int n = 0; n < sensorFidelity; ++n)
	// {
	unsigned int x = (cellWorldPositionX - animals[animalIndex].body[cellLocalPositionI].sensorRange) + extremelyFastNumberFromZeroTo( animals[animalIndex].body[cellLocalPositionI]. sensorRange * 2);
	unsigned int y = (cellWorldPositionY - animals[animalIndex].body[cellLocalPositionI].sensorRange) + extremelyFastNumberFromZeroTo( animals[animalIndex].body[cellLocalPositionI]. sensorRange * 2);
	bool detected = false;
	if (x < worldSize && x > 0 && y < worldSize && y > 0)
	{
		unsigned int targetWorldPositionI =    (( y * worldSize ) + x ); // center the search area on the cell's world position.
		if ( organ  == ORGAN_SENSOR_LIGHT)
		{
			unsigned int virtualDestination = (animals[animalIndex].destinationY * worldSize) + animals[animalIndex].destinationX;
			if (virtualDestination < worldSquareSize)
			{
				if (world[targetWorldPositionI].light > world[virtualDestination].light )
				{
					detected = true;
				}
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


				if (

				    // animals[world[targetWorldPositionI].identity].position == targetWorldPositionI



// check if an animal is currently occupying a square. return the local index of the occupying cell, otherwise, return -1 if not occupied.
// int isAnimalInSquare(unsigned int animalIndex, unsigned int cellWorldPositionX,  unsigned int cellWorldPositionY,  unsigned int cellWorldPositionI)


				    isAnimalInSquare(world[targetWorldPositionI].identity , x, y, targetWorldPositionI) > 0
				) //only sense the present location of the animal, not where it has previously been.
				{
					detected = true;
				}
			}
		}




		if (detected)
		{

			// for ( int excite_y =  - excitationRange; excite_y < excitationRange; ++excite_y)
			// {
			// 	for ( int excite_x = -excitationRange; excite_x <  excitationRange; ++excite_x)
			// // 	{
			// 		int excitedX = cellLocalPositionX + excite_x;
			// 		int excitedY = cellLocalPositionY + excite_y;
			// 		if (excitedX < 0 || excitedX >= animalSize || excitedY < 0 || excitedY >= animalSize) { continue;}
			// 		int excitedI = (excitedY * animalSize) + excitedX;
			// 		if (excitedI < animalSquareSize)
			// 		{
			// 			float mag = abs(excite_x) + abs(excite_y);
			// 			if (mag < 1.0f) {mag = 1.0f;}
			// 			float sig =  animals[animalIndex].body[cellLocalPositionI].signalIntensity /  mag;
			// 			if (sig  > animals[animalIndex].body[excitedI].signalIntensity )
			// 			{
			// 				animals[animalIndex].body[excitedI].signalIntensity = sig;//animals[animalIndex].body[cellLocalPositionI].signalIntensity /  mag;
			// animals[animalIndex].body[excitedI].signalLocation = animals[animalIndex].body[cellLocalPositionI].signalLocation;




			// unsigned int parentPos = targetWorldPositionI ;//animals[animals[animalIndex].parentIdentity].position;
			unsigned int targetPosX = targetWorldPositionI % worldSize;
			unsigned int targetPosY = targetWorldPositionI / worldSize;

			animals[animalIndex].destinationX += (targetPosX - animals[animalIndex]. destinationX) * 0.1f;
			animals[animalIndex].destinationY += (targetPosY - animals[animalIndex]. destinationY) * 0.1f;





			// 	}
			// }
			// 	}
			// }
		}
	}
	// }




}


int defenseAtPoint(unsigned int animalIndex, unsigned int cellLocalPositionI)
{

	ZoneScoped;
	int nBones = 0;
	for (unsigned int n = 0; n < nNeighbours; ++n)
	{
		unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
		if (cellNeighbour < animalSquareSize)
		{
			if (animals[animalIndex].body[cellNeighbour].organ  == ORGAN_BONE)
			{
				nBones++;
			}
		}
	}
	return nBones * nBones;
}





void grow_all()
{
	ZoneScoped;

	// grow all growable cells.
	unsigned int newpop = 0;
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		if (!animals[animalIndex].retired)
		{
			newpop++;
			if (animals[animalIndex].energyDebt > 0.0f)
			{
				float amount = animals[animalIndex].energy - (animals[animalIndex].mass / 2.0f);
				animals[animalIndex].energyDebt -= amount;
				animals[animalIndex].energy     -= amount;
			}
			else
			{
				for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI) // process organs and signals and clear animalIndex on grid
				{
					if (!(animals[animalIndex].body[cellLocalPositionI].grown))
					{
						grow( animalIndex,  cellLocalPositionI);
					}
				}
			}
		}
	}
	populationCount = newpop;
}




void organs_all()
{

	ZoneScoped;
	// perform organ functionality.
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		if (!animals[animalIndex].retired)
		{
			float totalLiver = 0;
			for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)                                      // place animalIndex on grid and attack / eat. add captured energy
			{
				if (animals[animalIndex].body[cellLocalPositionI].organ == MATERIAL_NOTHING) {continue;}



				// float reductionRatio = 1.0f;
				// if (animals[animalIndex].stride > 0)
				// {
				// 	reductionRatio = (1.0f - (0.05f / animals[animalIndex].stride));
				// }
				// else
				// {
				// 	reductionRatio = (1.0f - (0.05f));
				// }
				// animals[animalIndex].body[cellLocalPositionI].signalIntensity *= reductionRatio;




				unsigned int organ = animals[animalIndex].body[cellLocalPositionI].organ;


				if (
				    organ == ORGAN_SENSOR_FOOD ||
				    organ == ORGAN_SENSOR_LIGHT ||
				    organ == ORGAN_SENSOR_CREATURE ||
				    organ == ORGAN_SENSOR_RANDOM ||
				    organ == ORGAN_SENSOR_INVERT ||
				    organ == ORGAN_SENSOR_HOME ||
				    organ == ORGAN_SENSOR_PARENT
				)
				{
					sensor(animalIndex, cellLocalPositionI);
					continue;
				}
				else
				{




					switch (organ)
					{
					case ORGAN_GONAD:
					{
						if (doReproduction && animals[animalIndex].body[cellLocalPositionI].grown )
						{
							if (animals[animalIndex].energy > ((animals[animalIndex].mass / 2 ) + animals[animalIndex].offspringEnergy ))
							{
								unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
								unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
								unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
								unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
								unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
								unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
								unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;

								if (cellWorldPositionI < worldSquareSize)
								{
									int result = spawnAnimal( animals[animalIndex].genes , cellWorldPositionI, true );
									if (result >= 0)
									{
										animals[animalIndex].body[cellLocalPositionI].organ = MATERIAL_NOTHING;// animals[animalIndex].body[cellLocalPositionI].organ ^ ORGAN_GONAD;
										animals[animalIndex].numberOfTimesReproduced++;
										animals[animalIndex].energy -= animals[animalIndex].offspringEnergy;
										animals[result].energy       =  animals[animalIndex].offspringEnergy;
										animals[result].parentIdentity       = animalIndex;
									}
								}
							}
						}
						break;
					}
					case ORGAN_LIVER :
					{
						totalLiver += 1.0f;
						break;
					}
					case ORGAN_LEAF:
					{
						if (doPhotosynth)
						{
							unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
							unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
							unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
							unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
							unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
							unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
							unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;
							if (world[cellWorldPositionI].identity == animalIndex)
							{
								animals[animalIndex].energy += world[cellWorldPositionI].light * energyScaleIn;
							}
						}
						break;
					}
					case ORGAN_MOUTH :
					{
						unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
						unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
						unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
						unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
						unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
						unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
						unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;

						if (world[cellWorldPositionI].material == MATERIAL_FOOD && world[cellWorldPositionI].identity == animalIndex)
						{
							animals[animalIndex].energy += foodEnergy * energyScaleIn;
							world[cellWorldPositionI].material = MATERIAL_NOTHING;
						}
						// animals[animalIndex].energy -= taxEnergyScale * energyScaleOut * organUpkeepCost(ORGAN_MOUTH);
						break;
					}
					case ORGAN_MUSCLE :
					{
						if (doMuscles)
						{


							unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
							unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;

							// go through your neighbours. find their angles.
							// break angle down into x and y component (doing it this way decouples intensity from the distance. they usually mean the same thing, but it is good to have options).
							// multiply components by the intensity. add the multiplied components together into a final vector. basically this whole process is a vector addition.
							float muscleX = 0.0f;
							float muscleY = 0.0f;
							// for (unsigned int n = 0; n < nNeighbours; ++n)
							// {
							// 	unsigned int cellNeighbour = cellLocalPositionI + cellNeighbourOffsets[n];
							// 	if (cellNeighbour < animalSquareSize)
							// 	{
							// 		if ( abs(animals[animalIndex].body[cellNeighbour].signalIntensity) > 0.001f && animals[animalIndex].body[cellNeighbour].signalLocation != 0)
							// 		{
							// 			float angle = getRelativeDirection( animals[animalIndex].position, animals[animalIndex].body[cellNeighbour].signalLocation  );
							// 			muscleX += (cos(angle) * animals[animalIndex].body[cellNeighbour].signalIntensity);
							// 			muscleY += (sin(angle) * animals[animalIndex].body[cellNeighbour].signalIntensity);
							// 		}
							// 	}
							// }


							float diffX = (animals[animalIndex].destinationX - animalWorldPositionX);
							float diffY = (animals[animalIndex].destinationY - animalWorldPositionY);
							float diffSum = abs(diffX) + abs(diffY);

							if (diffSum != 0.0f)
							{
								muscleX = diffX / diffSum;
								muscleY = diffY / diffSum;
							}

							if (animals[animalIndex].mass > 0)
							{
								animals[animalIndex].fPosX += ( muscleX ) / animals[animalIndex].mass;
								animals[animalIndex].fPosY += ( muscleY ) / animals[animalIndex].mass;
							}
							animals[animalIndex].energy -= (muscleX + muscleY) * movementEnergyScale * energyScaleOut;
						}
						break;
					}
					}
				}
			}
			animals[animalIndex].maxEnergy = animals[animalIndex].mass + (totalLiver * liverStorage);
		}
	}
}

void move_all() // perform movement, feeding, and combat.
{
	ZoneScoped;
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		if (!animals[animalIndex].retired)
		{
			if (brownianMotion)
			{
				if (animals[animalIndex].mass > 0)
				{
					animals[animalIndex].fPosY +=  ((RNG() - 0.5f) * 0.1f) / animals[animalIndex].mass;
					animals[animalIndex].fPosX +=  ((RNG() - 0.5f) * 0.1f) / animals[animalIndex].mass;
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
				if ( world[newPosition].material != MATERIAL_ROCK)
				{
					animals[animalIndex].position = newPosition;
					for (unsigned int cellLocalPositionI = 0; cellLocalPositionI < animalSquareSize; ++cellLocalPositionI)                                      // place animalIndex on grid and attack / eat. add captured energy
					{
						if ( (animals[animalIndex].body[cellLocalPositionI].organ != MATERIAL_NOTHING) )
						{
							animals[animalIndex].energy -= taxEnergyScale * energyScaleOut * organUpkeepCost(animals[animalIndex].body[cellLocalPositionI].organ);
							bool okToStep = true;
							unsigned int animalWorldPositionX    = animals[animalIndex].position % worldSize;
							unsigned int animalWorldPositionY    = animals[animalIndex].position / worldSize;
							unsigned int cellLocalPositionX = cellLocalPositionI % animalSize;
							unsigned int cellLocalPositionY = cellLocalPositionI / animalSize;
							unsigned int cellWorldPositionX = cellLocalPositionX + animalWorldPositionX;
							unsigned int cellWorldPositionY = cellLocalPositionY + animalWorldPositionY;
							unsigned int cellWorldPositionI = (cellWorldPositionY * worldSize) + cellWorldPositionX;

							if (world[cellWorldPositionI].identity >= 0 && world[cellWorldPositionI].identity != animalIndex && world[cellWorldPositionI].identity < numberOfAnimals)
							{
								int targetLocalPositionI = isAnimalInSquare( world[cellWorldPositionI].identity, cellWorldPositionX, cellWorldPositionY, cellWorldPositionI);
								if (targetLocalPositionI >= 0)
								{
									okToStep = false;
									if (animals[animalIndex].body[cellLocalPositionI].organ == ORGAN_MOUTH)
									{
										float amount = 0;

										if (animals[world[cellWorldPositionI].identity].mass > 0)
										{
											amount = (animals[world[cellWorldPositionI].identity].energy) / (animals[world[cellWorldPositionI].identity].mass);
										}
										animals[animalIndex].energy += amount;
										animals[world[cellWorldPositionI].identity].energy -= amount;
									}
									if (animals[animalIndex].body[cellLocalPositionI].organ == ORGAN_WEAPON)
									{
										int defense = defenseAtPoint(world[cellWorldPositionI].identity, targetLocalPositionI);
										if (extremelyFastNumberFromZeroTo(defense) == 0)
										{
											animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].organ = MATERIAL_NOTHING;
											// animals[world[cellWorldPositionI].identity].body[targetLocalPositionI].signalIntensity = 0.0f;

											if (animals[world[cellWorldPositionI].identity].mass >= 1)
											{
												animals[world[cellWorldPositionI].identity].mass--;

											}
											animals[world[cellWorldPositionI].identity].damageReceived++;
											okToStep = true;
											animals[animalIndex].damageDone++;
											if (world[cellWorldPositionI].material == MATERIAL_NOTHING)
											{
												world[cellWorldPositionI].material = MATERIAL_FOOD;
											}
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
		}
	}
}

void energy_all() // perform energies.
{
	ZoneScoped;
	// unsigned int newPopulationCount = 0;
	for (unsigned int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	{
		if (!animals[animalIndex].retired)
		{

			// newPopulationCount++;
			if (animals[animalIndex].energy > animals[animalIndex].maxEnergy) {animals[animalIndex].energy = animals[animalIndex].maxEnergy;}
			if (!immortality)// die
			{
				if (animals[animalIndex].energy < 0.0f ||
				        animals[animalIndex].damageReceived > animals[animalIndex].mass)
				{
					killAnimal( animalIndex);
					return;
				}

				if (animals[animalIndex].mass <= 0)
				{
					killAnimal(animalIndex);
					return;
				}
			}
			if (tournament)
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
		}
	}
	// populationCount = newPopulationCount;
}

void computeAllAnimalsOneTurn()
{
	ZoneScoped;



	// boost::thread t7{ interfaceSupervisor };
	boost::thread t7{ grow_all   };
	boost::thread t8{ organs_all };
	boost::thread t9{ move_all   };
	boost::thread t10{ energy_all };

	t10.join();
	t9.join();
	t8.join();
	t7.join();
}

void camera()
{

	ZoneScoped;
	for ( int vy = viewFieldY - 1; vy >= 0; --vy) // correct for Y axis inversion
	{
		for ( int vx = 0; vx < viewFieldX; ++vx)
		{
			if (cameraTargetCreature >= 0)
			{
				int creatureX = animals[cameraTargetCreature].position;
				int creatureY = animals[cameraTargetCreature].position;

				int newCameraPositionX = creatureX - (viewFieldX / 2);
				int newCameraPositionY = creatureY - (viewFieldY / 2);

				if (newCameraPositionX > 0 && newCameraPositionX < worldSize && newCameraPositionY > 0 && newCameraPositionY < worldSize)
				{
					// if (newCameraPositionX > 0 && crea)
					cameraPositionX = newCameraPositionX;
					cameraPositionY = newCameraPositionY;
				}
				if (animals[cameraTargetCreature].retired)
				{
					cameraTargetCreature = -1;
				}
			}
			if (cameraFollowsChampion)
			{
				if (champion >= 0)
				{
					cameraTargetCreature = champion;
				}
			}
			else
			{
				if (cameraFrameCount % 1000 == 0) // switch the view to a new creature every 1000 frames.
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
			int worldX = (cameraPositionX + vx) % worldSize; // center the view on the targeted position, instead of having it in the corner
			int worldY = (cameraPositionY + vy) % worldSize;
			int worldI = (worldY * worldSize) + worldX;
			char displayChar = ' ';
			if (worldX > 0 && worldX < worldSize && worldY > 0 && worldY < worldSize)
			{
				if (worldI < worldSquareSize)
				{
					if (world[worldI].material == MATERIAL_ROCK)
					{
						displayChar = '#';
					}
					else if (world[worldI].material == MATERIAL_FOOD)
					{
						displayChar = '@';
					}
					else if (world[worldI].material == MATERIAL_WATER)
					{
						displayChar = '~';
					}

					if (world[worldI].identity > -1)
					{
						if (world[worldI].identity == cameraTargetCreature)
						{
							displayChar = '-';
							if (world[worldI].material == MATERIAL_ROCK)
							{
								displayChar = '#';
							}
							else if (world[worldI].material == MATERIAL_FOOD)
							{
								displayChar = '@';
							}
							else if (world[worldI].material == MATERIAL_WATER)
							{
								displayChar = '~';
							}
						}
						else
						{
							displayChar = '_';
						}
						if (world[worldI].identity < numberOfAnimals)
						{
							if (!animals[world[worldI].identity].retired)
							{
								int targetLocalPositionI = isAnimalInSquare(world[worldI].identity , worldX, worldY, worldI);

								if (targetLocalPositionI >= 0)
								{
									if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ != MATERIAL_NOTHING))
									{
										displayChar = '?';
									}
									if ((animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == 0x00))
									{
										displayChar = '0';
									}
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == DIRECTION_L)           { displayChar = '<'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == DIRECTION_DL)          { displayChar = '['; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == DIRECTION_D)           { displayChar = 'V'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == DIRECTION_DR)          { displayChar = ']'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == DIRECTION_R)           { displayChar = '>'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == DIRECTION_UR)          { displayChar = '}'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == DIRECTION_U)           { displayChar = '^'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == DIRECTION_UL)          { displayChar = '{'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_LIVER)           { displayChar = 'L'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_BONE)            { displayChar = 'B'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_WEAPON)          { displayChar = 'W'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_LEAF)            { displayChar = 'P'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_MOUTH)           { displayChar = 'O'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_GONAD)           { displayChar = 'G'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_MUSCLE)          { displayChar = 'M'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_FOOD)     { displayChar = 'F'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_CREATURE) { displayChar = 'C'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_LIGHT)    { displayChar = 'Y'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_RANDOM)   { displayChar = 'R'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_INVERT)   { displayChar = 'I'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_HOME)     { displayChar = 'H'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == ORGAN_SENSOR_PARENT)   { displayChar = 'K'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == GROW_END)                { displayChar = '!'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == GROW_BRANCH)             { displayChar = '@'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == GROW_STRIDE)             { displayChar = '#'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == GROW_SEQUENCE)           { displayChar = '$'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == GROW_ADDOFFSPRINGENERGY) { displayChar = '%'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == MATERIAL_ROCK)           { displayChar = '&'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == MATERIAL_WATER)          { displayChar = '*'; }
									if (animals[   world[worldI].identity  ].body[targetLocalPositionI].organ == MARKER)                  { displayChar = '('; }
								}
							}
						}
					}
				}
			}

			// print borders (this is useful to see if the image is distorted- if the border is not a clean square)
			if (vx == 0 || vx == viewFieldX - 1 )
			{
				displayChar = '|';
			}
			if (vy == 0 || vy == viewFieldY - 1 )
			{
				displayChar = '_';
			}

			printf("%c", displayChar);
		}
		printf("\n");
	}

	float usPerFrame = threadTimer;

	float fps = 1.0f;
	if (!lockfps && usPerFrame > 0)
	{
		fps = (1000000.0f / usPerFrame) ;
	}

	printf( "x%u y%u, %f turns/s | %u/%u animals, %f in, %f out |\n| animal %i energy %f of %u, dd %u, dr %u | t. %u of %u\n",
	        cameraPositionX, cameraPositionY, fps,  populationCount, numberOfAnimals, energyScaleIn, energyScaleOut, cameraTargetCreature, animals[cameraTargetCreature].energy,
	        animals[cameraTargetCreature].mass ,  animals[cameraTargetCreature].damageDone, animals[cameraTargetCreature].damageReceived,  tournamentCounter, tournamentInterval);
}

void populationController()
{

	ZoneScoped;
	if (populationCount > (numberOfAnimals / 2))
	{
		float populationDifference = populationCount - (numberOfAnimals / 2);
		if (numberOfAnimals > 0)
		{
			energyScaleOut = (populationDifference) / numberOfAnimals;
		}
		energyScaleOut += 1.0f;
		energyScaleOut = energyScaleOut * energyScaleOut  * energyScaleOut ;
		energyScaleOut -= 1.0f;
	}
	else
	{
		energyScaleOut = minimumEntropy;
	}
}

void regenerateKnights()
{

	ZoneScoped;
	for (unsigned int i = 0; i < numberOfKnights; ++i)	// initial random creatures.
	{
		unsigned int targetWorldPositionX = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int targetWorldPositionY = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		spawnAnimalIntoSlot( i,  championGenes, targetWorldPositionI, true);
		if (champion < 0 || champion > numberOfAnimals)
		{
			for (int j = 0; j < genomeSize; ++j)
			{
				animals[i].genes[j] = randomLetter();
			}
		}
	}
}

void setupTournamentAnimals()
{

	ZoneScoped;
	for (unsigned int i = 0; i < (numberOfAnimals / 2); ++i)	// initial random creatures.
	{
		unsigned int targetWorldPositionX = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int targetWorldPositionY = extremelyFastNumberFromZeroTo(worldSize - 1);
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		int newAnimal = spawnAnimal( exampleAnimal, targetWorldPositionI, true);
		if (newAnimal >= 0)
		{
			// if (extremelyFastNumberFromZeroTo(1) == 0)
			// {
			// 	// setupExampleAnimal(newAnimal);
			// }
			// else
			// {
			examplePlant(newAnimal);
			// }
		}
	}
}

void setupRandomWorld()
{

	ZoneScoped;
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


		unsigned int targetWorldPositionX = cameraPositionX + (viewFieldX / 2);
		unsigned int targetWorldPositionY = cameraPositionY + (viewFieldY / 2);
		unsigned int targetWorldPositionI = ( targetWorldPositionY * worldSize ) + targetWorldPositionX;
		int animalIndex = spawnAnimal( exampleAnimal, targetWorldPositionI, false);
		if (animalIndex >= 0)
		{
			cameraTargetCreature = animalIndex;




			setupExampleAnimal(animalIndex);

		}

		unsigned int foodpos = targetWorldPositionI + (10 * worldSize) + 10;

		world[foodpos].material = MATERIAL_FOOD;
	}
	else if (worldToLoad == WORLD_RANDOM)
	{
		printf("placing materials\n");
		// initial random materials.
		for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		{
			unsigned int x = worldPositionI % worldSize;
			unsigned int y = worldPositionI / worldSize;

			// if (extremelyFastNumberFromZeroTo(1000) == 0)
			// {
			// 	world[worldPositionI].material = MATERIAL_LIGHT;
			// 	for (unsigned int n = 0; n < nNeighbours; ++n)
			// 	{
			// 		unsigned int worldNeighbour = worldPositionI + neighbourOffsets[n];
			// 		if (worldNeighbour < worldSquareSize)
			// 		{
			// 			world[worldNeighbour].material = MATERIAL_LIGHT;
			// 		}
			// 	}
			// }
			if (extremelyFastNumberFromZeroTo(50) == 0)
			{
				world[worldPositionI].material = MATERIAL_FOOD;
			}
		}

		printf("growing materials \n");
		// for (int i = 0; i < 10; ++i)
		// {

		// 	printf("%i\n", i);
		// 	// expand the light
		// 	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		// 	{
		// 		unsigned int nRockNeighbours = 0;
		// 		for (unsigned int n = 0; n < nNeighbours; ++n)
		// 		{
		// 			unsigned int worldNeighbour = worldPositionI + neighbourOffsets[n];
		// 			if (worldNeighbour < worldSquareSize)
		// 			{
		// 				if (world[worldNeighbour].material == MATERIAL_LIGHT)
		// 				{
		// 					nRockNeighbours++;
		// 				}
		// 			}
		// 		}
		// 		if (nRockNeighbours == 1) {  world[worldPositionI].material = MARKER;}
		// 	}
		// 	for (unsigned int worldPositionI = 0; worldPositionI < worldSquareSize; ++worldPositionI)
		// 	{
		// 		if (world[worldPositionI].material == MARKER)
		// 		{
		// 			world[worldPositionI].material = MATERIAL_LIGHT;
		// 		}
		// 	}
		// }

		setupTournamentAnimals();
		regenerateKnights();
	}
}

void model()
{

	ZoneScoped;
	auto start = std::chrono::steady_clock::now();
	// unsigned int newPopulationCount = 0;
	// for (int animalIndex = 0; animalIndex < numberOfAnimals; ++animalIndex)
	// {
	// 	if (!animals[animalIndex].retired)
	// 	{
	// 		newPopulationCount ++;
	// 		animalTurn(animalIndex);
	// 	}
	// }
	// populationCount = newPopulationCount;

	computeAllAnimalsOneTurn();

	populationController();

	if (tournament)
	{
		if (tournamentCounter >= tournamentInterval )
		{
			tournamentCounter = 0;
			regenerateKnights();
		}
		else
		{
			tournamentCounter++;
		}
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
		camera();
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
		FrameMark;
	}
}

int main()
{
	ZoneScoped;
	setupRandomWorld();
	boost::thread t7{ interfaceSupervisor };
	modelSupervisor();

}
