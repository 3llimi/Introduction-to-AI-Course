//Ahmed Baha Eddine Alimi - ISE-05 - a.alimi@innopolis.university*******
#include <bits/stdc++.h>


using namespace std;

// Declaration of Global Constants
const double initial_mutation_rate = 0.1;
const int stagnation_limit = 20;
int population_size = 200;

// A function to calculate a variance of a dataset
double calculateVariance(const vector<double>& data) {
    double mean = accumulate(data.begin(), data.end(), 0.0) / data.size();
    double variance = 0.0;
    for (double value : data) {
        variance += (value - mean) * (value - mean);
    }
    return variance / data.size();
}

// A function to calculate fitness of an individual based on conflicts
// The fitness is calculated by checking these types of conflicts: row conflicts, column conflicts, and subgrid conflicts.
double computeFitness(const vector<vector<int>>& individual) {
    int rowConflicts = 0, columnConflicts = 0, subgridConflicts = 0;

    // Calculating row conflicts
    for (int row = 0; row < 9; ++row) {
        set<int> uniqueNumbers(individual[row].begin(), individual[row].end());
        rowConflicts += (9 - uniqueNumbers.size());
    }

    // Calculating column conflicts
    for (int col = 0; col < 9; ++col) {
        vector<int> column;
        for (int row = 0; row < 9; ++row) {
            column.push_back(individual[row][col]);
        }
        set<int> uniqueNumbers(column.begin(), column.end());
        columnConflicts += (9 - uniqueNumbers.size());
    }

    // Calculating subgrid conflicts
    for (int subgridRow = 0; subgridRow < 9; subgridRow += 3) {
        for (int subgridCol = 0; subgridCol < 9; subgridCol += 3) {
            vector<int> subgrid;
            for (int row = subgridRow; row < subgridRow + 3; ++row) {
                for (int col = subgridCol; col < subgridCol + 3; ++col) {
                    subgrid.push_back(individual[row][col]);
                }
            }
            set<int> uniqueNumbers(subgrid.begin(), subgrid.end());
            subgridConflicts += (9 - uniqueNumbers.size());
        }
    }

    int totalConflicts = rowConflicts + columnConflicts + subgridConflicts;
    double rowWeight = (totalConflicts > 1) ? static_cast<double>(rowConflicts) / totalConflicts : 1.0;
    double columnWeight = (totalConflicts > 1) ? static_cast<double>(columnConflicts) / totalConflicts : 1.0;
    double subgridWeight = (totalConflicts > 1) ? static_cast<double>(subgridConflicts) / totalConflicts : 1.0;
    return rowConflicts * rowWeight + columnConflicts * columnWeight + subgridConflicts * subgridWeight;
}

// A function to generate the initial individual for the genetic algorithm
// This function takes an initial puzzle and fills in the missing numbers with randomly shuffled valid numbers to create an individual solution.
vector<vector<int>> generateIndividual(const vector<vector<int>>& puzzleGrid) {
    vector<vector<int>> individual = puzzleGrid;
    random_device randomDevice;
    mt19937 randomGenerator(randomDevice());
    for (int row = 0; row < 9; ++row) {
        set<int> presentNumbers(individual[row].begin(), individual[row].end());
        vector<int> missingNumbers;
        for (int num = 1; num <= 9; ++num) {
            if (presentNumbers.find(num) == presentNumbers.end()) {
                missingNumbers.push_back(num);
            }
        }
        shuffle(missingNumbers.begin(), missingNumbers.end(), randomGenerator);
        for (int col = 0; col < 9; ++col) {
            if (individual[row][col] == 0) {
                individual[row][col] = missingNumbers.back();
                missingNumbers.pop_back();
            }
        }
    }
    return individual;
}



// A function to perform crossover between two parent solutions
// Crossover combines parts of two parent Sudoku solutions to create a child solution.
vector<vector<int>> performCrossover(const vector<vector<int>>& parent1, const vector<vector<int>>& parent2) {
    vector<vector<int>> offspring = parent1;
    random_device randomDevice;
    mt19937 randomGenerator(randomDevice());
    for (int rowBlock = 0; rowBlock < 9; rowBlock += 3) {
        uniform_real_distribution<double> probability(0.0, 1.0);
        if (probability(randomGenerator) > 0.5) {
            for (int row = 0; row < 3; ++row) {
                offspring[rowBlock + row] = parent2[rowBlock + row];
            }
        }
    }
    return offspring;
}


