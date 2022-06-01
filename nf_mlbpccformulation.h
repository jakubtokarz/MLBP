#ifndef __NF_MLBPCC_FORMULATION_H__
#define __NF_MLBPCC_FORMULATION_H__


#include "problems.h"
#include "mipsolver.h"

template<typename> struct Instance;
template<typename> struct Solution;

class NF_MLBPCCFormulation : public MIPFormulation<MLBPCC>
{
public:
	virtual void createDecisionVariables(IloEnv env, const Instance<MLBPCC>& inst);
	virtual void addConstraints(IloEnv env, IloModel model, const Instance<MLBPCC>& inst);
	virtual void addObjectiveFunction(IloEnv env, IloModel model, const Instance<MLBPCC>& inst);
	virtual void extractSolution(IloCplex cplex, const Instance<MLBPCC>& inst, Solution<MLBPCC>& sol);
private:
	// binary decision variables x_{kij}: item/bin i in level k-1 is inserted into bin j at level k (=1) or not (=0)
	IloArray<IloArray<IloNumVarArray>> x;

	// binary decision variables y_{kj}: bin j of level k is used (=1) or not (=0)
	IloArray<IloNumVarArray> y;

	// decision variables f_{kij}: flow between item/bin i in level k-1 and bin j at level k (=0) if no flow and (=[1,n[0]] if there is flow)
	IloArray<IloArray<IloNumVarArray>> f;

	// binary decision variables c_{kij}: item i is in bin j of level k (=1) or not (=0) 
	IloArray<IloArray<IloNumVarArray>> c;
};


#endif // __NF_MLBPCC_FORMULATION_H__
