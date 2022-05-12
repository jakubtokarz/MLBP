#include "solution_verifier.h"
#include "instance.h"
#include "solution.h"

#include <sstream>
#include <algorithm>
#include <set>



/*************************************************************************************************/
/* BP ********************************************************************************************/
/*************************************************************************************************/
bool SolutionVerifier<BP>::verify(const Instance<BP>& inst, const Solution<BP>& sol, std::vector<std::string>* error_msg)
{
	bool ret = true;
	std::vector<int> levels(sol.total_bins, 0);  // size of each bin
	for (int i : inst.I) {
		int bin = sol.item_to_bins[i];
		if (bin >= sol.total_bins) {
			if (error_msg) {
				std::stringstream ss;
				ss << "Detected bin with id " << bin << " but solutions claims that there are only " << sol.total_bins << ".";
				error_msg->push_back(ss.str());
			}
			ret = false;
		} else if (bin < 0) {
			if (error_msg) {
				std::stringstream ss;
				ss << "Item " << i << " is not assigned to any bin.";
				error_msg->push_back(ss.str());
			}
			ret = false;
		} else {
			levels[bin] += inst.s[i];
		}
	}
	for (int bin = 0; bin < sol.total_bins; bin++) {
		if (levels[bin] > inst.smax) {
			if (error_msg) {
				std::stringstream ss;
				ss << "Bin " << bin << " of size " << levels[bin] << " exceeds maximum capaicty (" << inst.smax << ").";
				error_msg->push_back(ss.str());
			}
			ret = false;
		}
	}
	return ret;
}


/*************************************************************************************************/
/* MLBP ******************************************************************************************/
/*************************************************************************************************/
bool SolutionVerifier<MLBP>::verify(const Instance<MLBP>& inst, const Solution<MLBP>& sol, std::vector<std::string>* error_msg)
{
	bool ret = true;

	//1. All items are in the top bins
	//2. No bin contents exceed its capacity
	//3. Once items/bins have been put into the same bin, they have to stay together in all the upcoming bins

	std::vector<std::vector<int>> bins;  // size of each bin
	std::vector<std::vector<bool>> used_bins;
	for (int k : inst.M) {
		bins.push_back(std::vector<int> (inst.n[k], 0));
		used_bins.push_back(std::vector<bool>(inst.n[k-1], false));
		for (int i = 0; i < inst.n[0]; i++) {
			int curr = i;
			if (k > 1) {
				curr = sol.item_to_bins[k - 2][i];
			}
			int bin_idx = sol.item_to_bins[k-1][i];
			//1.
			if (k == inst.m && bin_idx < 0) {
				if (error_msg) {
					std::stringstream ss;
					ss << "Item " << i << " is not assigned to any bin in the top level.";
					error_msg->push_back(ss.str());
				}
				ret = false;
			}
			if (!used_bins[k - 1][curr]) {
				used_bins[k - 1][curr] = true;
				bins[k - 1][bin_idx] += inst.s[k - 1][curr];
			}
			
		}
	}

	for (int k : inst.M) {
		for (int j = 0; j < inst.n[k]; j++) {
			//2.
			if (bins[k-1][j] > inst.w[k][j]) {
				if (error_msg) {
					std::stringstream ss;
					ss << "Bin " << j << " at level " << k << " with contents of size " << bins[k - 1][j] << " exceeds maximum capaicty (" << inst.w[k][j] << ").";
					error_msg->push_back(ss.str());
				}
				ret = false;
			}
		}
	}

	for (int k : inst.M) {
		std::vector<std::vector<int>> same_bins_set;

		for (int i = 0; i < inst.n[0]; i++) {
			same_bins_set.push_back(std::vector<int>());
			for (int j = 1; j < same_bins_set[i].size(); j++) {
				//3.
				if (sol.item_to_bins[k-1][same_bins_set[i][0]] != sol.item_to_bins[k-1][same_bins_set[i][j]]) {
					if (error_msg) {
						std::stringstream ss;
						ss << "Items [" << same_bins_set[i][0] << ", " << same_bins_set[i][j] << "]" <<
							" where put together in level " << k-1 << ", but are in seperate bins in level " << k;
						error_msg->push_back(ss.str());
					}
					ret = false;
				}
			}
		}

		for (int i = 0; i < inst.n[k-1]; i++) {
			int item_or_bin_idx = sol.item_to_bins[k-1][i];
			same_bins_set[item_or_bin_idx].push_back(i);
		}
	}

	return ret;
}





/*************************************************************************************************/
/* CCMLBP ****************************************************************************************/
/*************************************************************************************************/
bool SolutionVerifier<CCMLBP>::verify(const Instance<CCMLBP>& inst, const Solution<CCMLBP>& sol, std::vector<std::string>* error_msg)
{
	bool ret = true;
	// TODO
	return ret;
}




