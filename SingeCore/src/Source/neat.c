#pragma once

#include "ai/neat.h"

#include "singine/memory.h"

#include "singine/random.h"

#include "Tests/cunit.h"

#include <math.h>

#define DOUBLE_E 2.71828

#define RANDOM_NUMBER_GEN() Random.NextFloat()

private Population CreatePopulation(size_t populationSize, size_t inputNodeCount, size_t outputNodeCount);
private void DisposePopulation(Population);
private ai_number SigmoidalTransferFunction(ai_number number);
private void RunUnitTests();
private void Propogate(Population, ARRAY(ai_number));
private void CalculateFitness(Population, ARRAY(ai_number));
private void CrossMutateAndSpeciate(Population);
private void SpeciatePopulation(Population);

struct _neatMethods Neat = {
	.DefaultGenePoolSize = 1024,
	.DefaultAddConnectionMutationChance = 0.05f,
	.DefaultAddNodeMutationChance = 0.03f,
	.DefaultWeightMutationChance = 0.8f,
	.DefaultNewWeightMutationChance = 0.1f,
	.DefaultDisjointGeneImportance = 1.0f,
	.DefaultExcessGeneImportance = 1.0f,
	.DefaultMatchingGeneImportance = 0.4f,
	.DefaultOrganismCullingRate = 0.5f,
	.DefaultGenerationsBeforeStagnation = 15,
	.DefaultMatingWithCrossoverRatio = 0.75f,
	.DefaultTransferFunction = SigmoidalTransferFunction,
	.Create = CreatePopulation,
	.Dispose = DisposePopulation,
	.RunUnitTests = RunUnitTests,
	.Propogate = Propogate,
	.CalculateFitness = CalculateFitness,
	.CrossMutateAndSpeciate = CrossMutateAndSpeciate,
	.Speciate = SpeciatePopulation
};

TYPE_ID(Gene);
TYPE_ID(Species);
TYPE_ID(Organism);
TYPE_ID(Population);

private bool TryFindGene(Population species, gene gene, size_t* out_geneIndex)
{
	for (size_t i = 0; i < species->Genes->Count; i++)
	{
		Gene globalGene = &species->Genes->Values[i];
		if (gene.StartNodeIndex is globalGene->StartNodeIndex && gene.EndNodeIndex is globalGene->EndNodeIndex)
		{
			*out_geneIndex = globalGene->Id;
			return true;
		}
	}

	return false;
}

private size_t AddGlobalGeneToPopulation(Population population, gene gene)
{
	size_t newId = population->Genes->Count;

	Arrays.Append((Array)population->Genes, &gene);

	population->Genes->Values[newId] = gene;

	return newId;
}

private gene CreateGene(Population population, size_t startNodeIndex, size_t endNodeIndex)
{
	gene result = {
		.Id = 0,
		.Enabled = true,
		.StartNodeIndex = startNodeIndex,
		.EndNodeIndex = endNodeIndex,
		.Weight = RANDOM_NUMBER_GEN()
	};

	size_t existingGeneId;
	if (TryFindGene(population, result, &existingGeneId))
	{
		result.Id = existingGeneId;
	}
	else
	{
		result.Id = AddGlobalGeneToPopulation(population, result);
	}

	return result;
}

private size_t GetNewNodeId(Organism organism)
{
	size_t newNodeId = organism->NodeCount;

	organism->NodeCount = safe_add(newNodeId, 1);

	size_t previousSize = sizeof(ai_number) * newNodeId;
	size_t newSize = sizeof(ai_number) * organism->NodeCount;

	Memory.ReallocOrCopy(&organism->Nodes, previousSize, newSize, Memory.GenericMemoryBlock);

	// give it a random bias
	organism->Nodes[newNodeId] = Random.NextFloat();

	return newNodeId;
}

private void AddGeneToOrganism(Organism organism, gene gene)
{
	Arrays.Append((Array)organism->Genes, &gene);
}

