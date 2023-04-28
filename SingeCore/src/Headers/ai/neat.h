#pragma once

#include "csharp.h"
#include "singine/array.h"

// the integral or floating point value type that should be used for math for all the AI stuff
// default is: float (for speed)
typedef float ai_number;

typedef struct gene gene;

typedef gene* Gene;

struct gene
{
	size_t Id;
	bool Enabled;
	size_t StartNodeIndex;
	size_t EndNodeIndex;
	ai_number Weight;
};

typedef struct organism organism;

typedef organism* Organism;

typedef struct species species;

typedef species* Species;

DEFINE_ARRAY(gene);
DEFINE_ARRAY(Organism);
DEFINE_ARRAY(Species);

struct organism
{
	// Id of the organism
	size_t Id;
	Species Parent;
	ARRAY(gene) Genes;
	// the nodes of this organism
	// format:
	// input | output | otherNodes
	ai_number* Nodes;
	size_t NodeCount;
	// The last calculated fitness
	ai_number Fitness;
	size_t InputNodeCount;
	size_t OutputNodeCount;
};

typedef struct population population;

typedef population* Population;

struct species
{
	size_t Id;
	Population Parent;
	ARRAY(Organism) Organisms;
	size_t InputNodeCount;
	size_t OutputNodeCount;
};

struct population
{
	size_t Id;
	// Array of GLOBAL genes for all organisms within this population
	Gene Genes;
	// Number of GLOBAL genes for all organisms within this population
	size_t GeneCount;
	ARRAY(Species) Species;
	// the chance, checked once per fitness eval, of a node mutation 
	// range: [0f - 1f]
	ai_number AddNodeMutationChance;
	// the chance, checked once per fitness eval, of a connection mutation 
	// range: [0f - 1f]
	ai_number AddConnectionMutationChance;
	// the chance, checked once per gene per fitness eval, of a weight mutating to another weight
	// range: [0f - 1f]
	ai_number WeightMutationChance;
	// the chance, checked once per gene per fitness eval, of a weight mutating to another weight
	// range: [0f - 1f]
	ai_number NewWeightMutationChance;
	// The importance scalar when comparing two organisms for similarity
	ai_number ExcessGeneImportance;
	// The importance scalar when comparing two organisms for similarity
	ai_number DisjointGeneImportance;
	// The importance scalar when comparing two organisms for similarity
	ai_number MatchingGeneImportance;
	// The threshold in similarity that two organisms must have to be considered part of the same
	// species
	ai_number SimilarityThreshold;
	// the transfer function that should be used between all nodes in the network
	ai_number(*TransferFunction)(ai_number input);
};

extern struct _neatMethods
{
	// How big the backing array of the gene pool should be
	size_t DefaultGenePoolSize;
	// the chance, checked once per fitness eval, of a node mutation 
	// range: [0f - 1f]
	ai_number DefaultAddNodeMutationChance;
	// the chance, checked once per fitness eval, of a connection mutation 
	// range: [0f - 1f]
	ai_number DefaultAddConnectionMutationChance;
	// the chance, checked once per gene per fitness eval, of a weight mutating to another weight
	// range: [0f - 1f]
	ai_number DefaultWeightMutationChance;
	// the chance, checked once per gene per fitness eval, of a weight mutating to another weight
	// range: [0f - 1f]
	ai_number DefaultNewWeightMutationChance;
	// the chance, checked once per gene per fitness eval, of all weights in a organism mutating to a similar weight
	// range: [0f - 1f]
	ai_number DefaultUniformWeightMutationChance;
	// The importance scalar when comparing two organisms for similarity
	ai_number DefaultExcessGeneImportance;
	// The importance scalar when comparing two organisms for similarity
	ai_number DefaultDisjointGeneImportance;
	// The importance scalar when comparing two organisms for similarity
	ai_number DefaultMatchingGeneImportance;
	// The threshold in similarity that two organisms must have to be considered part of the same
	// species
	ai_number DefaultSimilarityThreshold;
	// the transfer function that should be used between all nodes in the network
	ai_number(*DefaultTransferFunction)(ai_number input);
	Population(*Create)(size_t populationSize, size_t inputNodeCount, size_t outputNodeCount);
	void (*Dispose)(Population);
} Neat;