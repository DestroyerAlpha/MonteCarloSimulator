#include <montecarlo/montecarlo.h>
#include <iostream>
#include <iomanip>
#include <map>

/**
 * @brief Dice rolling simulation
 * 
 * This example demonstrates using Monte Carlo simulation to analyze
 * probability distributions for dice rolling scenarios.
 */

int main() {
    using namespace montecarlo;
    
    std::cout << "Dice Rolling Simulation\n";
    std::cout << "=======================\n\n";
    
    const size_t num_rolls = 10000;
    
    // Example 1: Roll two dice and sum them
    {
        std::cout << "Example 1: Sum of two 6-sided dice\n";
        std::cout << "-----------------------------------\n";
        
        MonteCarloSimulator<int> simulator(num_rolls);
        
        auto dice_roll = []() -> int {
            auto& rng = getThreadLocalGenerator();
            int die1 = rng.uniform(1, 6);
            int die2 = rng.uniform(1, 6);
            return die1 + die2;
        };
        
        auto results = simulator.runRaw(dice_roll);
        
        // Count frequencies
        std::map<int, int> frequency;
        for (int result : results) {
            frequency[result]++;
        }
        
        std::cout << "  Sum  | Frequency | Probability\n";
        std::cout << "  -----|-----------|------------\n";
        for (const auto& [sum, count] : frequency) {
            double prob = static_cast<double>(count) / num_rolls;
            std::cout << "  " << std::setw(4) << sum << " | "
                      << std::setw(9) << count << " | "
                      << std::fixed << std::setprecision(4) << prob << "\n";
        }
        
        auto stats = simulator.run(dice_roll);
        std::cout << "\n  Mean sum: " << std::fixed << std::setprecision(2) 
                  << stats.mean << " (expected: 7.00)\n";
        std::cout << "  Std Dev:  " << stats.std_dev << "\n\n";
    }
    
    // Example 2: Yahtzee - probability of getting five of a kind
    {
        std::cout << "Example 2: Yahtzee - Five of a Kind\n";
        std::cout << "------------------------------------\n";
        
        MonteCarloSimulator<double> simulator(num_rolls);
        
        auto yahtzee_roll = []() -> double {
            auto& rng = getThreadLocalGenerator();
            
            // Roll 5 dice
            int dice[5];
            for (int i = 0; i < 5; ++i) {
                dice[i] = rng.uniform(1, 6);
            }
            
            // Check if all dice are the same
            bool five_of_kind = true;
            for (int i = 1; i < 5; ++i) {
                if (dice[i] != dice[0]) {
                    five_of_kind = false;
                    break;
                }
            }
            
            return five_of_kind ? 1.0 : 0.0;
        };
        
        auto result = simulator.run(yahtzee_roll);
        double estimated_prob = result.mean;
        double theoretical_prob = 6.0 / std::pow(6.0, 5.0);  // 6/7776
        
        std::cout << "  Estimated probability:   " << std::fixed << std::setprecision(6)
                  << estimated_prob << " (" << (estimated_prob * 100) << "%)\n";
        std::cout << "  Theoretical probability: " << theoretical_prob 
                  << " (" << (theoretical_prob * 100) << "%)\n";
        std::cout << "  Difference:              " 
                  << std::abs(estimated_prob - theoretical_prob) << "\n\n";
    }
    
    // Example 3: Craps - Pass line win probability
    {
        std::cout << "Example 3: Craps - Pass Line Win Probability\n";
        std::cout << "---------------------------------------------\n";
        
        MonteCarloSimulator<double> simulator(num_rolls);
        
        auto craps_game = []() -> double {
            auto& rng = getThreadLocalGenerator();
            
            // Come-out roll
            int roll = rng.uniform(1, 6) + rng.uniform(1, 6);
            
            // Natural win
            if (roll == 7 || roll == 11) return 1.0;
            
            // Craps (lose)
            if (roll == 2 || roll == 3 || roll == 12) return 0.0;
            
            // Point is established
            int point = roll;
            
            // Keep rolling until point or 7
            while (true) {
                roll = rng.uniform(1, 6) + rng.uniform(1, 6);
                if (roll == point) return 1.0;  // Win
                if (roll == 7) return 0.0;      // Lose
            }
        };
        
        auto result = simulator.run(craps_game);
        double estimated_prob = result.mean;
        double theoretical_prob = 244.0 / 495.0;  // Approximately 0.4929
        
        std::cout << "  Estimated probability:   " << std::fixed << std::setprecision(6)
                  << estimated_prob << " (" << (estimated_prob * 100) << "%)\n";
        std::cout << "  Theoretical probability: " << theoretical_prob 
                  << " (" << (theoretical_prob * 100) << "%)\n";
        std::cout << "  Difference:              " 
                  << std::abs(estimated_prob - theoretical_prob) << "\n";
    }
    
    return 0;
}
