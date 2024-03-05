#pragma once

#include "engine/ai/neat.h"

#include "core/memory.h"
#include "core/file.h"
#include "core/random.h"

#include "core/cunit.h"

#include <math.h>

#define DOUBLE_E 2.71828

#define RANDOM_NUMBER_GEN() Random.NextFloat()

private Population CreatePopulation(ulong populationSize, ulong inputNodeCount, ulong outputNodeCount);
private void DisposePopulation(Population);
private void SigmoidalTransferFunction(ai_number* number);
private void RunUnitTests();
private void Propogate(Population, array(ai_number) inputData);
private void CalculateFitness(Population);
private void CrossMutateAndSpeciate(Population);
private void SpeciatePopulation(Population);
private void DisposeSpecies(Species species);
private void AssignReferenceOrganism(Species species);

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
	.DefaultSimilarityThreshold = 3.0f,
	.DefaultTransferFunction = SigmoidalTransferFunction,
	.Create = CreatePopulation,
	.Dispose = DisposePopulation,
	.RunUnitTests = RunUnitTests,
	.Propogate = Propogate,
	.CalculateFitness = CalculateFitness,
	.CrossMutateAndSpeciate = CrossMutateAndSpeciate,
	.Speciate = SpeciatePopulation
};

DEFINE_TYPE_ID(Gene);
DEFINE_TYPE_ID(Species);
DEFINE_TYPE_ID(Organism);
DEFINE_TYPE_ID(Population);

private bool TryFindGene(Population population, gene gene, ulong* out_geneIndex)
{
	for (ulong i = 0; i < population->Genes->Count; i++)
	{
		Gene globalGene = &population->Genes->Values[i];
		if (gene.StartNodeIndex is globalGene->StartNodeIndex && gene.EndNodeIndex is globalGene->EndNodeIndex)
		{
			*out_geneIndex = globalGene->Id;
			return true;
		}
	}

	return false;
}

private ulong AddGlobalGeneToPopulation(Population population, gene gene)
{
	ulong newId = population->Genes->Count;

	gene.Id = newId;

	Arrays(gene).Append(population->Genes, gene);

	return newId;
}

