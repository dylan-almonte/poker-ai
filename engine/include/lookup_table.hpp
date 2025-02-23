#pragma once

#include <unordered_map>
#include <string>
#include <vector>

class LookupTable {
public:
    LookupTable();
    ~LookupTable() = default;

    // Constants for hand rankings
    static constexpr int MAX_STRAIGHT_FLUSH = 10;
    static constexpr int MAX_FOUR_OF_A_KIND = 166;
    static constexpr int MAX_FULL_HOUSE = 322;
    static constexpr int MAX_FLUSH = 1599;
    static constexpr int MAX_STRAIGHT = 1609;
    static constexpr int MAX_THREE_OF_A_KIND = 2467;
    static constexpr int MAX_TWO_PAIR = 3325;
    static constexpr int MAX_PAIR = 6185;
    static constexpr int MAX_HIGH_CARD = 7462;

    // Lookup methods
    int getFlushRank(int prime_product) const;
    int getUnsuitedRank(int prime_product) const;
    
    // Ranking helpers
    int getRankClass(int hand_rank) const;
    std::string classToString(int class_int) const;
    double getFiveCardRankPercentage(int hand_rank) const;

    // Rank class conversion
    static int maxToRankClass(int max_rank);
    static std::string rankClassToString(int rank_class);

private:
    void generateLookupTables();
    void generateFlushes();
    void generateMultiples();
    void fillLookupTable(int rank_init, const std::vector<int>& rankbits, 
                        std::unordered_map<int, int>& lookup_table);
    std::vector<int> getLexographicallyNextBitSequence(int bits);

    void initFlushLookup();
    void initUnsuitedLookup();
    void initRankClasses();

    std::unordered_map<int, int> flush_lookup_;
    std::unordered_map<int, int> unsuited_lookup_;
    std::unordered_map<int, int> rank_class_lookup_;
    std::unordered_map<int, std::string> class_strings_;

    static const std::unordered_map<int, int> MAX_TO_RANK_CLASS_MAP;
    static const std::unordered_map<int, std::string> RANK_CLASS_TO_STRING_MAP;
}; 