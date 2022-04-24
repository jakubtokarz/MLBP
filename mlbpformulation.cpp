#include "mlbpformulation.h"

#include "instance.h"
#include "solution.h"
#include "users.h"


void MLBPFormulation::createDecisionVariables(IloEnv env, const Instance<MLBP>& inst)
{
	// decision variables x_{ikj}
	x = IloArray<IloArray<IloNumVarArray>>(env, inst.n[0]);	
	int bin_num = 0;

	for (int i = 0; i < inst.n[0]; i++) {
		x[i] = IloArray<IloNumVarArray>(env, inst.m);
		bin_num = 0;
		for (int k = 0; k < inst.m; k++) {
			x[i][k] = IloNumVarArray(env, inst.n[k+1], 0, 1, ILOBOOL);
			bin_num += inst.n[k + 1];
		}
	}

	SOUT() << "created " << inst.n[0] *  bin_num << " x_{ikj} variables" << std::endl;

	// decision variables y_{kj}
	y = IloArray<IloNumVarArray>(env, inst.m); 

	for (int k = 0; k < inst.m; k++) {
		y[k] = IloNumVarArray(env, inst.n[k +1], 0, 1, ILOBOOL);
	}
		
	SOUT() << "created " << bin_num << " y_{kj} variables" << std::endl;
//	SOUT() << "size: " << x.getSize() << ", " << x[0].getSize() << ", " << x[0][0].getSize() << std::endl;
//	SOUT() << "size: " << x.getSize() << ", " << x[1].getSize() << ", " << x[1][1].getSize() << std::endl;
//	SOUT() << "sizYe: " << y.getSize() << ", " << y[0].getSize() << ", " << y[1].getSize() << std::endl;
}

void MLBPFormulation::addConstraints(IloEnv env, IloModel model, const Instance<MLBP>& inst)
{
	int bin_num = 0;

	// each item must be inserted into exactly one bin in every level
	for (int i = 0; i < inst.n[0]; i++) {
		IloExpr item_i_in_levels(env);
		bin_num = 0;
		for (int k = 0; k < inst.m; k++) {
			IloExpr bins_have_item_i(env);
			for (int j = 0; j < inst.n[k+1]; j++) {
				bins_have_item_i += x[i][k][j];
				item_i_in_levels += x[i][k][j];
			}
			bin_num += inst.n[k + 1];
			model.add(bins_have_item_i == 1);
			bins_have_item_i.end();
		}	
		model.add(item_i_in_levels == inst.m);
		item_i_in_levels.end();
	}

	SOUT() << "added " << bin_num + inst.m << " constraints to enforce the packing of each item to every level" << std::endl;

	// the size of the content of a bin must not exceed the bin's capacity
	for (int k = 0; k < inst.m; k++) {
		for (int j = 0; j < inst.n[k+1]; j++) {
			IloExpr sum(env);
			for (int i = 0; i < inst.n[0]; i++)
				sum += x[i][k][j] * inst.s[k][i]; //TODO: check addressing [i][j] or [j][i] -> im assuming [level][item]
			model.add(sum <= y[k][j] * inst.w[k+1][j]); //TODO^ -> im assuming [level][bin]
			sum.end();
		}
	}
	SOUT() << "added " << bin_num << " capacity constraints" << std::endl;

	//sort and use BP symmetry breaking constraint?

}

void MLBPFormulation::addObjectiveFunction(IloEnv env, IloModel model, const Instance<MLBP>& inst)
{
	IloExpr sum(env);
	for (int k = 0; k < inst.m; k++) {
		for (int j = 0; j < inst.n[k+1]; j++) {
			sum += y[k][j] * inst.c[k+1][j];
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

	// item_to_bins[level][item] = bin
	for (int k = 0; k < inst.m; k++) {
		for (int i = 0; i < inst.n[0]; i++) {
			for (int j = 0; j < inst.n[k + 1]; j++) {
				if (cplex.getValue(x[i][k][j]) > 0.5) {
					sol.total_bin_cost += inst.c[k+1][j];
					sol.item_to_bins[k][i] = j;
				}
			}
		}
	}
}