private void MutateAddNode(Organism organism)
{
	// create the node
	// get the highest id of nodes in the organism
	size_t endNode = GetNewNodeId(organism);

	// choose a random node to point to it
	size_t startNode = Random.BetweenSize_t(0, endNode - 1);

	gene newGene = CreateGene(organism->Parent->Parent, startNode, endNode);

	AddGeneToOrganism(organism, newGene);
}

private void MutateAddConnection(Organism organism)
{
	size_t startIndex = 0;
	size_t endIndex = 0;

	// dont point to the same node
	do
	{
		startIndex = Random.BetweenSize_t(0, safe_subtract(organism->NodeCount, 1));
		endIndex = Random.BetweenSize_t(0, safe_subtract(organism->NodeCount, 1));
	} while (startIndex is endIndex);

	gene gene = CreateGene(organism->Parent->Parent, startIndex, endIndex);

	Arrays.Append((Array)organism->Genes, &gene);
}

private void MutateWeights(Organism organism)
{
	Population population = organism->Parent->Parent;

	ai_number uniformMutationValue = RANDOM_NUMBER_GEN();

	for (size_t geneIndex = 0; geneIndex < organism->Genes->Count; geneIndex++)
	{
		Gene gene = &organism->Genes->Values[geneIndex];

		if (Random.Chance(population->WeightMutationChance))
		{
			if (Random.Chance(population->NewWeightMutationChance))
			{
				gene->Weight = RANDOM_NUMBER_GEN();
			}
			else
			{
				gene->Weight *= uniformMutationValue;
			}
		}
	}
}

private Organism CreateOrganism(size_t inputNodeCount, size_t outputNodeCount)
{
	Organism result = Memory.Alloc(sizeof(organism), OrganismTypeId);

	result->Genes = (ARRAY(gene))Arrays.Create(sizeof(gene), 0, GeneTypeId);
	result->Id = 0;
	result->InputNodeCount = inputNodeCount;
	result->Nodes = Memory.Alloc(sizeof(ai_number) * (inputNodeCount + outputNodeCount), Memory.GenericMemoryBlock);
	result->NodeCount = (inputNodeCount + outputNodeCount);
	result->OutputNodeCount = outputNodeCount;
	result->Parent = null;

	return result;
}

private Species CreateSpecies(size_t inputNodeCount, size_t outputNodeCount, size_t organismCount)
{
	Species result = Memory.Alloc(sizeof(species), SpeciesTypeId);

	result->Id = 0;
	result->InputNodeCount = inputNodeCount;
	result->OutputNodeCount = outputNodeCount;
	result->Organisms = (ARRAY(Organism))Arrays.Create(sizeof(Organism), organismCount, OrganismTypeId);

	for (size_t i = 0; i < result->Organisms->Count; i++)
	{
		Organism newOrganism = CreateOrganism(inputNodeCount, outputNodeCount);

		newOrganism->Id = i;

		result->Organisms->Values[i] = newOrganism;
	}

	return result;
}

private Population CreatePopulation(size_t populationSize, size_t inputNodeCount, size_t outputNodeCount)
{
	REGISTER_TYPE(Gene);
	REGISTER_TYPE(Species);
	REGISTER_TYPE(Organism);

	Population result = Memory.Alloc(sizeof(population), SpeciesTypeId);

	population population = {
		.NextId = 0,
		.SummedAverageFitness = 0,
		.Genes = (ARRAY(gene))Arrays.Create(sizeof(gene), Neat.DefaultGenePoolSize, GeneTypeId),
		.Species = (ARRAY(Species))Arrays.Create(sizeof(Species), 1, SpeciesTypeId),
		.AddConnectionMutationChance = Neat.DefaultAddConnectionMutationChance,
		.AddNodeMutationChance = Neat.DefaultAddNodeMutationChance,
		.NewWeightMutationChance = Neat.DefaultNewWeightMutationChance,
		.WeightMutationChance = Neat.DefaultWeightMutationChance,
		.DisjointGeneImportance = Neat.DefaultDisjointGeneImportance,
		.ExcessGeneImportance = Neat.DefaultExcessGeneImportance,
		.MatchingGeneImportance = Neat.DefaultMatchingGeneImportance,
		.SimilarityThreshold = Neat.DefaultSimilarityThreshold,
		.GenerationsBeforeStagnation = Neat.DefaultGenerationsBeforeStagnation,
		.OrganismCullingRate = Neat.DefaultOrganismCullingRate,
		.Generation = 1,
		.Count = populationSize,
		.TransferFunction = Neat.DefaultTransferFunction,
		.FitnessFunction = null
	};

	for (size_t i = 0; i < population.Species->Count; i++)
	{
		Species species = CreateSpecies(inputNodeCount, outputNodeCount, populationSize);

		population.Species->Values[i] = species;

		species->Id = i;
		species->Parent = result;

		for (size_t organism = 0; organism < species->Organisms->Count; organism++)
		{
			species->Organisms->Values[organism]->Parent = population.Species->Values[i];
		}
	}

	*result = population;

	return result;
}