private gene CreateGene(Population population, ulong startNodeIndex, ulong endNodeIndex)
{
	// a node shouldn't point to itself
	if (startNodeIndex is endNodeIndex)
	{
		throw(InvalidArgumentException);
	}

	gene result = {
		.Id = 0,
		.Enabled = true,
		.StartNodeIndex = startNodeIndex,
		.EndNodeIndex = endNodeIndex,
		.Weight = RANDOM_NUMBER_GEN()
	};

	ulong existingGeneId;
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

private ulong GetNewNodeId(Organism organism)
{
	ulong newNodeId = organism->NodeCount;

	organism->NodeCount = safe_add(newNodeId, 1);

	return newNodeId;
}

private void MutateAddNode(Organism organism)
{
	// create the node
	// get the highest id of nodes in the organism
	ulong endNode = GetNewNodeId(organism);

	// choose a random node to point to it
	ulong startNode = Random.Betweenulong(0, endNode - 1);

	gene newGene = CreateGene(organism->Parent->Parent, startNode, endNode);

	Arrays(gene).Append(organism->Genes, newGene);
}

private void MutateAddConnection(Organism organism)
{
	ulong startIndex = 0;
	ulong endIndex = 0;

	// dont point to the same node
	do
	{
		startIndex = Random.Betweenulong(0, safe_subtract(organism->NodeCount, 1));
		endIndex = Random.Betweenulong(0, safe_subtract(organism->NodeCount, 1));
	} while (startIndex is endIndex);

	gene gene = CreateGene(organism->Parent->Parent, startIndex, endIndex);

	Arrays(gene).Append(organism->Genes, gene);
}

private void MutateWeights(Organism organism)
{
	Population population = organism->Parent->Parent;

	ai_number uniformMutationValue = RANDOM_NUMBER_GEN();

	for (ulong geneIndex = 0; geneIndex < organism->Genes->Count; geneIndex++)
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

private void CreateDefaultGenome(Population population, array(gene) genes, ulong inputNodeCount, ulong outputNodeCount)
{
	for (ulong input = 0; input < inputNodeCount; input++)
	{
		for (ulong output = inputNodeCount; output < outputNodeCount + inputNodeCount; output++)
		{
			gene newGene = CreateGene(population, input, output);

			Arrays(gene).Append(genes, newGene);
		}
	}
}

private Organism CreateOrganism(ulong inputNodeCount, ulong outputNodeCount)
{
	Organism result = Memory.Alloc(sizeof(organism), OrganismTypeId);

	result->Genes = Arrays(gene).Create(0);
	result->Id = 0;
	result->InputNodeCount = inputNodeCount;
	result->WeightMatrix = BigMatrices.Create((inputNodeCount + outputNodeCount), (inputNodeCount + outputNodeCount));
	result->Outputs = Arrays(ai_number).Create(outputNodeCount);
	result->NodeCount = (inputNodeCount + outputNodeCount);
	result->OutputNodeCount = outputNodeCount;
	result->Parent = null;

	result->Outputs->Count = 0;

	return result;
}

private Organism CloneOrganism(Organism organism)
{
	Organism result = CreateOrganism(organism->InputNodeCount, organism->OutputNodeCount);

	Arrays(gene).AppendArray(result->Genes, organism->Genes);
	Arrays(ai_number).AppendArray(result->Outputs, organism->Outputs);
	Arrays(float).AppendArray(result->WeightMatrix->Values, organism->WeightMatrix->Values);

	result->Fitness = organism->Fitness;
	result->Generation = organism->Generation;
	result->Id = organism->Id;
	result->Parent = organism->Parent;

	return result;
}

private Species CreateSpecies(Population population, ulong organismCount)
{
	Species result = Memory.Alloc(sizeof(species), SpeciesTypeId);

	result->Id = 0;
	result->ReferenceOrganism = null;
	result->Generation = population->Generation;
	result->AverageFitness = 0.0;
	result->MaximumFitness = 0.0;
	result->InputNodeCount = population->InputNodeCount;
	result->OutputNodeCount = population->OutputNodeCount;
	result->Organisms = Arrays(Organism).Create(organismCount);
	result->Parent = population;

	for (ulong i = 0; i < result->Organisms->Count; i++)
	{
		Organism newOrganism = CreateOrganism(result->InputNodeCount, result->OutputNodeCount);

		newOrganism->Generation = result->Generation;

		newOrganism->Id = i;

		result->Organisms->Values[i] = newOrganism;
	}

	return result;
}

private Population CreatePopulation(ulong populationSize, ulong inputNodeCount, ulong outputNodeCount)
{
	REGISTER_TYPE(Gene);
	REGISTER_TYPE(Species);
	REGISTER_TYPE(Organism);

	Population result = Memory.Alloc(sizeof(population), SpeciesTypeId);

	population population = {
		.NextId = 0,
		.SummedAverageFitness = 0,
		.Genes = Arrays(gene).Create(Neat.DefaultGenePoolSize),
		.Species = Arrays(Species).Create(1),
		.InputNodeCount = inputNodeCount,
		.OutputNodeCount = outputNodeCount,
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
		.MatingWithCrossoverChance = Neat.DefaultMatingWithCrossoverRatio,
		.Generation = 1,
		.Count = populationSize,
		.TransferFunction = Neat.DefaultTransferFunction,
		.FitnessFunction = null
	};

	population.Genes->Count = 0;

	*result = population;

	for (ulong i = 0; i < population.Species->Count; i++)
	{
		Species species = CreateSpecies(result, populationSize);

		population.Species->Values[i] = species;

		species->Id = i;
		species->Parent = result;

		for (ulong organism = 0; organism < species->Organisms->Count; organism++)
		{
			species->Organisms->Values[organism]->Parent = population.Species->Values[i];

			CreateDefaultGenome(result, species->Organisms->Values[organism]->Genes, inputNodeCount, outputNodeCount);
		}
	}

	return result;
}


private void SerializeGene(File stream, Gene gene)
{
	fprintf_s(stream, "        [Id: %lli %lli->%lli %s %1.4f]\n", gene->Id, gene->StartNodeIndex, gene->EndNodeIndex, gene->Enabled ? "Enabled" : "Disabled", gene->Weight);
}

private void SerializeOrganism(File stream, Organism* organismPointer)
{
	Organism organism = *organismPointer;
	if (organism)
	{
		fprintf_s(stream, "      Id: %lli\n      Generation: %lli\n      Fitness: %f\n      Genes(%lli):\n", organism->Id, organism->Generation, organism->Fitness, organism->Genes->Count);

		Arrays(gene).ForeachWithContext(organism->Genes, stream, SerializeGene);
	}
}

private void SerializeSpecies(File stream, Species* speciesPointer)
{
	Species species = *speciesPointer;

	fprintf_s(stream, "    Id: %lli\n    Start Generation: %lli\n    Generation: %lli\n    Average Fitness: %f\n    Maximum Fitness: %f\n    Last Generation Where Fitness Improved: %lli\n    Reference Organism:\n", species->Id, species->StartGeneration, species->Generation, species->AverageFitness, species->MaximumFitness, species->LastGenerationWhereFitnessImproved);

	SerializeOrganism(stream, &species->ReferenceOrganism);

	fprintf_s(stream, "    Organisms(%lli):\n", species->Organisms->Count);

	Arrays(Organism).ForeachWithContext(species->Organisms, stream, SerializeOrganism);
}

private void SerializePopulation(File stream, Population population)
{
	fprintf_s(stream, "Population(%lli):\n  Generation: %lli\n  Summed Average Fitness: %f\n  Genes(%lli):\n", population->Count, population->Generation, population->SummedAverageFitness, population->Genes->Count);

	Arrays(gene).ForeachWithContext(population->Genes, stream, SerializeGene);

	fprintf_s(stream, "  Species(%lli):\n", population->Species->Count);
	Arrays(Species).ForeachWithContext(population->Species, stream, SerializeSpecies);
}

private ulong GetHighestGeneId(const array(gene) genes)
{
	ulong largestId = 0;
	for (ulong geneIndex = 0; geneIndex < genes->Count; geneIndex++)
	{
		if (genes->Values[geneIndex].Id > largestId)
		{
			largestId = genes->Values[geneIndex].Id;
		}
	}

	return largestId;
}

private ulong GetNodeCount(array(gene) genes)
{
	ulong nodeCount = 0;

	for (ulong i = 0; i < genes->Count; i++)
	{
		ulong highestIndex = max(genes->Values[i].EndNodeIndex, genes->Values[i].StartNodeIndex);
		if (highestIndex > nodeCount) {
			nodeCount = highestIndex;
		}
	}

	return nodeCount + 1;
}

private void LoadWeightsFromGenomeIntoMatrix(array(gene) genes, BigMatrix matrix)
{
	ulong matrixWidth = GetNodeCount(genes);

	// make sure the matrix is big enough
	BigMatrices.Resize(matrix, matrixWidth, matrixWidth);

	// write zeros
	BigMatrices.Clear(matrix);

	for (ulong geneIndex = 0; geneIndex < genes->Count; geneIndex++)
	{
		Gene gene = Arrays(gene).At(genes, geneIndex);

		if (gene->Enabled)
		{
			*BigMatrices.At(matrix, gene->StartNodeIndex, gene->EndNodeIndex) = gene->Weight;
		}
	}
}

// returns true when the gene array contain the given gene
private bool ContainsGeneId(ulong geneId, const array(gene) genes)
{
	for (ulong geneIndex = 0; geneIndex < genes->Count; geneIndex++)
	{
		if (genes->Values[geneIndex].Id is geneId)
		{
			return true;
		}
	}

	return false;
}

private ulong GetDisjointGeneCount(const array(gene) leftGenes, const array(gene) rightGenes)
{
	ulong smallestCount = min(leftGenes->Count, rightGenes->Count);
	ulong disjointCount = 0;
	for (ulong leftIndex = 0; leftIndex < smallestCount; leftIndex++)
	{
		ulong leftId = leftGenes->Values[leftIndex].Id;

		if (ContainsGeneId(leftId, rightGenes) is false)
		{
			safe_increment(disjointCount);
		}
	}

	return disjointCount;
}

private ai_number GetAverageDifferenceBetweenWeights(const array(gene) leftGenes, const array(gene) rightGenes)
{
	ulong count = 0;
	ai_number value = 0;

	for (ulong leftIndex = 0; leftIndex < leftGenes->Count; leftIndex++)
	{
		const Gene leftGene = &leftGenes->Values[leftIndex];

		for (ulong rightIndex = 0; rightIndex < rightGenes->Count; rightIndex++)
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

private ulong GetExcessGeneCount(const array(gene) left, const array(gene) right)
{
	ulong highestLeftId = GetHighestGeneId(left);
	ulong highestRightId = GetHighestGeneId(right);

	return safe_subtract(highestRightId, highestLeftId);
}

private ulong GetBothDisjointGeneCount(const array(gene) left, const array(gene) right)
{
	ulong leftDisjointCount = GetDisjointGeneCount(left, right);
	ulong rightDisjointCount = GetDisjointGeneCount(right, left);

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

private void RemoveDisimilarOrganismsFromSpecies(Population population, Species species, array(Organism) destinationArray)
{
	Organism referenceOrganism = species->ReferenceOrganism;
	ulong organismIndex = species->Organisms->Count;
	while (organismIndex-- > 0)
	{
		Organism organism = species->Organisms->Values[organismIndex];
		if (SimilarOrganisms(population, referenceOrganism, organism) is false)
		{
			// add to the array to get sorted
			Arrays(Organism).Append(destinationArray, organism);

			// remove it from this array
			Arrays(Organism).RemoveIndex(species->Organisms, organismIndex);
		}
	}
}

private void SortOrganismsIntoSpecies(Population population, array(Organism) organisms)
{
	ulong organismIndex = organisms->Count;
	while (organismIndex-- > 0)
	{
		Organism organism = organisms->Values[organismIndex];

		bool sorted = false;
		for (ulong speciesIndex = 0; speciesIndex < population->Species->Count; speciesIndex++)
		{
			Species species = population->Species->Values[speciesIndex];

			if (species->Organisms->Count is 0)
			{
				// species should not have zero members here
				throw(InvalidLogicException);
			}

			if (SimilarOrganisms(population, organism, species->ReferenceOrganism))
			{
				Arrays(Organism).Append(species->Organisms, organism);
				sorted = true;
				break;
			}
		}

		if (sorted)
		{
			continue;
		}

		// create a new species
		Species newSpecies = CreateSpecies(population, 0);

		Arrays(Organism).Append(newSpecies->Organisms, organism);

		organism->Parent = newSpecies;

		AssignReferenceOrganism(newSpecies);

		Arrays(Species).Append(population->Species, newSpecies);
	}
}

// Goes through all the organisms of the population
// and ensures that they are properly sorted into
// species according to their similarities
private void SpeciatePopulation(Population population)
{
	// create a spot to store the organisms needing sorting
	array(Organism) organismsNeedingSorting = Arrays(Organism).Create(0);

	ulong speciesIndex = population->Species->Count;
	while (speciesIndex-- > 0)
	{
		Species species = population->Species->Values[speciesIndex];

		// Remove the species if it doesnt have any organisms
		if (species->Organisms->Count is 0)
		{
			DisposeSpecies(species);
			Arrays(Species).RemoveIndex(population->Species, speciesIndex);

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

private void SigmoidalTransferFunction(ai_number* number)
{
	*number = (ai_number)((ai_number)1.0 / ((ai_number)1.0 + pow((ai_number)DOUBLE_E, (ai_number)-4.9 * *number)));
}

private Gene GetGeneWithId(array(gene) genes, ulong id)
{
	for (ulong i = 0; i < genes->Count; i++)
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

	Organism newOrganism = CreateOrganism(left->InputNodeCount, left->OutputNodeCount);

	for (ulong geneIndex = 0; geneIndex < moreFitOrganism->Genes->Count; geneIndex++)
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

			Arrays(gene).Append(newOrganism->Genes, *lessFitGene);
		}
		else
		{
			Arrays(gene).Append(newOrganism->Genes, *moreFitGene);
		}
	}

	return newOrganism;
}

private void DisposeOrganism(Organism organism)
{
	Arrays(gene).Dispose(organism->Genes);
	Arrays(ai_number).Dispose(organism->Outputs);
	BigMatrices.Dispose(organism->WeightMatrix);

	Memory.Free(organism, OrganismTypeId);
}

private void DisposeSpecies(Species species)
{
	for (ulong organismIndex = 0; organismIndex < species->Organisms->Count; organismIndex++)
	{
		DisposeOrganism(species->Organisms->Values[organismIndex]);
	}

	DisposeOrganism(species->ReferenceOrganism);

	Arrays(Organism).Dispose(species->Organisms);

	Memory.Free(species, SpeciesTypeId);
}

private void DisposePopulation(Population population)
{
	if (population is null)
	{
		return;
	}

	for (ulong speciesIndex = 0; speciesIndex < population->Species->Count; speciesIndex++)
	{
		Species species = population->Species->Values[speciesIndex];

		DisposeSpecies(species);
	}

	Arrays(Species).Dispose(population->Species);
	Arrays(gene).Dispose(population->Genes);

	Memory.Free(population, SpeciesTypeId);
}

private void PropogateOrganism(Population population, Organism organism, array(ai_number) inputData)
{
	LoadWeightsFromGenomeIntoMatrix(organism->Genes, organism->WeightMatrix);

	Arrays(ai_number).Clear(organism->Outputs);

	BigMatrices.MultiplyVector(organism->WeightMatrix, (array(float))inputData, (array(float))organism->Outputs);

	Arrays(ai_number).Foreach(organism->Outputs, population->TransferFunction);
}

private void Propogate(Population population, array(ai_number) inputData)
{
	for (ulong i = 0; i < population->Species->Count; i++)
	{
		Species species = population->Species->Values[i];

		for (ulong organismIndex = 0; organismIndex < species->Organisms->Count; organismIndex++)
		{
			PropogateOrganism(population, species->Organisms->Values[organismIndex], inputData);
		}
	}
}

private void CalculateFitness(Population population)
{
	population->SummedAverageFitness = 0;

	for (ulong i = 0; i < population->Species->Count; i++)
	{
		Species species = population->Species->Values[i];

		species->AverageFitness = 0;

		for (ulong organismIndex = 0; organismIndex < species->Organisms->Count; organismIndex++)
		{
			Organism organism = species->Organisms->Values[organismIndex];

			organism->Fitness = population->FitnessFunction(organism);

			organism->Fitness = GetAdjustedFitness(organism);

			if (species->MaximumFitness < organism->Fitness)
			{
				species->MaximumFitness = organism->Fitness;
				species->LastGenerationWhereFitnessImproved = safe_subtract(population->Generation, 1);
			}

			species->AverageFitness += organism->Fitness;
		}


		species->AverageFitness = species->AverageFitness / species->Organisms->Count;

		population->SummedAverageFitness += species->AverageFitness;
	}
}

private bool OrganismFitnessComparator(Organism* left, Organism* right)
{
	return (*left)->Fitness < (*right)->Fitness;
}

private void AssignReferenceOrganism(Species species)
{
	// set the reference organism to a random organism
	species->ReferenceOrganism = CloneOrganism(species->Organisms->Values[Random.Nextulong() % species->Organisms->Count]);
}

// removes the poorest performing organisms within a species
// returns the number of organisms within this species that
// were removed
private ulong RemovePoorFitnessOrganisms(Population population, Species species)
{
	// sort by fitness
	Arrays(Organism).InsertionSort(species->Organisms, OrganismFitnessComparator);

	// determine how many to remove
	const ulong count = (ulong)((ai_number)species->Organisms->Count * population->OrganismCullingRate);
	const ulong stopIndex = safe_subtract(species->Organisms->Count, count);

	ulong index = species->Organisms->Count;
	while (index-- > stopIndex)
	{
		DisposeOrganism(species->Organisms->Values[index]);
		Arrays(Organism).RemoveIndex(species->Organisms, index);
	}

	AssignReferenceOrganism(species);

	return count;
}

private void CalculatePopulationFitnesses(Population population)
{
	ai_number summedAverage = 0;
	for (ulong speciesIndex = 0; speciesIndex < population->Species->Count; speciesIndex++)
	{
		Species species = population->Species->Values[speciesIndex];
		ai_number speciesAverage = 0;

		for (ulong organismIndex = 0; organismIndex < species->Organisms->Count; organismIndex++)
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
	ulong speciesIndex = population->Species->Count;
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

			Arrays(Species).RemoveIndex(population->Species, speciesIndex);
		}
	}
}

private void MutateOrganism(Population population, Organism organism)
{
	if (Random.Chance(population->AddConnectionMutationChance))
	{
		MutateAddConnection(organism);
	}
	if (Random.Chance(population->AddNodeMutationChance))
	{
		MutateAddNode(organism);
	}
	if (Random.Chance(population->WeightMutationChance))
	{
		MutateWeights(organism);
	}
}

private void ReplenishSpecies(Population population, Species species, ulong count)
{
	if (count is 0)
	{
		return;
	}

	ulong organismsCreated = 1;

	do
	{
		Organism child = null;
		const ulong index = Random.Betweenulong(0, species->Organisms->Count);
		Organism left = species->Organisms->Values[index];

		if (Random.Chance(population->MatingWithCrossoverChance))
		{
			Organism right = species->Organisms->Values[safe_add(index, 1) % species->Organisms->Count];

			child = BreedOrganisms(left, right);
		}
		else
		{
			child = CloneOrganism(left);
		}

		child->Generation = population->Generation;
		child->Parent = species;
		child->Id = safe_increment(population->NextId);

		MutateOrganism(population, child);

		Arrays(Organism).Append(species->Organisms, child);
	} while (safe_increment(organismsCreated) < count);
}

private void CrossMutateAndSpeciate(Population population)
{
	fprintf(stdout, "------------------------------------ Before CrossMutateAndSpeciate:\n");
	SerializePopulation(stdout, population);

	// since we're crossing and mutating we should increase our generation
	safe_increment(population->Generation);

	RemoveStagnatingSpecies(population);

	for (ulong speciesIndex = 0; speciesIndex < population->Species->Count; speciesIndex++)
	{
		Species species = population->Species->Values[speciesIndex];

		// since we're allowed to reproduce increase the generation of the species
		species->Generation = population->Generation;

		ai_number allotedPercentage = species->AverageFitness / population->SummedAverageFitness;

		ulong allotedCount = (ulong)(allotedPercentage * (ai_number)population->Count);

		// kill the bottom performing organisms within the species
		RemovePoorFitnessOrganisms(population, species);

		// replenish UP TO the allotted count even if we removed more then the alloted
		ReplenishSpecies(population, species, safe_subtract(species->Organisms->Count, allotedCount));
	}

	SpeciatePopulation(population);
}

// Tests
private array(gene) ExampleGenome_0()
{
	array(gene) genome = Arrays(gene).Create(5);

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

private array(gene) ExampleGenome_1()
{
	array(gene) genome = Arrays(gene).Create(7);

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

private array(gene) ExampleGenome_2()
{
	array(gene) genome = Arrays(gene).Create(7);

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

TEST(ArrayWorks)
{
	array(int) ints = Arrays(int).Create(4);

	ints->Values[0] = 0;
	ints->Values[1] = 1;
	ints->Values[2] = 2;
	ints->Values[3] = 3;

	Arrays(int).RemoveIndex(ints, 1);

	IsEqual(3ull, ints->Count);
	IsEqual(0, ints->Values[0]);
	IsEqual(2, ints->Values[1]);
	IsEqual(3, ints->Values[2]);

	return true;
}

TEST(GetExcessGeneCount)
{
	array(gene) left = ExampleGenome_0();

	array(gene) right = ExampleGenome_1();

	ulong expected = 2;

	IsEqual(expected, GetExcessGeneCount(left, right));

	Arrays(gene).Dispose(left);
	Arrays(gene).Dispose(right);

	return true;
}

TEST(GetBothDisjointGeneCount)
{
	array(gene) left = ExampleGenome_0();

	array(gene) right = ExampleGenome_1();

	ulong expected = 2;

	IsEqual(expected, GetBothDisjointGeneCount(left, right));

	Arrays(gene).Dispose(left);
	Arrays(gene).Dispose(right);

	return true;
}

TEST(GetAverageDifferenceBetweenWeights)
{
	array(gene) left = ExampleGenome_0();

	array(gene) right = ExampleGenome_1();

	ai_number expected = 0;

	IsApproximate(expected, GetAverageDifferenceBetweenWeights(left, right));

	Arrays(gene).Dispose(left);
	Arrays(gene).Dispose(right);

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

	IsApproximate(expected, actual);

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

	array(gene) expectedGenes = ExampleGenome_2();

	Random.Seed = 42;

	Organism offspring = BreedOrganisms(&left, &right);

	array(gene) actualGenes = offspring->Genes;

	IsEqual(expectedGenes->Count, actualGenes->Count);

	for (ulong i = 0; i < expectedGenes->Count; i++)
	{
		Gene expectedGene = &expectedGenes->Values[i];

		Gene actualGene = &actualGenes->Values[i];

		// NON RANDOM TESTS
		IsEqual(expectedGene->Id, actualGene->Id);
		IsEqual(expectedGene->StartNodeIndex, actualGene->StartNodeIndex);
		IsEqual(expectedGene->EndNodeIndex, actualGene->EndNodeIndex);

		// RANDOM TESTS
		IsEqual(expectedGene->Enabled, actualGene->Enabled);
		IsApproximate(expectedGene->Weight, actualGene->Weight);
	}

	Arrays(gene).Dispose(expectedGenes);
	Arrays(gene).Dispose(actualGenes);

	return true;
}

DEFINE_ARRAY(ai_number_array);

array(ai_number_array) GetXORTestData()
{
	array(ai_number_array) result = (array(ai_number_array))Arrays.Create(sizeof(ai_number_array), 5, Memory.GenericMemoryBlock);

	result->Values[0] = (array(ai_number))Arrays.Create(sizeof(ai_number), 2, Memory.GenericMemoryBlock);
	result->Values[1] = (array(ai_number))Arrays.Create(sizeof(ai_number), 2, Memory.GenericMemoryBlock);
	result->Values[2] = (array(ai_number))Arrays.Create(sizeof(ai_number), 2, Memory.GenericMemoryBlock);
	result->Values[3] = (array(ai_number))Arrays.Create(sizeof(ai_number), 2, Memory.GenericMemoryBlock);
	result->Values[4] = (array(ai_number))Arrays.Create(sizeof(ai_number), 4, Memory.GenericMemoryBlock);

	result->Values[0]->Values[0] = false;
	result->Values[0]->Values[1] = false;

	result->Values[1]->Values[0] = true;
	result->Values[1]->Values[1] = true;

	result->Values[2]->Values[0] = true;
	result->Values[2]->Values[1] = false;

	result->Values[3]->Values[0] = false;
	result->Values[3]->Values[1] = true;

	// expected data
	result->Values[4]->Values[0] = false;
	result->Values[4]->Values[1] = false;
	result->Values[4]->Values[2] = true;
	result->Values[4]->Values[3] = true;

	return result;
}

static bool GLOBAL_expected = false;

ai_number XORFitnessFunction(const Organism organism)
{
	return (ai_number)(fabs((double)organism->Outputs->Values[0]) - (double)GLOBAL_expected);
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

	ulong inputs = 2;
	ulong outputs = 1;

	Population ai = Neat.Create(15, inputs, outputs);

	SerializePopulation(stdout, ai);

	ai->FitnessFunction = XORFitnessFunction;

	ulong expectedIterations = 3600;

	array(ai_number_array) inputDataArrays = GetXORTestData();

	for (ulong i = 0; i < expectedIterations; i++)
	{
		array(ai_number) expected = inputDataArrays->Values[inputDataArrays->Count - 1];

		for (ulong inputDataIndex = 0; inputDataIndex < inputDataArrays->Count - 1; inputDataIndex++)
		{
			array(ai_number) inputData = inputDataArrays->Values[inputDataIndex];

			GLOBAL_expected = expected->Values[inputDataIndex];

			Neat.Propogate(ai, inputData);

			Neat.CalculateFitness(ai);

			Neat.CrossMutateAndSpeciate(ai);
		}
	}

	Neat.Dispose(ai);
	Arrays(ai_number_array).Dispose(inputDataArrays);

	Memory.PrintAlloc(stdout);
	Memory.PrintFree(stdout);

	return true;
}

TEST_SUITE(
	RunUnitTests,
	APPEND_TEST(ArrayWorks)
	APPEND_TEST(XOR_Works)
	APPEND_TEST(GetExcessGeneCount)
	APPEND_TEST(GetBothDisjointGeneCount)
	APPEND_TEST(GetAverageDifferenceBetweenWeights)
	APPEND_TEST(GetSimilarity)
	APPEND_TEST(BreedOrganisms)
)