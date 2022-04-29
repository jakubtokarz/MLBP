#include "mlbpformulation.h"

#include "instance.h"
#include "solution.h"
#include "users.h"


void MLBPFormulation::createDecisionVariables(IloEnv env, const Instance<MLBP>& inst)
{
	// decision variables x_{kij}
	x = IloArray<IloArray<IloNumVarArray>>(env, inst.m);
	int var_x_count = 0;

	for (int k : inst.M) {
		x[k-1] = IloArray<IloNumVarArray>(env, inst.n[k-1]);
		for (int i = 0; i < inst.n[k-1]; i++) {
			x[k-1][i] = IloNumVarArray(env, inst.n[k], 0, 1, ILOBOOL);
			var_x_count += inst.n[k];
		}
	}

	//SOUT() << "created " << bin_and_item_num << " x_{kij} variables" << std::endl;
	SOUT() << "created " << var_x_count << " x_{kij} variables" << std::endl;

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
	//1. All items must be put into level 1
	int num = 0;
	for (int i = 0; i < inst.n[0]; i++) {
		IloExpr sum(env);
		for (int j = 0; j < inst.n[1]; j++) {
			sum += x[0][i][j];
		}
		model.add(sum == 1);
		sum.end();
		num++;
	}
	SOUT() << "added " << num << " constraints to enforce the packing of each item" << std::endl;

	//2. All used bins (level 1+) must be put into the next level
	num = 0;
	for (int k = 2; k <= inst.m; k++) {
		for (int i = 0; i < inst.n[k - 1]; i++) {
			IloExpr sum(env); 
			for (int j = 0; j < inst.n[k]; j++) {
				sum += x[k-1][i][j];
			}
			model.add(sum == y[k - 2][i]);
			sum.end(); 
			num++; 
		}
	}
	SOUT() << "added " << num << " constraints to enforce the packing of each used bin to the next level" << std::endl;

	//3. All bins' capacity must be greater than the size of its contents
	num = 0;
	for (int k : inst.M) {
		num += inst.n[k];
		for (int j = 0; j < inst.n[k]; j++) {
			IloExpr sum(env);
			for (int i = 0; i < inst.n[k - 1]; i++) {
				sum += x[k-1][i][j] * inst.s[k-1][i];
			}
			model.add(sum <= y[k-1][j] * inst.w[k][j]);
			sum.end();
		}
	}
	SOUT() << "added " << num << " capacity constraints" << std::endl;
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
	// add cost of each bin that has been used
	sol.total_bin_cost = 0;
	for (int k = 0; k < inst.m; k++) {
		for (int j = 0; j < inst.n[k + 1]; j++) {
			if (cplex.getValue(y[k][j]) > 0.5) {
				sol.total_bin_cost += inst.c[k + 1][j];
			}
		}
	}

	//initalize sol.item_to_bins (size = m x n[0])
	// item_to_bins[level][item] = bin
	for (int i = 0; i < inst.m; i++) {
		sol.item_to_bins[i].assign(inst.n[0], -1);
	}
	
	for (int j = 0; j < inst.n[0]; j++) {
		int i = j;
		for (int k = 1; k < inst.m + 1; k++) {
			int res = -1;
			for (int h = 0; h < inst.n[k]; h++) {
				if (cplex.getValue(x[k - 1][i][h]) > 0.5) {
					res = h;
					break;
				}
			}
			sol.item_to_bins[k - 1][j] = res;
			i = res;
		}
	}

	////printing y
	//for (int k = 0; k < inst.m; k++) {
	//	SOUT() << "[";
	//	for (int j = 0; j < inst.n[k+1]; j++) {
	//		if (j > 0) {
	//			SOUT() << ", ";
	//		}
	//		SOUT() << cplex.getValue(y[k][j]);
	//	}
	//	SOUT() << "]" << std::endl;
	//}
}

//int reconstruct(IloArray<IloArray<IloNumVarArray>> x, IloCplex cplex, const Instance<MLBP>& inst, int level, int item) {
//	for (int j = 0; j < inst.n[level]; j++) {
//		if (cplex.getValue(x[level][item][j]) > 0.5) {
//			return j;
//		}
//	}
//	return -1;
//}