private size_t GetHighestGeneId(const ARRAY(gene) genes)
{
	size_t largestId = 0;
	for (size_t geneIndex = 0; geneIndex < genes->Count; geneIndex++)
	{
		if (genes->Values[geneIndex].Id > largestId)
		{
			largestId = genes->Values[geneIndex].Id;
		}
	}

	return largestId;
}

// returns true when the gene array contain the given gene
private bool ContainsGeneId(size_t geneId, const ARRAY(gene) genes)
{
	for (size_t geneIndex = 0; geneIndex < genes->Count; geneIndex++)
	{
		if (genes->Values[geneIndex].Id is geneId)
		{
			return true;
		}
	}

	return false;
}

private size_t GetDisjointGeneCount(const ARRAY(gene) leftGenes, const ARRAY(gene) rightGenes)
{
	size_t smallestCount = min(leftGenes->Count, rightGenes->Count);
	size_t disjointCount = 0;
	for (size_t leftIndex = 0; leftIndex < smallestCount; leftIndex++)
	{
		size_t leftId = leftGenes->Values[leftIndex].Id;

		if (ContainsGeneId(leftId, rightGenes) is false)
		{
			safe_increment(disjointCount);
		}
	}

	return disjointCount;
}

private ai_number GetAverageDifferenceBetweenWeights(const ARRAY(gene) leftGenes, const ARRAY(gene) rightGenes)
{
	size_t count = 0;
	ai_number value = 0;

	for (size_t leftIndex = 0; leftIndex < leftGenes->Count; leftIndex++)
	{
		const Gene leftGene = &leftGenes->Values[leftIndex];

		for (size_t rightIndex = 0; rightIndex < rightGenes->Count; rightIndex++)
		{
			const Gene rightGene = &rightGenes->Values[rightIndex];

			if (leftGene->Id == rightGene->Id)
			{
				count = safe_add(count, 1);
				value += (leftGene->Weight - rightGene->Weight);

				break;
			}
		}
	}

	return value / (ai_number)count;
}

private size_t GetExcessGeneCount(const ARRAY(gene) left, const ARRAY(gene) right)
{
	size_t highestLeftId = GetHighestGeneId(left);
	size_t highestRightId = GetHighestGeneId(right);

	return safe_subtract(highestRightId, highestLeftId);
}

private size_t GetBothDisjointGeneCount(const ARRAY(gene) left, const ARRAY(gene) right)
{
	size_t leftDisjointCount = GetDisjointGeneCount(left, right);
	size_t rightDisjointCount = GetDisjointGeneCount(right, left);

	return safe_add(leftDisjointCount, rightDisjointCount);
}

// gets the similarity of two gene pools
private ai_number GetSimilarity(const Population population, const Organism leftOrganism, const Organism rightOrganism)
{
	ai_number largestCount = (ai_number)max(leftOrganism->Genes->Count, rightOrganism->Genes->Count);

	ai_number excessSimilarity = (population->ExcessGeneImportance * (ai_number)GetExcessGeneCount(leftOrganism->Genes, rightOrganism->Genes)) / largestCount;

	ai_number disjointSimilarity = (population->DisjointGeneImportance * (ai_number)GetBothDisjointGeneCount(leftOrganism->Genes, rightOrganism->Genes)) / largestCount;

	ai_number weightSimilarity = population->MatchingGeneImportance * GetAverageDifferenceBetweenWeights(leftOrganism->Genes, rightOrganism->Genes);

	return excessSimilarity + disjointSimilarity + weightSimilarity;
}

