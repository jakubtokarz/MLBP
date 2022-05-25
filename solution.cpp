#include "solution.h"

#include "lib/util.h"
#include "users.h"

#include <algorithm>
#include <numeric>
#include <cassert>



/*****************************************************************************************/
/** Bin Packing Problem ******************************************************************/
/*****************************************************************************************/
Solution<BP>::Solution(const Instance<BP>& inst) : item_to_bins(inst.s.size())
{
	std::iota(item_to_bins.begin(), item_to_bins.end(), 0);
	total_bins = (int)inst.s.size();
}

std::ostream& operator<<(std::ostream& os, const Solution<BP>& sol)
{
	os << sol.item_to_bins;
	return os;
}




/*****************************************************************************************/
/** Multi-Level Bin Packing Problem ******************************************************/
/*****************************************************************************************/
//Solution<MLBP>::Solution(const Instance<MLBP>& inst) : db(-1)
//{
//}

Solution<MLBP>::Solution(const Instance<MLBP>& inst)
{
	total_bin_cost = 0;
	for (int k = 0; k < inst.m; k++) {
		item_to_bins.push_back(std::vector<int>(inst.n[0], -1));
		for (int j = 0; j < inst.n[k+1]; j++) {

			total_bin_cost += inst.c[k+1][j];
		}
	}
}

std::ostream& operator<<(std::ostream& os, const Solution<MLBP>& sol) {
	for (int i = 0; i < sol.item_to_bins.size(); i++) {
		os << sol.item_to_bins[i];
	}
	return os;
}




/*****************************************************************************************/
/** Class Constrained Multi-Level Bin Packing Problem ************************************/
/*****************************************************************************************/
Solution<CCMLBP>::Solution(const Instance<CCMLBP>& inst) : Solution<MLBP>(inst)
{
}

std::ostream& operator<<(std::ostream& os, const Solution<CCMLBP>& sol) {
	// TOD
	return os;
}




/*****************************************************************************************/
/** Multi-Level Bin Packing Problem with Conflict Constraints ****************************/
/*****************************************************************************************/
Solution<MLBPCC>::Solution(const Instance<MLBPCC>& inst) : Solution<MLBP>(inst)
{
}

std::ostream& operator<<(std::ostream& os, const Solution<MLBPCC>& sol) {
	for (int i = 0; i < sol.item_to_bins.size(); i++) {
		os << sol.item_to_bins[i];
	}
	return os;
}




/*****************************************************************************************/
/** Multi-Level Bin Packing Problem with Partial Orders **********************************/
/*****************************************************************************************/
Solution<MLBPPO>::Solution(const Instance<MLBPPO>& inst) : Solution<MLBP>(inst)
{
}

std::ostream& operator<<(std::ostream& os, const Solution<MLBPPO>& sol) {
	// TOD
	return os;
}




/*****************************************************************************************/
/** Multi-Level Bin Packing Problem with Time Windows ************************************/
/*****************************************************************************************/
Solution<MLBPTW>::Solution(const Instance<MLBPTW>& inst) : Solution<MLBP>(inst)
{
}

std::ostream& operator<<(std::ostream& os, const Solution<MLBPTW>& sol) {
	// TOD
	return os;
}




/*****************************************************************************************/
/** Multi-Level Bin Packing Problem with Fragmentation Constraints ***********************/
/*****************************************************************************************/
Solution<MLBPFC>::Solution(const Instance<MLBPFC>& inst) : Solution<MLBP>(inst)
{
}

std::ostream& operator<<(std::ostream& os, const Solution<MLBPFC>& sol) {
	// TOD
	return os;
}


