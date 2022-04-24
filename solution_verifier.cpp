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
	//3. Once items have been put into the same bin, they have to stay together in all the upcoming bins

	std::vector<std::vector<int> > bins;  // size of each bin
	for (int k = 0; k < inst.m; k++) {
		bins.push_back(std::vector<int> (inst.n[k+1], 0));
		for (int i = 0; i < inst.n[0]; i++) {
			int bin = sol.item_to_bins[k][i];
			//1.
			if (k == inst.m-1 && bin < 0) {
				if (error_msg) {
					std::stringstream ss;
					ss << "Item " << i << " is not assigned to any bin in the top level.";
					error_msg->push_back(ss.str());
				}
				ret = false;
			}
			bins[k][bin] += inst.s[k][i];
		}
	}
	for (int k = 0; k < inst.m; k++) {
		for (int j = 0; j < inst.n[k+1]; j++) {
			//2.
			if (bins[k][j] > inst.c[k+1][j]) {
				if (error_msg) {
					std::stringstream ss;
					ss << "Bin " << j << " with contents of size " << bins[k][j] << " exceeds maximum capaicty (" << inst.c[k + 1][j] << ").";
					error_msg->push_back(ss.str());
				}
				ret = false;
			}
		}
	}

	std::vector<std::vector<int>> same_bins_set;
	for (int i = 0; i < inst.n[0]; i++) {
		same_bins_set.push_back(std::vector<int>());
	}

	for (int k = 0; k < inst.m; k++) {
		for (int i = 0; i < same_bins_set.size(); i++) {
			for (int j = 1; j < same_bins_set[i].size(); j++) {
				//3.
				if (sol.item_to_bins[k][same_bins_set[i][0]] != sol.item_to_bins[k][same_bins_set[i][j]]) {
					if (error_msg) {
						std::stringstream ss;
						ss << "Items [" << same_bins_set[i][0] << ", " << same_bins_set[i][j] << "]" <<
							" where put together in level " << k << ", but are in seperate bins in level " << k+1;
						error_msg->push_back(ss.str());
					}
					ret = false;
				}
			}
		}

		for (int i = 0; i < inst.n[0]; i++) {
			same_bins_set[sol.item_to_bins[k][i]].push_back(i);
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
	// TODO
	return ret;
}




/*************************************************************************************************/
/* MLBPPO ****************************************************************************************/
/*************************************************************************************************/
bool SolutionVerifier<MLBPPO>::verify(const Instance<MLBPPO>& inst, const Solution<MLBPPO>& sol, std::vector<std::string>* error_msg)
{
	bool ret = true;
	// TODO
	return ret;
}




/*************************************************************************************************/
/* MLBPTW ****************************************************************************************/
/*************************************************************************************************/
bool SolutionVerifier<MLBPTW>::verify(const Instance<MLBPTW>& inst, const Solution<MLBPTW>& sol, std::vector<std::string>* error_msg)
{
	bool ret = true;
	// TODO
	return ret;
}




/*************************************************************************************************/
/* MLBPFC ****************************************************************************************/
/*************************************************************************************************/
bool SolutionVerifier<MLBPFC>::verify(const Instance<MLBPFC>& inst, const Solution<MLBPFC>& sol, std::vector<std::string>* error_msg)
{
	bool ret = true;
	// TODO
	return ret;
}