// returns true when two organisms are similar
private bool SimilarOrganisms(const Population population, const Organism leftOrganism, const Organism rightOrganism)
{
	return GetSimilarity(population, leftOrganism, rightOrganism) <= population->SimilarityThreshold;
}

private void RemoveDisimilarOrganismsFromSpecies(Population population, Species species, ARRAY(Organism) destinationArray)
{
	Organism referenceOrganism = species->ReferenceOrganism;
	for (size_t organismIndex = 1; organismIndex < species->Organisms->Count; organismIndex++)
	{
		Organism organism = species->Organisms->Values[organismIndex];
		if (GetSimilarity(population, referenceOrganism, organism) > population->SimilarityThreshold)
		{
			// add to the array to get sorted
			Arrays.Append((ARRAY(void))destinationArray, &organism);

			// remove it from this array
			Arrays.RemoveIndex((ARRAY(void))species->Organisms, organismIndex);

			// go backwards and restart at this index since we removed this index from the array
			safe_decrement(organismIndex);
		}
	}
}

private void SortOrganismsIntoSpecies(Population population, ARRAY(Organism) organisms)
{
	size_t organismIndex = organisms->Count;
	while (organismIndex-- > 0)
	{
		Organism organism = organisms->Values[organismIndex];

		bool sorted = false;
		for (size_t speciesIndex = 0; speciesIndex < population->Species->Count; speciesIndex++)
		{
			Species species = population->Species->Values[speciesIndex];

			if (species->Organisms->Count is 0)
			{
				// species should not have zero members here
				throw(InvalidLogicException);
			}

			if (SimilarOrganisms(population, organism, species->ReferenceOrganism))
			{
				Arrays.Append((ARRAY(void))species->Organisms, &organism);
				sorted = true;
				break;
			}
		}

		if (sorted)
		{
			continue;
		}

		// create a new species
		Species newSpecies = CreateSpecies(organism->InputNodeCount, organism->OutputNodeCount, 0);

		Arrays.Append((Array)newSpecies->Organisms, &organism);

		Arrays.Append((Array)population->Species, &newSpecies);
	}
}

// Goes through all the organisms of the population
// and ensures that they are properly sorted into
// species according to their similarities
private void SpeciatePopulation(Population population)
{
	// create a spot to store the organisms needing sorting
	ARRAY(Organism) organismsNeedingSorting = (ARRAY(Organism))Arrays.Create(sizeof(Organism), 0, OrganismTypeId);

	for (size_t speciesIndex = 0; speciesIndex < population->Species->Count; speciesIndex++)
	{
		Species species = population->Species->Values[speciesIndex];

		// Remove the species if it doesnt have any organisms
		if (species->Organisms->Count is 0)
		{
			Arrays.RemoveIndex((ARRAY(void))population->Species, speciesIndex);
			safe_decrement(speciesIndex);
			continue;
		}

		// go through all the organisms of this species
		// check to make sure they're all similar to the first organism
		// in the species, if they're not sort them into the other species
		RemoveDisimilarOrganismsFromSpecies(population, species, organismsNeedingSorting);
	}

	SortOrganismsIntoSpecies(population, organismsNeedingSorting);
}

private ai_number GetAdjustedFitness(const Organism organism)
{
	return organism->Fitness / (ai_number)organism->Parent->Organisms->Count;
}

private ai_number SigmoidalTransferFunction(ai_number number)
{
	return (ai_number)((ai_number)1.0 / ((ai_number)1.0 + pow((ai_number)DOUBLE_E, (ai_number)-4.9 * number)));
}

private Gene GetGeneWithId(ARRAY(gene) genes, size_t id)
{
	for (size_t i = 0; i < genes->Count; i++)
	{
		if (genes->Values[i].Id is id)
		{
			return &genes->Values[i];
		}
	}

	return null;
}

