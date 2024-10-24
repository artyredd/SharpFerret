#pragma once

#include "core/csharp.h"
#include "core/array.h"

#include "core/math/bigMatrix.h"


// the integral or floating point value type that should be used for math for all the AI stuff
// default is: float (for speed)
typedef float ai_number;

typedef struct gene gene;

typedef gene* Gene;

struct gene
{
	ulong Id;
	bool Enabled;
	ulong StartNodeIndex;
	ulong EndNodeIndex;
	ai_number Weight;
};

typedef struct organism organism;

typedef organism* Organism;

typedef struct species species;

typedef species* Species;

DEFINE_CONTAINERS(gene);
DEFINE_CONTAINERS(Organism);
DEFINE_CONTAINERS(Species);
DEFINE_CONTAINERS(ai_number);



struct organism
{
	// Id of the organism
	ulong Id;
	// The generation this organism was born
	ulong Generation;
	Species Parent;
	array(gene) Genes;
	// Big matrix containing all the weights
	BigMatrix WeightMatrix;
	// Contains the results of the organism after propogation
	array(ai_number) Outputs;
	ulong NodeCount;
	// The last calculated fitness
	ai_number Fitness;
	ulong InputNodeCount;
	ulong OutputNodeCount;
};

typedef struct population population;

typedef population* Population;

struct species
{
	ulong Id;
	// The average fitness for all the organisms within the species
	ai_number AverageFitness;
	// The highest fitness this species has achieved
	ai_number MaximumFitness;
	// The Generation this species was created
	ulong StartGeneration;
	// The current highest generation of an organism within this species
	ulong Generation;
	// The generation where the maximum fitness of this species went up
	ulong LastGenerationWhereFitnessImproved;
	Population Parent;
	array(Organism) Organisms;
	// This is an organism from the previous generation that is used
	// for speciation
	Organism ReferenceOrganism;
	ulong InputNodeCount;
	ulong OutputNodeCount;
};

struct population
{
	// The next available organism Id for this population
	ulong NextId;
	// The current generation this population is on
	ulong Generation;
	// The number of organisms alloted to this population
	ulong Count;
	ulong InputNodeCount;
	ulong OutputNodeCount;
	// The sum of all the average fitnesses within the population
	// used to determine how many organisms get alloted to species
	ai_number SummedAverageFitness;
	array(gene) Genes;
	array(Species) Species;
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
	// The percentage of organisms that should be culled from each species to produce new offspring
	ai_number OrganismCullingRate;
	// The number of generations a species can exist while not improving it's maximum fitness
	ulong GenerationsBeforeStagnation;
	// The default ratio represented as a percentage [0-1], of how much of the new organisms in a generation
	// should be created using crossover or just mutations with no crossover
	ai_number MatingWithCrossoverChance;
	// the transfer function that should be used between all nodes in the network
	// Takes in a reference to a number, should mutate it
	void(*TransferFunction)(ai_number* input);
	ai_number(*FitnessFunction)(const Organism organism);
};

extern struct _neatMethods
{
	// How big the backing array of the gene pool should be
	ulong DefaultGenePoolSize;
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
	// The percentage of organisms that should be culled from each species to produce new offspring
	ai_number DefaultOrganismCullingRate;
	// The number of generations a species can exist while not improving it's maximum fitness
	ulong DefaultGenerationsBeforeStagnation;
	// The default ratio represented as a percentage [0-1], of how much of the new organisms in a generation
	// should be created using crossover or just mutations with no crossover
	ai_number DefaultMatingWithCrossoverRatio;
	// the transfer function that should be used between all nodes in the network
	// Takes in a reference to a number, should mutate it
	void(*DefaultTransferFunction)(ai_number* input);
	Population(*Create)(ulong populationSize, ulong inputNodeCount, ulong outputNodeCount);
	// Propogates inputdata forward through all the organisms within a population
	void (*Propogate)(Population, array(ai_number) inputData);
	void (*CalculateFitness)(Population);
	// Drops the lowest fitness organisms within a population and then mates the top performing
	// organisms with eachother to replace the dropped organisms
	void (*CrossMutateAndSpeciate)(Population);
	// Makes sure all the organisms in a population is in the correct species
	void (*Speciate)(Population);
	void (*Dispose)(Population);
	void (*RunUnitTests)();
} Neat;