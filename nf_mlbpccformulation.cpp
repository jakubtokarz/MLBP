#include "nf_mlbpccformulation.h"

#include "instance.h"
#include "solution.h"
#include "users.h"



void NF_MLBPCCFormulation::createDecisionVariables(IloEnv env, const Instance<MLBPCC>& inst)
{
	MIP_OUT(TRACE) << "NF" << std::endl;

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

	//SOUT() << "created " << bin_and_item_num << " x_{kij} variables" << std::endl;
	MIP_OUT(TRACE) << "created " << var_x_count << " x_{kij} variables" << std::endl;

	// decision variables y_{kj}
	y = IloArray<IloNumVarArray>(env, inst.m + 1);
	int amount_of_bins = 0;

	for (int k : inst.M) {
		y[k] = IloNumVarArray(env, inst.n[k], 0, 1, ILOBOOL);
		amount_of_bins += inst.n[k];
	}

	MIP_OUT(TRACE) << "created " << amount_of_bins << " y_{kj} variables" << std::endl;


	// decision variables f_{kij}
	f = IloArray<IloArray<IloNumVarArray>>(env, inst.m + 1);
	int var_f_count = 0;

	for (int k : inst.M) {
		f[k] = IloArray<IloNumVarArray>(env, inst.n[k - 1]);
		for (int i = 0; i < inst.n[k - 1]; i++) {
			if (k == 1) {
				f[k][i] = IloNumVarArray(env, inst.n[k], 0, 1, ILOBOOL);
			}
			else {
				f[k][i] = IloNumVarArray(env, inst.n[k], 0, inst.n[0], ILOINT);
			}
			var_f_count += inst.n[k];
		}
	}

	// should be same as x
	MIP_OUT(TRACE) << "created " << var_f_count << " f_{kij} variables" << std::endl;

	// decision variables c_{kij}
	c = IloArray<IloArray<IloNumVarArray>>(env, inst.m + 1);
	int var_c_count = 0;

	for (int k : inst.M) {
		c[k] = IloArray<IloNumVarArray>(env, inst.n[0]);
		for (int i = 0; i < inst.n[0]; i++) {
			c[k][i] = IloNumVarArray(env, inst.n[k], 0, 1, ILOBOOL);
			var_c_count += inst.n[k];
		}
	}

	//SOUT() << "created " << bin_and_item_num << " c_{kij} variables" << std::endl;
	SOUT() << "created " << var_c_count << " c_{kij} variables" << std::endl;
}