private Organism BreedOrganisms(const Organism left, const Organism right)
{
	// matching genes are inherited randomly between organisms
	// disjoint and excess genes are inherited from the more fit parent
	const Organism moreFitOrganism = left->Fitness > right->Fitness ? left : right;
	const Organism lessFitOrganism = left->Fitness < right->Fitness ? left : right;

	Organism newOrganism = CreateOrganism(left->InputNodeCount, right->InputNodeCount);

	for (size_t geneIndex = 0; geneIndex < moreFitOrganism->Genes->Count; geneIndex++)
	{
		const Gene moreFitGene = &moreFitOrganism->Genes->Values[geneIndex];

		const bool matching = ContainsGeneId(moreFitGene->Id, lessFitOrganism->Genes);

		// see if its a matching gene
		if (matching && Random.NextBool())
		{
			const Gene lessFitGene = GetGeneWithId(lessFitOrganism->Genes, moreFitGene->Id);

			if (lessFitGene is null)
			{
				throw(InvalidLogicException);
			}

			Arrays.Append((Array)newOrganism->Genes, lessFitGene);
		}
		else
		{
			Arrays.Append((Array)newOrganism->Genes, moreFitGene);
		}
	}

	return newOrganism;
}

private void DisposeOrganism(Organism organism)
{
	Arrays.Dispose((Array)organism->Genes);
	Arrays.Dispose((Array)organism->Nodes);

	Memory.Free(organism, OrganismTypeId);
}

private void DisposeSpecies(Species species)
{
	for (size_t organismIndex = 0; organismIndex < species->Organisms->Count; organismIndex++)
	{
		DisposeOrganism(species->Organisms->Values[organismIndex]);
	}

	DisposeOrganism(species->ReferenceOrganism);

	Arrays.Dispose((Array)species->Organisms);

	Memory.Free(species, SpeciesTypeId);
}

private void DisposePopulation(Population population)
{
	if (population is null)
	{
		return;
	}

	for (size_t speciesIndex = 0; speciesIndex < population->Species->Count; speciesIndex++)
	{
		Species species = population->Species->Values[speciesIndex];

		DisposeSpecies(species);
	}

	Arrays.Dispose((Array)population->Species);
	Arrays.Dispose((Array)population->Genes);

	Memory.Free(population, SpeciesTypeId);
}

private void Propogate(Population population, ARRAY(ai_number) inputData)
{
	throw(NotImplementedException);
}

private void CalculateFitness(Population population, ARRAY(ai_number) inputData)
{
	throw(NotImplementedException);
}

private bool OrganismFitnessComparator(Organism left, Organism right)
{
	return left->Fitness < right->Fitness;
}

private void SortOrganismsByFitness(ARRAY(Organism) organisms)
{
	Arrays.InsertionSort((Array)organisms, OrganismFitnessComparator);
}

// removes the poorest performing organisms within a species
// returns the number of organisms within this species that
// were removed
private size_t RemovePoorFitnessOrganisms(Population population, Species species)
{
	// sort by fitness
	SortOrganismsByFitness(species->Organisms);

	// determine how many to remove
	const size_t count = (size_t)((ai_number)species->Organisms->Count * population->OrganismCullingRate);
	const size_t stopIndex = safe_subtract(species->Organisms->Count, count);

	size_t index = species->Organisms->Count;
	while (index-- > stopIndex)
	{
		DisposeOrganism(species->Organisms->Values[index]);
		Arrays.RemoveIndex((Array)species->Organisms, index);
	}

	return count;
}

private void CalculatePopulationFitnesses(Population population)
{
	ai_number summedAverage = 0;
	for (size_t speciesIndex = 0; speciesIndex < population->Species->Count; speciesIndex++)
	{
		Species species = population->Species->Values[speciesIndex];
		ai_number speciesAverage = 0;

		for (size_t organismIndex = 0; organismIndex < species->Organisms->Count; organismIndex++)
		{
			ai_number fitness = species->Organisms->Values[organismIndex]->Fitness;
			if (species->MaximumFitness < fitness)
			{
				species->MaximumFitness = fitness;
				species->LastGenerationWhereFitnessImproved = safe_subtract(population->Generation, 1);
			}

			speciesAverage += fitness;
		}

		species->AverageFitness = speciesAverage / species->Organisms->Count;
		summedAverage += speciesAverage;
	}

	population->SummedAverageFitness = summedAverage;
}

