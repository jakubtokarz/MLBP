#include "mlbpccformulation.h"

#include "instance.h"
#include "solution.h"
#include "users.h"


void MLBPCCFormulation::createDecisionVariables(IloEnv env, const Instance<MLBPCC>& inst)
{
	// decision variables x_{kij}
	x = IloArray<IloArray<IloNumVarArray>>(env, inst.m + 1);
	int var_x_count = 0;

	for (int k : inst.M) {
		x[k] = IloArray<IloNumVarArray>(env, inst.n[k - 1]);
		for (int i = 0; i < inst.n[k - 1]; i++) {
			x[k][i] = IloNumVarArray(env, inst.n[k], 0, 1, ILOBOOL);
			var_x_count += inst.n[k];
		}
	}
	MIP_OUT(TRACE) << "created " << var_x_count << " x_{kij} variables" << std::endl;

	// decision variables y_{kj}
	y = IloArray<IloNumVarArray>(env, inst.m + 1);
	int amount_of_bins = 0;

	for (int k : inst.M) {
		y[k] = IloNumVarArray(env, inst.n[k], 0, 1, ILOBOOL);
		amount_of_bins += inst.n[k];
	}
	MIP_OUT(TRACE) << "created " << amount_of_bins << " y_{kj} variables" << std::endl;

	// decision variables z_{kij}
	z = IloArray<IloArray<IloNumVarArray>>(env, inst.m + 1);
	int var_z_count = 0;

	for (int k : inst.M) {
		z[k] = IloArray<IloNumVarArray>(env, inst.n[0]);
		for (int i = 0; i < inst.n[0]; i++) {
			z[k][i] = IloNumVarArray(env, inst.n[k], 0, 1, ILOBOOL);
			var_z_count += inst.n[k];
		}
	}
	MIP_OUT(TRACE) << "created " << var_z_count << " z_{kij} variables" << std::endl;
}

void MLBPCCFormulation::addConstraints(IloEnv env, IloModel model, const Instance<MLBPCC>& inst)
{
	//1. All items must be put into level 1
	int num = 0;
	for (int i = 0; i < inst.n[0]; i++) {
		IloExpr sum(env);
		for (int j = 0; j < inst.n[1]; j++) {
			sum += x[1][i][j];
		}
		model.add(sum == 1);
		sum.end();
		num++;
	}
	MIP_OUT(TRACE) << "added " << num << " constraints to enforce the packing of each item" << std::endl;

	//2. All used bins (level 1+) must be put into the next level
	num = 0;
	for (int k = 2; k <= inst.m; k++) {
		for (int i = 0; i < inst.n[k - 1]; i++) {
			IloExpr sum(env);
			for (int j = 0; j < inst.n[k]; j++) {
				sum += x[k][i][j];
			}
			model.add(sum == y[k - 1][i]);
			sum.end();
			num++;
		}
	}
	MIP_OUT(TRACE) << "added " << num << " constraints to enforce the packing of each used bin to the next level" << std::endl;

	//3. All bins' capacity must be greater than the size of its contents
	num = 0;
	for (int k : inst.M) {
		num += inst.n[k];
		for (int j = 0; j < inst.n[k]; j++) {
			IloExpr sum(env);
			for (int i = 0; i < inst.n[k - 1]; i++) {
				sum += x[k][i][j] * inst.s[k - 1][i];
			}
			model.add(sum <= y[k][j] * inst.w[k][j]);
			sum.end();
		}
	}
	MIP_OUT(TRACE) << "added " << num << " capacity constraints" << std::endl;

	//----------------------------------------------------------------------
	//4. Populate z

	//a) If there is a connection between item a and bin j of level 1, then item a is in bin j of level 1
	num = 0;
	for (int a = 0; a < inst.n[0]; a++) {
		for (int j = 0; j < inst.n[1]; j++) {
			model.add(x[1][a][j] == z[1][a][j]);
			num++;
		}
	}
	MIP_OUT(TRACE) << "added " << num << " constraints to enforce the packing of each item to level 1 bins" << std::endl;

	//b) If item a is in bin i of level k-1 and there is a connection between bin i of level k-1 and bin j of level k, 
	//then item a needs to be in a bin j of level k
	num = 0;
	for (int k = 2; k <= inst.m; k++) {
		for (int i = 0; i < inst.n[k-1]; i++) {
			for (int j = 0; j < inst.n[k]; j++) {
				for (int a = 0; a < inst.n[0]; a++) {
					model.add(IloIfThen(env, z[k - 1][a][i] == 1 && x[k][i][j] == 1, z[k][a][j] == 1));
					num++;
				}
			}
		}
	}
	MIP_OUT(TRACE) << "added " << num << " constraints to enforce the packing in between levels" << std::endl;
	//----------------------------------------------------------------------

	//5. If item a and item b are conflicting, they cannot be in the same bin j for all the levels
	num = 0;
	for (int k : inst.M) {
		for (int j = 0; j < inst.n[k]; j++) {
			for (int a = 0; a < inst.n[0]-1; a++) {
				for (int b = a+1; b < inst.n[0]; b++) {
					if (inst.conflict[a][b] == 1) {
						model.add(z[k][a][j] + z[k][b][j] <= y[k][j]);
						num++;
					}
				}
			}			
		}
	}
	MIP_OUT(TRACE) << "added " << num << " constraints to conflicting items" << std::endl;
}

void MLBPCCFormulation::addObjectiveFunction(IloEnv env, IloModel model, const Instance<MLBPCC>& inst)
{
	IloExpr sum(env);
	for (int k : inst.M) {
		for (int j = 0; j < inst.n[k]; j++) {
			sum += y[k][j] * inst.c[k][j];
		}
	}
	model.add(IloMinimize(env, sum));
	sum.end();
}

void MLBPCCFormulation::extractSolution(IloCplex cplex, const Instance<MLBPCC>& inst, Solution<MLBPCC>& sol)
{
	// add cost of each bin that has been used
	sol.total_bin_cost = 0;
	for (int k : inst.M) {
		for (int j = 0; j < inst.n[k]; j++) {
			if (cplex.getValue(y[k][j]) > 0.5) {
				sol.total_bin_cost += inst.c[k][j];
			}
		}
	}

	// initalize sol.item_to_bins (size = m x n[0])
	for (int i = 0; i < inst.m; i++) {
		sol.item_to_bins[i].assign(inst.n[0], -1);
	}

	for (int j = 0; j < inst.n[0]; j++) {
		int i = j;
		for (int k = 1; k < inst.m + 1; k++) {
			int res = -1;
			for (int h = 0; h < inst.n[k]; h++) {
				if (cplex.getValue(x[k][i][h]) > 0.5) {
					res = h;
					break;
				}
			}
			sol.item_to_bins[k - 1][j] = res;
			i = res;
		}
	}
}