void NF_MLBPCCFormulation::addConstraints(IloEnv env, IloModel model, const Instance<MLBPCC>& inst)
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

	//4. Every item node must have an outoging flow of 1 (source is redundant and therefore is omitted)
	num = 0;
	for (int i = 0; i < inst.n[0]; i++) {
		IloExpr sum(env);
		for (int j = 0; j < inst.n[1]; j++) {
			sum += f[1][i][j];
		}
		model.add(sum == 1);
		sum.end();
		num++;
	}
	MIP_OUT(TRACE) << "added " << num << " constraints to enforce the source constraint" << std::endl;

	//5. Every bin's incoming flow equals to its outgoing flow
	num = 0;
	for (int k = 2; k < inst.m; k++) { //skip items and the last level
		for (int j = 0; j < inst.n[k]; j++) {
			IloExpr total_flow(env);
			for (int p = 0; p < inst.n[k - 1]; p++) {
				total_flow += f[k][p][j];
			}
			for (int q = 0; q < inst.n[k + 1]; q++) {
				total_flow -= f[k + 1][j][q];
			}
			model.add(total_flow == 0);
			total_flow.end();
			num++;
		}
	}
	MIP_OUT(TRACE) << "added " << num << " constraints to enforce the equal flow constraint" << std::endl;

	//6. All the flow (amount of items) is consumed in the end (sink is redundant and therefore is omitted)
	num = 0;
	IloExpr sum(env);
	for (int j = 0; j < inst.n[inst.m]; j++) {
		for (int i = 0; i < inst.n[inst.m - 1]; i++) {
			sum += f[inst.m][i][j];
		}
	}
	model.add(sum == inst.n[0]);
	sum.end();
	num++;
	MIP_OUT(TRACE) << "added " << num << " constraints to enforce the sink constraint" << std::endl;

	//-------------------------------
	//7. Populate c

	//a) If there is a connection between item a and bin j of level 1, then item a is in bin j of level 1
	num = 0;
	for (int a = 0; a < inst.n[0]; a++) {
		for (int j = 0; j < inst.n[1]; j++) {
			model.add(x[1][a][j] <= x[1][a][j] * c[1][a][j]);
			num++;
		}
	}
	//SOUT() << "added " << num << " constraints to enforce the packing of each item" << std::endl;

	//b) If there is a connection between bin i of level k-1 and bin j of level k and item a is in bin i of level k-1, 
	//then item a needs to be in a bin j of level k
	num = 0;
	for (int k = 2; k <= inst.m; k++) {
		for (int i = 0; i < inst.n[k - 1]; i++) {
			for (int j = 0; j < inst.n[k]; j++) {
				for (int a = 0; a < inst.n[0]; a++) {
					model.add(x[k][i][j] * c[k - 1][a][i] <= x[k][i][j] * c[k][a][j]);
					num++;
				}
			}
		}
	}
	//-------------------------------

	//8. If item a and item b are conflicting, they cannot be in the same bin j for all the levels
	for (int k : inst.M) {
		for (int j = 0; j < inst.n[k]; j++) {
			for (int a = 0; a < inst.n[0] - 1; a++) {
				for (int b = a + 1; b < inst.n[0]; b++) {
					model.add(inst.conflict[a][b] + c[k][a][j] + c[k][b][j] < 3);
				}
			}
		}
	}

	//----------------------------------------------------------------------
	// implicated LP-Relaxation constraint that can improve the performance

	//Every connection between i and j should be less than or equal to whether bin i is used for all k
	num = 0;
	for (int k = 2; k <= inst.m; k++) {
		for (int i = 0; i < inst.n[k - 1]; i++) {
			for (int j = 0; j < inst.n[k]; j++) {
				model.add(x[k][i][j] <= y[k - 1][i]);
				num++;
			}
		}
	}
	MIP_OUT(TRACE) << "added " << num << " constraints to enforce LP x-y relaxation" << std::endl;

	// Every connection between i and j should be active/inactive for both x and f
	num = 0;
	for (int k = 1; k <= inst.m; k++) {
		for (int i = 0; i < inst.n[k - 1]; i++) {
			for (int j = 0; j < inst.n[k]; j++) {
				model.add(f[k][i][j] <= x[k][i][j] * inst.n[0]);
				num++;
			}
		}
	}

	MIP_OUT(TRACE) << "added " << num << " constraints to enforce LP x-f relaxation" << std::endl;

	//----------------------------------------------------------------------
	//Symmetry breaking constraints

	//1. If bin j is better than or as good as bin j+1, use bin j first
	num = 0;
	for (int k = 1; k <= inst.m; k++) {
		for (int j = 0; j < inst.n[k]; j++) {
			for (int q = 0; j < inst.n[k]; j++) {
				if (j == q)continue;
				if (inst.s[k][j] >= inst.s[k][q] && inst.w[k][j] >= inst.w[k][q] && inst.c[k][j] <= inst.c[k][q]) {
					model.add(y[k][j] >= y[k][q]);
				}
				num++;
			}
		}
	}

	MIP_OUT(TRACE) << "added " << num << " better bin symmetry breaking constraints" << std::endl;
}

void NF_MLBPCCFormulation::addObjectiveFunction(IloEnv env, IloModel model, const Instance<MLBPCC>& inst)
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

void NF_MLBPCCFormulation::extractSolution(IloCplex cplex, const Instance<MLBPCC>& inst, Solution<MLBPCC>& sol)
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
				if (cplex.getValue(x[k][i][h]) > 0.5) {
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