private void RemoveStagnatingSpecies(Population population)
{
	size_t speciesIndex = population->Species->Count;
	while (speciesIndex-- > 0)
	{
		Species species = population->Species->Values[speciesIndex];

		bool removeSpecies = safe_subtract(species->Generation, species->LastGenerationWhereFitnessImproved) > population->GenerationsBeforeStagnation;

		if (removeSpecies)
		{
			// if a species isn't allowed to reproduce anymore 
			// allot the portion of organisms to the rest
			// of the species
			population->SummedAverageFitness -= species->AverageFitness;

			// cull the whole species
			DisposeSpecies(species);

			Arrays.RemoveIndex((Array)population->Species, speciesIndex);
		}
	}
}

private void ReplenishSpecies(Population population, Species species, size_t count)
{
	throw(NotImplementedException);
}

private void CrossMutateAndSpeciate(Population population)
{
	// since we're crossing and mutating we should increase our generation
	safe_increment(population->Generation);

	// go through and average fitnesses and sum them so we can determine
	// how much each species can reproduce
	CalculatePopulationFitnesses(population);

	RemoveStagnatingSpecies(population);

	for (size_t speciesIndex = 0; speciesIndex < population->Species->Count; speciesIndex++)
	{
		Species species = population->Species->Values[speciesIndex];

		// since we're allowed to reproduce increase the generation of the species
		species->Generation = population->Generation;

		ai_number allotedPercentage = species->AverageFitness / population->SummedAverageFitness;

		size_t allotedCount = (size_t)(allotedPercentage * (ai_number)population->Count);

		// kill the bottom performing organisms within the species
		RemovePoorFitnessOrganisms(population, species);

		// replenish UP TO the allotted count even if we removed more then the alloted
		ReplenishSpecies(population, species, allotedCount);
	}

	SpeciatePopulation(population);
}

// Tests
ARRAY(gene) ExampleGenome_0()
{
	ARRAY(gene) genome = (ARRAY(gene))Arrays.Create(sizeof(gene), 5, GeneTypeId);

	genome->Values[0] = (gene)
	{
		.Id = 1,
		.Enabled = true,
		.StartNodeIndex = 1,
		.EndNodeIndex = 4,
		.Weight = 1.0
	};
	genome->Values[1] = (gene)
	{
		.Id = 2,
		.Enabled = true,
		.StartNodeIndex = 2,
		.EndNodeIndex = 4,
		.Weight = 1.0
	};
	genome->Values[2] = (gene)
	{
		.Id = 4,
		.Enabled = false,
		.StartNodeIndex = 2,
		.EndNodeIndex = 5,
		.Weight = 1.0
	};
	genome->Values[3] = (gene)
	{
		.Id = 5,
		.Enabled = true,
		.StartNodeIndex = 3,
		.EndNodeIndex = 5,
		.Weight = 1.0
	};
	genome->Values[4] = (gene)
	{
		.Id = 6,
		.Enabled = true,
		.StartNodeIndex = 4,
		.EndNodeIndex = 5,
		.Weight = 1.0
	};

	return genome;
}

