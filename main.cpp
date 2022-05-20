#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>

#include "lib/util.h"
#include "lib/log.h"
#include "lib/lutze.h"      // time measurement
#include "lib/argparser.h"  // simple command line argument parser

#include "users.h"              // register users/submodules for writing to stdout, register your own user name for writing to stdout
#include "problems.h"           // contains for each problem a struct type that is used in other classes / algorithms
#include "instance.h"           // contains for each problem an instance class that contains all information related to a specific instance of a problem
#include "solution.h"           // contains for each problem a solution class that represents a solution to a specific instance
#include "solution_verifier.h"  // verifies if a solution is feasible with respect to a given instance object

// MIP stuff
#include "mipsolver.h"			 // generic mip solver, uses CPLEX to solve mips
#include "bpformulation.h"		 // mip formulation for the bin packing problem
#include "mlbpformulation.h"	 // mip formulation 1 for the multi-level bin packing problem
#include "nf_mlbpformulation.h"  // mip formulation 2 for the multi-level bin packing problem


int main(int argc, char* argv[])
{
	int ticket = LuTze::start();  // get ticket for time measurement

	//SOUT() << "# version: " << VERSION << std::endl;  // print git version to stdout

	// read command line arguments
	ArgParser arg_parser(argc, argv);

	try {
		arg_parser.add<std::string>("ifile", "Input file", "inst/bp/bp1.inst");
		arg_parser.add<std::string>("prob", "Problem: Bin Packing (BP), Multi-Level Bin Packing (MLBP)", "BP", {"BP", "MLBP"});
		arg_parser.add<int>("ttime", "total time limit", 0, 0, std::numeric_limits<int>::max());
		arg_parser.add<int>("threads", "Number of used threads", 1, 0, 100);

		if (arg_parser.isHelpSet()) {
			arg_parser.help(std::cout);
			return EXIT_SUCCESS;
		}

		arg_parser.parse();
	} catch (const std::exception& exp) {
		std::cerr << "ERROR: " << exp.what() << std::endl;
		return EXIT_FAILURE;
	}

	SOUT() << arg_parser;  // print arguments to stdout

	std::string instance_filename = arg_parser.get<std::string>("ifile");

	if (arg_parser.get<std::string>("prob") == "BP") {
		/*****************************************************************************************/
		/** Bin Packing Problem ******************************************************************/
		/*****************************************************************************************/
		Instance<BP> inst(instance_filename);  // read BP instance

		SOUT() << "instance: " << instance_filename << std::endl;
		SOUT() << "\t" << inst << std::endl;

		Solution<BP> sol(inst);  // create empty BP solution

		// setup MIP solver
		MIPSolver<BP> mip_solver;
		mip_solver.setTimeLimit(arg_parser.get<int>("ttime"));  // set time limit; 0 -> no time limit
		mip_solver.setThreads(arg_parser.get<int>("threads"));  // number of used threads, should be always one for our experiments

		mip_solver.setFormulation<BPFormulation>();  // set MIP formulation

		/**************************************************************/
		auto status = mip_solver.run(inst, sol);  /** run MIP solver **/
		/**************************************************************/

		if (status == MIPSolver<BP>::Feasible || status == MIPSolver<BP>::Optimal) {
			SOUT() << std::endl;
			SOUT() << "# best solution:" << std::endl;
			SOUT() << "best objective value:\t" << inst.objective(sol) << std::endl;
			SOUT() << "best dual bound value:\t" << sol.db << std::endl;
			SOUT() << "optimality gap:\t" << (double)(inst.objective(sol) - sol.db) / (double)inst.objective(sol) * 100.0 << "%" << std::endl;
			SOUT() << "solution:\n\t" << sol.item_to_bins << std::endl;
		}

		// check if solution is feasible
		std::vector<std::string> msg;
		if (!SolutionVerifier<BP>::verify(inst, sol, &msg)) {
			std::cerr << "ERROR:" << std::endl;
			for (auto it = msg.begin(); it != msg.end(); ++it)
				std::cerr << *it << std::endl;
			return EXIT_FAILURE;
		}

	} else if (arg_parser.get<std::string>("prob") == "MLBP") {
		/*****************************************************************************************/
		/** Multi-Level Bin Packing Problem ******************************************************/
		/*****************************************************************************************/
		Instance<MLBP> inst(instance_filename);  // read MLBP instance
		
		SOUT() << "instance: " << instance_filename << std::endl;
		SOUT() << "\t" << inst << std::endl;

		Solution<MLBP> sol(inst);  // create empty MLBP solution

		// setup MIP solver
		MIPSolver<MLBP> mip_solver;
		mip_solver.setTimeLimit(arg_parser.get<int>("ttime"));  // set time limit; 0 -> no time limit
		mip_solver.setThreads(arg_parser.get<int>("threads"));  // number of used threads, should always be one for our experiments

		//mip_solver.setFormulation<MLBPFormulation>();  // set MIP formulation
		mip_solver.setFormulation<NF_MLBPFormulation>();  // set MIP formulation

		/**************************************************************/
		auto status = mip_solver.run(inst, sol);  /** run MIP solver **/
		/**************************************************************/

		if (status == MIPSolver<MLBP>::Feasible || status == MIPSolver<MLBP>::Optimal) {
			SOUT() << std::endl;
			SOUT() << "# best solution:" << std::endl;
			SOUT() << "best objective value:\t" << inst.objective(sol) << std::endl;
			SOUT() << "best dual bound value:\t" << sol.db << std::endl;
			SOUT() << "optimality gap:\t" << (double)(inst.objective(sol) - sol.db) / (double)inst.objective(sol) * 100.0 << "%" << std::endl;
			SOUT() << "solution item to bin:\n\t" << sol.item_to_bins << std::endl;
			
			std::vector<std::vector<int>> bins;
			for (int k : inst.M) {
				bins.push_back(std::vector<int>(inst.n[k - 1], -1));
			}
			bins[0] = sol.item_to_bins[0];
			for (int k = 1; k < inst.m; k++) {
				for (int i = 0; i < inst.n[k]; i++) {
					for (int j = 0; j < inst.n[k-1]; j++) {
						if (sol.item_to_bins[k - 1][j] == i) {
							bins[k][i] = sol.item_to_bins[k][j];
						}
					}
				}
			}
			SOUT() << "solution k-1 to k:\n\t" << bins << std::endl;
		}

		// check if solution is feasible
		std::vector<std::string> msg;
		if (!SolutionVerifier<MLBP>::verify(inst, sol, &msg)) {
			std::cerr << "ERROR:" << std::endl;
			for (auto it = msg.begin(); it != msg.end(); ++it)
				std::cerr << *it << std::endl;
			return EXIT_FAILURE;
		}
	}

	// print total computation time
	double time = LuTze::end(ticket);
	SOUT() << "CPU time:\t" << time << std::endl;

	return EXIT_SUCCESS;
}