// A function to apply mutation to an individual
// Mutation happens with a given mutation rate and focuses on resolving conflicts within rows by swapping repeated numbers.
void applyMutation(vector<vector<int>>& individual, const vector<vector<int>>& puzzleGrid, double mutationRate) {
    random_device randomDevice;
    mt19937 randomGenerator(randomDevice());
    for (int row = 0; row < 9; ++row) {
        uniform_real_distribution<double> probability(0.0, 1.0);
        if (probability(randomGenerator) < mutationRate) {
            vector<int> rowValues = individual[row];
            vector<int> conflictIndices;
            for (size_t index = 0; index < rowValues.size(); ++index) {
                if (count(rowValues.begin(), rowValues.end(), rowValues[index]) > 1 && puzzleGrid[row][index] == 0) {
                    conflictIndices.push_back(index);
                }
            }
            if (conflictIndices.size() >= 2) {
                uniform_int_distribution<int> indexDistribution(0, conflictIndices.size() - 1);
                int firstIndex = conflictIndices[indexDistribution(randomGenerator)];
                int secondIndex = conflictIndices[indexDistribution(randomGenerator)];
                swap(rowValues[firstIndex], rowValues[secondIndex]);
                individual[row] = rowValues;
            }
        }
    }
}

// A greedy local search to resolve row conflicts
// The search tries to replace duplicate values in the row with missing valid numbers.
void applyGreedyLocalSearch(vector<vector<int>>& individual, const vector<vector<int>>& puzzleGrid) {
    for (int row = 0; row < 9; ++row) {
        vector<int>& rowValues = individual[row];
        vector<int> conflictIndices;
        for (size_t index = 0; index < rowValues.size(); ++index) {
            if (count(rowValues.begin(), rowValues.end(), rowValues[index]) > 1 && puzzleGrid[row][index] == 0) {
                conflictIndices.push_back(index);
            }
        }
        if (!conflictIndices.empty()) {
            set<int> presentNumbers(rowValues.begin(), rowValues.end());
            vector<int> missingNumbers;
            for (int num = 1; num <= 9; ++num) {
                if (presentNumbers.find(num) == presentNumbers.end()) {
                    missingNumbers.push_back(num);
                }
            }
            for (size_t index : conflictIndices) {
                rowValues[index] = missingNumbers.front();
                missingNumbers.erase(missingNumbers.begin());
            }
        }
    }
}

// A simulated annealing to optimize fitness
// The function gradually lowers the temperature of annealing, reducing the likelihood of accepting worse solutions over time.
void applySimulatedAnnealing(vector<vector<int>>& individual, const vector<vector<int>>& puzzleGrid, double initialTemperature = 1.0, double coolingRate = 0.95) {
    double currentFitness = computeFitness(individual);
    random_device randomDevice;
    mt19937 randomGenerator(randomDevice());
    double temperature = initialTemperature;

    while (temperature > 0.001) {
        uniform_int_distribution<int> rowDistribution(0, 8);
        int row = rowDistribution(randomGenerator);
        vector<int>& rowValues = individual[row];
        vector<int> mutableIndices;
        for (size_t index = 0; index < rowValues.size(); ++index) {
            if (puzzleGrid[row][index] == 0) {
                mutableIndices.push_back(index);
            }
        }
        if (mutableIndices.size() >= 2) {
            uniform_int_distribution<int> indexDistribution(0, mutableIndices.size() - 1);
            int firstIndex = mutableIndices[indexDistribution(randomGenerator)];
            int secondIndex = mutableIndices[indexDistribution(randomGenerator)];
            swap(rowValues[firstIndex], rowValues[secondIndex]);

            double newFitness = computeFitness(individual);
            uniform_real_distribution<double> probability(0.0, 1.0);
            if (newFitness < currentFitness || probability(randomGenerator) < exp((currentFitness - newFitness) / temperature)) {
                currentFitness = newFitness;
            } else {
                swap(rowValues[firstIndex], rowValues[secondIndex]);
            }
        }
        temperature *= coolingRate;
    }
}

// Parent selection using tournament selection
vector<vector<int>> selectParent(const vector<pair<vector<vector<int>>, double>>& population) {
    vector<double> fitnessValues;
    for (const auto& individual : population) {
        fitnessValues.push_back(individual.second);
    }
    double fitnessVariance = calculateVariance(fitnessValues);
    int tournamentSize = (fitnessVariance > 5) ? 3 : 5;

    random_device randomDevice;
    mt19937 randomGenerator(randomDevice());
    vector<pair<vector<vector<int>>, double>> tournament(population.begin(), population.begin() + tournamentSize);
    sort(tournament.begin(), tournament.end(), [](const auto& a, const auto& b) { return a.second < b.second; });

    uniform_real_distribution<double> probability(0.0, 1.0);
    return (probability(randomGenerator) < 0.8) ? tournament[0].first : tournament[1].first;
}

