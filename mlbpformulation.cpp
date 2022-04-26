#include "mlbpformulation.h"

#include "instance.h"
#include "solution.h"
#include "users.h"


void MLBPFormulation::createDecisionVariables(IloEnv env, const Instance<MLBP>& inst)
{
	// decision variables x_{ijk}
	x = IloArray<IloArray<IloNumVarArray>>(env, inst.m);
	int var_x_count = 0;

	for (int k : inst.M) {
		x[k-1] = IloArray<IloNumVarArray>(env, inst.n[k-1]);
		for (int i = 0; i < inst.n[k-1]; i++) {
			x[k-1][i] = IloNumVarArray(env, inst.n[k], 0, 1, ILOBOOL);
			var_x_count += inst.n[k];
		}
	}

	//SOUT() << "created " << bin_and_item_num << " x_{ikj} variables" << std::endl;
	SOUT() << "created " << var_x_count << " x_{ikj} variables" << std::endl;

	// decision variables y_{kj}
	y = IloArray<IloNumVarArray>(env, inst.m); 
	int amount_of_bins = 0;

	for (int k : inst.M) {
		y[k-1] = IloNumVarArray(env, inst.n[k], 0, 1, ILOBOOL);
		amount_of_bins += inst.n[k];
	}
		
	SOUT() << "created " << amount_of_bins << " y_{kj} variables" << std::endl;
}

void MLBPFormulation::addConstraints(IloEnv env, IloModel model, const Instance<MLBP>& inst)
{

	//int amount_of_bins = 0;
	//each used item/bin needs to be placed 1 bin of the next level
	for (int k : inst.M) {
		//amount_of_bins += inst.n[k];
		for (int i = 0; i < inst.n[k - 1]; i++) {
			IloExpr item_in_x_bins(env);
			for (int j = 0; j < inst.n[k]; j++) {
				item_in_x_bins += x[k-1][i][j]; //(x[k-1][i][j] * y[k - 1][j])
			}
			//wrong, forces all bins to be used no matter if they are empty or not
			model.add(item_in_x_bins == 1); 
			item_in_x_bins.end();
		}
	}


	//the amount of used bins of level k should be equal to the amount of edges between k-1 and k
	IloExpr num_needed_edges(env);
	num_needed_edges += inst.n[0];
	
	for (int k : inst.M) {
		IloExpr num_edges_between_i_and_j(env);
		IloExpr num_used_bins_at_level_k(env);

		for (int j = 0; j < inst.n[k]; j++) {
			num_used_bins_at_level_k += y[k - 1][j];
			for (int i = 0; i < inst.n[k-1]; i++) {			
				num_edges_between_i_and_j += x[k-1][i][j];
			}
		}
		model.add(num_edges_between_i_and_j == num_needed_edges);

		num_needed_edges *= 0; //reset
		num_needed_edges += num_used_bins_at_level_k;
		num_edges_between_i_and_j.end();
		num_used_bins_at_level_k.end();
	}
	num_needed_edges.end();


	//capacity constraint
	int bin_num = 0;
	for (int k : inst.M) {
		bin_num += inst.n[k];
		for (int j = 0; j < inst.n[k]; j++) {
			IloExpr sum(env);
			for (int i = 0; i < inst.n[k - 1]; i++) {
				sum += x[k-1][i][j] * inst.s[k-1][i];
			}
			model.add(sum <= y[k-1][j] * inst.w[k][j]);
			sum.end();
		}
	}
	SOUT() << "added " << bin_num << " capacity constraints" << std::endl;

}

void MLBPFormulation::addObjectiveFunction(IloEnv env, IloModel model, const Instance<MLBP>& inst)
{
	IloExpr sum(env);
	for (int k : inst.M) {
		for (int j = 0; j < inst.n[k]; j++) {
			sum += y[k-1][j] * inst.c[k][j];
		}
	}
	model.add(IloMinimize(env, sum));
	sum.end();
}

void MLBPFormulation::extractSolution(IloCplex cplex, const Instance<MLBP>& inst, Solution<MLBP>& sol)
{
	//sol.item_to_bins.size() should equal to m
	for (int i = 0; i < sol.item_to_bins.size(); i++) {
		//sol.item_to_bins[i].size() should equal to amount of items (n[0])
		sol.item_to_bins[i].assign(inst.n[0], -1);
	}
	sol.total_bin_cost = 0;

	// add cost of each bin that has been used
	for (int k = 0; k < inst.m; k++) {
		for (int j = 0; j < inst.n[k + 1]; j++) {
			if (cplex.getValue(y[k][j]) > 0.5) {
				sol.total_bin_cost += inst.c[k + 1][j];
			}
		}
	}

	//for (int k = 0; k < inst.m; k++) {
	//	for (int i = 0; i < inst.n[0]; i++) {
	//		for (int j = 0; j < inst.n[k + 1]; j++) {
	//			if (cplex.getValue(x[k][i][j]) > 0.5) {
	//				//swapped idx for readability
	//				sol.item_to_bins[k][i] = j;
	//			}
	//		}
	//	}
	//}

	// item_to_bins[level][item] = bin
	for (int j = 0; j < inst.n[0]; j++) {
		int i = j;
		for (int k = 1; k < inst.m + 1; k++) {
			int res = -1;
			for (int h = 0; h < inst.n[k]; h++) {
				SOUT() << "x of: " << k - 1 << i << h << " = " << cplex.getValue(x[k - 1][i][h]) << std::endl;
				if (cplex.getValue(x[k - 1][i][h]) > 0.5) {
					res = h;
					break;
				}
				
			}
			if (res == -1) {
				SOUT() << "didn't find x of: " << k - 1 << i << std::endl;
			}
			sol.item_to_bins[k - 1][j] = res;
			i = res;
		}
	}

	SOUT() << y.getSize() << std::endl;
	SOUT() << y[0].getSize() << std::endl;
	for (int k = 0; k < inst.m; k++) {
		SOUT() << "[";
		for (int j = 0; j < inst.n[k+1]; j++) {
			if (j > 0) {
				SOUT() << ", ";
			}
			SOUT() << cplex.getValue(y[k][j]);
		}
		SOUT() << "]" << std::endl;
	}


}

//int reconstruct(IloArray<IloArray<IloNumVarArray>> x, IloCplex cplex, const Instance<MLBP>& inst, int level, int item) {
//	for (int j = 0; j < inst.n[level]; j++) {
//		if (cplex.getValue(x[level][item][j]) > 0.5) {
//			return j;
//		}
//	}
//	return -1;
//}