ARRAY(gene) ExampleGenome_1()
{
	ARRAY(gene) genome = (ARRAY(gene))Arrays.Create(sizeof(gene), 7, GeneTypeId);

	genome->Values[0] = (gene)
	{
		.Id = 1,
		.Enabled = false,
		.StartNodeIndex = 1,
		.EndNodeIndex = 4,
		.Weight = -1.0
	};
	genome->Values[1] = (gene)
	{
		.Id = 2,
		.Enabled = true,
		.StartNodeIndex = 2,
		.EndNodeIndex = 4,
		.Weight = -1.0
	};
	genome->Values[2] = (gene)
	{
		.Id = 3,
		.Enabled = true,
		.StartNodeIndex = 3,
		.EndNodeIndex = 4,
		.Weight = -1.0
	};
	genome->Values[3] = (gene)
	{
		.Id = 4,
		.Enabled = false,
		.StartNodeIndex = 2,
		.EndNodeIndex = 5,
		.Weight = -1.0
	};
	genome->Values[4] = (gene)
	{
		.Id = 6,
		.Enabled = true,
		.StartNodeIndex = 4,
		.EndNodeIndex = 5,
		.Weight = -1.0
	};
	genome->Values[5] = (gene)
	{
		.Id = 7,
		.Enabled = true,
		.StartNodeIndex = 1,
		.EndNodeIndex = 6,
		.Weight = -1.0
	};
	genome->Values[6] = (gene)
	{
		.Id = 8,
		.Enabled = true,
		.StartNodeIndex = 6,
		.EndNodeIndex = 4,
		.Weight = -1.0
	};

	return genome;
}

ARRAY(gene) ExampleGenome_2()
{
	ARRAY(gene) genome = (ARRAY(gene))Arrays.Create(sizeof(gene), 7, GeneTypeId);

	genome->Values[0] = (gene)
	{
		.Id = 1,
		.Enabled = true,
		.StartNodeIndex = 1,
		.EndNodeIndex = 4,
		.Weight = -1.0
	};
	genome->Values[1] = (gene)
	{
		.Id = 2,
		.Enabled = true,
		.StartNodeIndex = 2,
		.EndNodeIndex = 4,
		.Weight = -1.0
	};
	genome->Values[2] = (gene)
	{
		.Id = 3,
		.Enabled = true,
		.StartNodeIndex = 3,
		.EndNodeIndex = 4,
		.Weight = -1.0
	};
	genome->Values[3] = (gene)
	{
		.Id = 4,
		.Enabled = false,
		.StartNodeIndex = 2,
		.EndNodeIndex = 5,
		.Weight = -1.0
	};
	genome->Values[4] = (gene)
	{
		.Id = 6,
		.Enabled = true,
		.StartNodeIndex = 4,
		.EndNodeIndex = 5,
		.Weight = -1.0
	};
	genome->Values[5] = (gene)
	{
		.Id = 7,
		.Enabled = true,
		.StartNodeIndex = 1,
		.EndNodeIndex = 6,
		.Weight = -1.0
	};
	genome->Values[6] = (gene)
	{
		.Id = 8,
		.Enabled = true,
		.StartNodeIndex = 6,
		.EndNodeIndex = 4,
		.Weight = -1.0
	};

	return genome;
}

TEST(GetExcessGeneCount)
{
	ARRAY(gene) left = ExampleGenome_0();

	ARRAY(gene) right = ExampleGenome_1();

	size_t expected = 2;

	IsEqual(expected, GetExcessGeneCount(left, right), "%lli");

	return true;
}

TEST(GetBothDisjointGeneCount)
{
	ARRAY(gene) left = ExampleGenome_0();

	ARRAY(gene) right = ExampleGenome_1();

	size_t expected = 2;

	IsEqual(expected, GetBothDisjointGeneCount(left, right), "%lli");

	return true;
}

TEST(GetAverageDifferenceBetweenWeights)
{
	ARRAY(gene) left = ExampleGenome_0();

	ARRAY(gene) right = ExampleGenome_1();

	ai_number expected = 0;

	IsApproximate(expected, GetAverageDifferenceBetweenWeights(left, right), "%lf");

	return true;
}

TEST(GetSimilarity)
{
	organism left = {
		.Genes = ExampleGenome_0()
	};

	organism right = {
		.Genes = ExampleGenome_1()
	};

	ai_number expected = (ai_number)4 / (ai_number)7;

	population population = {
		.MatchingGeneImportance = 1.0,
		.ExcessGeneImportance = 1.0,
		.DisjointGeneImportance = 1.0
	};

	ai_number actual = GetSimilarity(&population, &left, &right);

	IsApproximate(expected, actual, "%lf");

	return true;
}