// Evaluating fitness for the entire population
vector<pair<vector<vector<int>>, double>> evaluatePopulation(const vector<pair<vector<vector<int>>, double>>& population) {
    vector<pair<vector<vector<int>>, double>> evaluatedPopulation;
    for (const auto& individual : population) {
        evaluatedPopulation.emplace_back(individual.first, computeFitness(individual.first));
    }
    return evaluatedPopulation;
}

// The Genetic algorithm to solve the puzzle
vector<vector<int>> solveSudokuUsingGeneticAlgorithm(const vector<vector<int>>& initialPuzzle) {
    vector<pair<vector<vector<int>>, double>> population;

    // Initializing population
    for (int i = 0; i < population_size; ++i) {
        population.emplace_back(generateIndividual(initialPuzzle), 0.0);
    }
    population = evaluatePopulation(population);
    int bestFitness = 300;
        sort(population.begin(), population.end(), [](const auto& a, const auto& b) {
            return a.second < b.second;
        });

        double bestFitnessInGeneration = population[0].second;

        if (bestFitnessInGeneration == 0.0) {  // Solution found
            return population[0].first;
        }

        double mutationRate = initial_mutation_rate;
        int stagnationCounter = 0;

        // Evolving until convergence or stagnation
        while (true) {
            sort(population.begin(), population.end(), [](const auto& a, const auto& b) {
                return a.second < b.second;
            });

            double bestFitnessInPopulation = population[0].second;

            if (bestFitnessInPopulation == 0.0) {  // Solution found
                return population[0].first;
            }

            // Applying local optimization if the solution is close
            if (bestFitnessInPopulation < 10.0) {
                for (int i = 0; i < 5; ++i) {
                    applyGreedyLocalSearch(population[i].first, initialPuzzle);
                }
            }

            if (bestFitnessInPopulation < 20.0) {
                for (int i = 0; i < 5; ++i) {
                    applySimulatedAnnealing(population[i].first, initialPuzzle);
                }
            }

            // Adjusting mutation rate dynamically
            if (bestFitnessInPopulation < 4.0) {
                mutationRate = min(mutationRate * 1.2, 0.3);
            }

            if (bestFitnessInPopulation< bestFitness ){
                bestFitness=bestFitnessInPopulation;
                stagnationCounter = 0;
            } else{
                stagnationCounter++;
            }

            // Handling stagnation
            if (stagnationCounter >= stagnation_limit) {
                vector<vector<int>> bestSolution = population[0].first;
                population.clear();
                for (int i = 0; i < population_size; ++i) {
                    population.emplace_back(bestSolution, computeFitness(bestSolution));
                }
                stagnationCounter = 0;
            }

            //Creating a new generation
            vector<pair<vector<vector<int>>, double>> newPopulation(population.begin(), population.begin() + 5);
            while (newPopulation.size() < population_size) {
                auto parent1 = selectParent(population);
                auto parent2 = selectParent(population);
                auto child = performCrossover(parent1, parent2);
                applyMutation(child, initialPuzzle, mutationRate);
                newPopulation.emplace_back(child, 0.0);
            }

            population = evaluatePopulation(newPopulation);
        }
}

// A function to print the Sudoku solution in the right format
void displaySolution(const vector<vector<int>>& solution) {
    for (size_t row = 0; row < solution.size(); ++row) {
        for (size_t col = 0; col < solution[row].size(); ++col) {
            cout << solution[row][col];
            if (col < solution[row].size() - 1) {
                cout << " ";
            }
        }
        cout << endl;
    }
}

// A function to read the Sudoku puzzle from the input
vector<vector<int>> readSudokuGrid() {
    vector<vector<int>> puzzleGrid(9, vector<int>(9));
    string inputLine;
    int rowIndex = 0;

    while (rowIndex < 9) {
        getline(cin, inputLine);

        if (inputLine.empty()) {
            break;
        }

        istringstream lineStream(inputLine);
        for (int colIndex = 0; colIndex < 9; ++colIndex) {
            string cell;
            lineStream >> cell;
            puzzleGrid[rowIndex][colIndex] = (cell == "-") ? 0 : stoi(cell);
        }

        ++rowIndex;
    }
    return puzzleGrid;
}

int main() {
    vector<vector<int>> inputPuzzle = readSudokuGrid(); // Reading the input
    vector<vector<int>> solvedPuzzle = solveSudokuUsingGeneticAlgorithm(inputPuzzle); // Solving the puzzle

    if (!solvedPuzzle.empty()) { // Printing the solution (if a solution is found)
        displaySolution(solvedPuzzle);
    } else {
        cout << "No solution found." << endl;
    }

    return 0;
}