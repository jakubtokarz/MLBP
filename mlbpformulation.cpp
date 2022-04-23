#include "mlbpformulation.h"

#include "instance.h"
#include "solution.h"
#include "users.h"


void MLBPFormulation::createDecisionVariables(IloEnv env, const Instance<MLBP>& inst)
{
	// decision variables x_{ijk}
	x = IloArray<IloArray<IloNumVarArray>>(env, inst.m);
	int var_num_x = 0;

	for (int i = 0; i < inst.m; i++) {
		x[i] = IloArray<IloNumVarArray>(env, inst.n[i]);
		for (int j : inst.B[i]) {
			x[i][j] = IloNumVarArray(env, inst.n[i], 0, 1, ILOBOOL);
		}	
		var_num_x += inst.n[i] * inst.n[i];
	}

	SOUT() << "created " << inst.m * var_num_x << " x_{ijk} variables" << std::endl;

	// decision variables y_{ij}
	y = IloArray<IloNumVarArray>(env, inst.m); 
	int var_num_y = 0;

	for (int i = 0; i < inst.m; i++) {
		y[i] = IloNumVarArray(env, inst.n[i], 0, 1, ILOBOOL);
		var_num_y += inst.n[i];
	}
		
	SOUT() << "created " << inst.m  * var_num_y << " y_{ij} variables" << std::endl;
}

void MLBPFormulation::addConstraints(IloEnv env, IloModel model, const Instance<MLBP>& inst)
{
	int bin_num = 0;
	// each item must be inserted into exactly one bin in each level
	for (int i = 0; i < inst.m; i++) {
		bin_num += inst.n[i];
		for (int j = 0; j < inst.n[i]; j++) {
			IloExpr sum(env);
			for (int k = 0; k < inst.n[i]; k++) {
				sum += x[i][j][k];
			}
			model.add(sum == 1);  // add constraint to model
			sum.end();
		}	
	}
	SOUT() << "added " << bin_num << " constraints to enforce the packing of each item to each of the levels" << std::endl;

	// the size of the content of a bin must not exceed the bin's capacity
	for (int i : inst.M) {
		for (int j = 0; j < inst.n[i]; j++) {
			IloExpr sum(env);
			for (int k : inst.B[i])
				//somehign wrong here
				sum += x[i][j][k] * inst.s[i+1][j];
			model.add(sum <= y[i][j] * inst.w[i+1][j]);
			sum.end();
		}
	}
	SOUT() << "added " << bin_num << " capacity constraints" << std::endl;

	//sort and use BP symmetry breaking constraint?

}

void MLBPFormulation::addObjectiveFunction(IloEnv env, IloModel model, const Instance<MLBP>& inst)
{
	IloExpr sum(env);
	for (int i = 0; i < inst.m; i++) {
		for (int j = 0; j < inst.n[i]; j++) {
			sum += y[i][j] * inst.c[i][j];
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

	for (int i = 0; i < inst.m; i++) {
		for (int j = 0; j < inst.n[i]; j++) {
			for (int k : inst.B[i]) {
				if (cplex.getValue(x[i][j][k]) > 0.5) {
					sol.total_bin_cost += inst.c[i][j];
					sol.item_to_bins[i][j] = k;
				}
			}
		}
	}
}