/*************************************************************************************************/
/* MLBPCC ****************************************************************************************/
/*************************************************************************************************/
bool SolutionVerifier<MLBPCC>::verify(const Instance<MLBPCC>& inst, const Solution<MLBPCC>& sol, std::vector<std::string>* error_msg)
{
	bool ret = true;

	//1. All items are in the top bins
	//2. No bin contents exceed its capacity
	//3. Once items/bins have been put into the same bin, they have to stay together in all the upcoming bins
	//4. Conflicting items cannot share any of the bins 
		//assuming once items are put into the same itermidate bin they will also end up in the same final bin
			// -> check if conflicting items are not in the same bins only of the final level

	std::vector<std::vector<int>> bins;  // size of each bin
	std::vector<std::vector<bool>> used_bins;
	for (int k : inst.M) {
		bins.push_back(std::vector<int>(inst.n[k], 0));
		used_bins.push_back(std::vector<bool>(inst.n[k - 1], false));
		for (int i = 0; i < inst.n[0]; i++) {
			int curr = i;
			if (k > 1) {
				curr = sol.item_to_bins[k - 2][i];
			}
			int bin_idx = sol.item_to_bins[k - 1][i];
			//1.
			if (k == inst.m && bin_idx < 0) {
				if (error_msg) {
					std::stringstream ss;
					ss << "Item " << i << " is not assigned to any bin in the top level.";
					error_msg->push_back(ss.str());
				}
				ret = false;
			}
			if (!used_bins[k - 1][curr]) {
				used_bins[k - 1][curr] = true;
				bins[k - 1][bin_idx] += inst.s[k - 1][curr];
			}
		}
	}

	for (int k : inst.M) {
		for (int j = 0; j < inst.n[k]; j++) {
			//2.
			if (bins[k - 1][j] > inst.w[k][j]) {
				if (error_msg) {
					std::stringstream ss;
					ss << "Bin " << j << " at level " << k << " with contents of size " << bins[k - 1][j] << " exceeds maximum capaicty (" << inst.w[k][j] << ").";
					error_msg->push_back(ss.str());
				}
				ret = false;
			}
		}
	}

	for (int k : inst.M) {
		std::vector<std::vector<int>> same_bins_set;

		for (int i = 0; i < inst.n[0]; i++) {
			same_bins_set.push_back(std::vector<int>());
			for (int j = 1; j < same_bins_set[i].size(); j++) {
				//3.
				if (sol.item_to_bins[k - 1][same_bins_set[i][0]] != sol.item_to_bins[k - 1][same_bins_set[i][j]]) {
					if (error_msg) {
						std::stringstream ss;
						ss << "Items [" << same_bins_set[i][0] << ", " << same_bins_set[i][j] << "]" <<
							" were put together in level " << k - 1 << ", but are in seperate bins in level " << k;
						error_msg->push_back(ss.str());
					}
					ret = false;
				}
			}
		}

		for (int i = 0; i < inst.n[k - 1]; i++) {
			int item_or_bin_idx = sol.item_to_bins[k - 1][i];
			same_bins_set[item_or_bin_idx].push_back(i);
		}
	}


	for (int j = 0; j < inst.conflict.size()-1; j++) {
		for (int q = j+1; q < inst.conflict[0].size(); q++) {
			if (inst.conflict[j][q] == 1) {
				//4.
				if (sol.item_to_bins[inst.m-1][j] == sol.item_to_bins[inst.m-1][q]) {
					if (error_msg) {
						std::stringstream ss;
						ss << "Items [" << j << " and " << q << "]" <<
							" are conflicting and were put in the same bin (" << sol.item_to_bins[inst.m-1][j] << ") of the last level";
						error_msg->push_back(ss.str());
					}
					ret = false;
				}
			}
		}
	}

	return ret;
}




/*************************************************************************************************/
/* MLBPPO ****************************************************************************************/
/*************************************************************************************************/
bool SolutionVerifier<MLBPPO>::verify(const Instance<MLBPPO>& inst, const Solution<MLBPPO>& sol, std::vector<std::string>* error_msg)
{
	bool ret = true;
	// TOD
	return ret;
}




/*************************************************************************************************/
/* MLBPTW ****************************************************************************************/
/*************************************************************************************************/
bool SolutionVerifier<MLBPTW>::verify(const Instance<MLBPTW>& inst, const Solution<MLBPTW>& sol, std::vector<std::string>* error_msg)
{
	bool ret = true;
	// TOD
	return ret;
}




/*************************************************************************************************/
/* MLBPFC ****************************************************************************************/
/*************************************************************************************************/
bool SolutionVerifier<MLBPFC>::verify(const Instance<MLBPFC>& inst, const Solution<MLBPFC>& sol, std::vector<std::string>* error_msg)
{
	bool ret = true;
	// TOD
	return ret;
}