TEST(BreedOrganisms)
{
	organism left = {
		.Genes = ExampleGenome_0(),
		.Fitness = 0
	};

	organism right = {
		.Genes = ExampleGenome_1(),
		.Fitness = 69
	};

	ARRAY(gene) expectedGenes = ExampleGenome_2();

	Random.Seed = 42;

	Organism offspring = BreedOrganisms(&left, &right);

	ARRAY(gene) actualGenes = offspring->Genes;

	IsEqual(expectedGenes->Count, actualGenes->Count, "%lli");

	for (size_t i = 0; i < expectedGenes->Count; i++)
	{
		Gene expectedGene = &expectedGenes->Values[i];

		Gene actualGene = &actualGenes->Values[i];

		// NON RANDOM TESTS
		IsEqual(expectedGene->Id, actualGene->Id, "%lli");
		IsEqual(expectedGene->StartNodeIndex, actualGene->StartNodeIndex, "%lli");
		IsEqual(expectedGene->EndNodeIndex, actualGene->EndNodeIndex, "%lli");

		// RANDOM TESTS
		IsEqual(expectedGene->Enabled, actualGene->Enabled, "%x");
		IsApproximate(expectedGene->Weight, actualGene->Weight, "%f");
	}

	return true;
}

DEFINE_ARRAY(ai_number_array);

ARRAY(ai_number_array) GetXORTestData()
{
	ARRAY(ai_number_array) result = (ARRAY(ai_number_array))Arrays.Create(sizeof(ai_number_array), 4, Memory.GenericMemoryBlock);

	result->Values[0] = (ARRAY(ai_number))Arrays.Create(sizeof(ai_number), 3, Memory.GenericMemoryBlock);
	result->Values[1] = (ARRAY(ai_number))Arrays.Create(sizeof(ai_number), 3, Memory.GenericMemoryBlock);
	result->Values[2] = (ARRAY(ai_number))Arrays.Create(sizeof(ai_number), 3, Memory.GenericMemoryBlock);
	result->Values[3] = (ARRAY(ai_number))Arrays.Create(sizeof(ai_number), 3, Memory.GenericMemoryBlock);

	result->Values[0]->Values[0] = false;
	result->Values[0]->Values[1] = false;
	result->Values[0]->Values[2] = false;

	result->Values[1]->Values[0] = true;
	result->Values[1]->Values[1] = true;
	result->Values[1]->Values[2] = false;

	result->Values[2]->Values[0] = true;
	result->Values[2]->Values[1] = false;
	result->Values[2]->Values[2] = true;

	result->Values[3]->Values[0] = false;
	result->Values[3]->Values[1] = true;
	result->Values[3]->Values[2] = true;

	return result;
}

TEST(XOR_Works)
{
	Random.Seed = 42;

	// XOR (opposite) problem
	// ^ 
	// false ^ false = false
	// true ^ true = false
	// true ^ false = true
	// false ^ true = true

	size_t inputs = 2;
	size_t outputs = 1;

	Population ai = Neat.Create(15, inputs, outputs);

	size_t expectedIterations = 3600;

	ARRAY(ai_number_array) inputDataArrays = GetXORTestData();

	for (size_t i = 0; i < expectedIterations; i++)
	{
		for (size_t inputDataIndex = 0; inputDataIndex < inputDataArrays->Count; inputDataIndex++)
		{
			ARRAY(ai_number) inputData = inputDataArrays->Values[inputDataIndex];

			Neat.Propogate(ai, inputData);

			Neat.CalculateFitness(ai, inputData);

			Neat.CrossMutateAndSpeciate(ai);
		}
	}

	Neat.Dispose(ai);

	return true;
}

TEST_SUITE(
	RunUnitTests,
	APPEND_TEST(GetExcessGeneCount)
	APPEND_TEST(GetBothDisjointGeneCount)
	APPEND_TEST(GetAverageDifferenceBetweenWeights)
	APPEND_TEST(GetSimilarity)
	APPEND_TEST(BreedOrganisms)
	APPEND_TEST(XOR_Works)
)