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

	//SOUT() << "created " << bin_and_item_num << " x_{kij} variables" << std::endl;
	SOUT() << "created " << var_x_count << " x_{kij} variables" << std::endl;

	// decision variables y_{kj}
	y = IloArray<IloNumVarArray>(env, inst.m + 1);
	int amount_of_bins = 0;

	for (int k : inst.M) {
		y[k] = IloNumVarArray(env, inst.n[k], 0, 1, ILOBOOL);
		amount_of_bins += inst.n[k];
	}

	SOUT() << "created " << amount_of_bins << " y_{kj} variables" << std::endl;

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
	SOUT() << "added " << num << " constraints to enforce the packing of each item" << std::endl;

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
	SOUT() << "added " << num << " constraints to enforce the packing of each used bin to the next level" << std::endl;

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
	SOUT() << "added " << num << " capacity constraints" << std::endl;

	//-------------------------------
	//4. Populate c

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
		for (int i = 0; i < inst.n[k-1]; i++) {
			for (int j = 0; j < inst.n[k]; j++) {
				for (int a = 0; a < inst.n[0]; a++) {
					model.add(x[k][i][j] * c[k-1][a][i] <= x[k][i][j] * c[k][a][j]);
					num++;
				}
			}
		}
	}
	//-------------------------------

	//5. If item a and item b are conflicting, they cannot be in the same bin j for all the levels
	for (int k : inst.M) {
		for (int j = 0; j < inst.n[k]; j++) {
			for (int a = 0; a < inst.n[0]-1; a++) {
				for (int b = a+1; b < inst.n[0]; b++) {
					model.add(inst.conflict[a][b] + c[k][a][j] + c[k][b][j] < 3);
				}
			}			
		}
	}




	//4. conflicting items can't share a bin

	////conflicting items can't share the same bin in level 1
	//for (int j = 0; j < inst.n[1]; j++) {
	//	for (int a = 0; a < inst.n[0] - 1; a++) {
	//		for (int b = a + 1; b < inst.n[0]; b++) {
	//			if (inst.conflict[a][b] == 1) {
	//				model.add(!(x[0][a][j] == 1 && x[0][b][j] == 1));
	//			}
	//		}
	//	}
	//}

	//additional data strucutre of conliciting bins?? - nah

	////conflicting bins can't share the same bin in levels 2+
	//for (int k = 2; k <= inst.m; k++) {
	//	for (int j = 0; j < inst.n[k]; j++) {
	//		for (int a = 0; a < inst.n[k-1] - 1; a++) {
	//			for (int b = a + 1; b < inst.n[k-1]; b++) {
	//				int item_in_a_idx = -1; //idx of item that goes to bin a
	//				int item_in_b_idx = -1; //idx of item that goes to bin b
	//				if (inst.conflict[item_in_a_idx][item_in_b_idx] == 1) {
	//					model.add(x[k-1][a][j] != 1 || x[k-1][b][j] != 1);
	//				}
	//			}
	//		}
	//	}
	//}


		//IloExpr sum(env);
		//for (int a = 0; a < inst.n[0]; a++) {
		//	sum += x[1][a][j];
		//}
		//model.add(sum == ())

	//if a and b are conflicting
	//inst.conflict[a][b] == 1
	//then they cannot be put into the same bin
	//x[k][a][j] != x[k][b][j]
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

