#ifndef GASLIST_HPP
#define GASLIST_HPP

#include "parameters.hpp"
#include "gas.hpp"
#include <iostream>
#include <vector>

namespace DiveComputer {

// List of gases
class GasList {
public:
    GasList();

    std::vector<Gas> gases;

    void addGas(double o2Percent, double hePercent, GasType gasType, GasStatus gasStatus);
    void editGas(int index, double o2Percent, double hePercent, GasType gasType, GasStatus gasStatus);
    void deleteGas(int index);

    void clearGaslist();
    bool loadGaslistFromFile();
    bool saveGaslistToFile();
    void print();

};

// Global instance - this will be defined in gaslist.cpp
extern GasList g_gasList;

} // namespace DiveComputer

#endif