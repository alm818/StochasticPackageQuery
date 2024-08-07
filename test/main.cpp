#include <iostream>

#include "util/udebug.hpp"
#include "spq/spq.hpp"
#include "core/stat.hpp"
#include "spq/bounder.hpp"
#include "core/checker.hpp"
#include "solver/starter.hpp"

#include <gurobi_c.h>

void analyzeAll(){
	vector<int> nStocks = {3, 4, 5, 6};
	vector<int> nPaths = {2, 4};
	for (auto nStock : nStocks){
		for (auto nPath : nPaths){
			INIT(pro);
			CLOCK("a");
			string filePath = fmt::format("resource/sqls/_stocks_{}_{}.spaql", nStock, nPath);
			auto spq = parseSpaqlFromFile(filePath);
			if (spq){
				cout << "Success!\n" << spq;
				deb(spq->validate());
				unique_ptr<Stat> stat = std::make_unique<Stat>();
				stat->analyze(spq->tableName);
			}
			STOP("a");
			PRINT(pro);
		}
	}
}

#include "solver/taylor.hpp"

void testInit(){
	string filePath = fmt::format("resource/sqls/_stocks_4_2.spaql");
	auto spq = parseSpaqlFromFile(filePath);
	if (spq){
		spq->validate();
		unique_ptr<Stat> stat = std::make_unique<Stat>();
		stat->analyze(spq->tableName);
		size_t N = 10000;
		double E = 50;
		Bounder bounder (spq, N, E);
		bounder.set(0);
		deb(spq->executable(), spq);
		Starter starter (spq, {}, {});
		starter.solve();
		// Taylor taylor (spq, {}, {{0,1}}, {
		// 	{"soft_deterministic_constraint", false},
		// 	{"max_number_of_iterations", 50}
		// });
		// taylor.solve();
	}
}

// void test(){
// 	INIT(pro);
// 	CLOCK("a");
// 	for (int i = 6; i <= 6; i ++){
// 		string filePath = fmt::format("resource/sqls/_stocks_{}_2.spaql", i);
// 		auto spq = parseSpaqlFromFile(filePath);
// 		if (spq){
// 			// cout << "Success!\n" << spq;
// 			// deb(spq->validate());
// 			spq->validate();
// 			unique_ptr<Stat> stat = std::make_unique<Stat>();
// 			stat->analyze(spq->tableName);
// 			size_t N = 10000;
// 			double E = 50;
// 			CLK(pro, "initBounder");
// 			Bounder bounder (spq, N, E);
// 			STP(pro, "initBounder");
// 			vector<double> hards;
// 			for (double i = -10; i <= 10; i ++) hards.push_back(i);
// 			CLK(pro, "hard");
// 			bounder.generate(hards);
// 			for (auto h : hards){
// 				bounder.set(h);
// 				STP(pro, "hard");
// 				deb(spq->executable(), spq);
// 				CLK(pro, "taylorinit");
// 				Taylor taylor (spq, {}, {
// 					{"soft_deterministic_constraint", false},
// 					{"max_number_of_iterations", 50},
// 					{"dependency_var", true}});
// 				STP(pro, "taylorinit");
// 				CLK(pro, "taylor");
// 				taylor.solve();
// 				STP(pro, "taylor");
// 				deb(filePath, h, taylor.status);
// 				SPQChecker chk (spq);
// 				chk.display(taylor.getSol());
// 			}
// 			// chk.display({{104,36.7424},{984,38.2576}});
// 		}
// 		STOP("a");
// 		// PRINT(pro);	
// 	}
// }

#include "oneapi/tbb/concurrent_unordered_map.h"
using oneapi::tbb::concurrent_unordered_map;
#include <pcg_random.hpp>
#include "util/uconfig.hpp"

void testTBB(){
	// concurrent_unordered_map<int, int> cmap;
	// cmap[0] = 1;
	// deb(cmap);
	// pcg32 gen (Config::getInstance()->seed());
	// vector<unsigned int> seeds;
	// for (int i = 0; i < 80; i ++) seeds.push_back(gen());
	// deb(seeds);
}

void testOmp(){
	size_t N = 10000000;
	size_t R = 10;
	vector<double> v (N);
	std::iota(v.begin(), v.end(), 0);
	INIT(pro);
	for (size_t r = 0; r < R; ++r){
		CLK(pro, "a");
		double norm = 0;
		vector<double> vv (N);
		for (size_t i = 0; i < N; ++i){
			norm += v[i]*v[i];
		}
		norm = sqrt(norm);
		for (size_t i = 0; i < N; ++i){
			vv[i] = v[i]/norm;
		}
		STP(pro, "a");
	}
	int core = 80;
	for (size_t r = 0; r < R; ++r){
		CLK(pro, "b");
		vector<double> vv (N);
		double norm;
		#pragma omp parallel num_threads(core)
		{
			double norm_ = 0;
			#pragma omp for nowait
			for (size_t i = 0; i < N; ++i){
				norm_ += v[i]*v[i];
			}
			#pragma omp atomic
			norm += norm_;
			#pragma omp barrier
			#pragma omp master
			{
				norm = sqrt(norm);
			}
			#pragma omp barrier
			#pragma omp for nowait
			for (size_t i = 0; i < N; ++i){
				vv[i] = v[i]/norm;
			}
		}
		STP(pro, "b");
	}
	PRINT(pro);
}

void testNumeric(){
	vector<double> v = {0, 0};
	deb(normalize(v));
	deb(v);
}

void testgb(){
	GRBenv* env = NULL;
	GRBmodel* model = NULL;
	ckg(GRBemptyenv(&env), env);
	ckg(GRBsetintparam(env, GRB_INT_PAR_PRESOLVE, GRB_PRESOLVE_OFF), env);
	ckg(GRBsetintparam(env, GRB_INT_PAR_SIFTING, 2), env);
	ckg(GRBsetintparam(env, GRB_INT_PAR_METHOD, GRB_METHOD_DUAL), env);
	// ckgb(GRBsetintparam(env, GRB_INT_PAR_OUTPUTFLAG, 0), env);
	ckg(GRBstartenv(env), env);
	int numvars = 2;
	vector<double> obj = {1, 1};
	vector<double> lb = {0, 1};
	vector<double> ub = {4, GRB_INFINITY};
	vector<char> vtype = {GRB_CONTINUOUS, GRB_CONTINUOUS};
	ckgb(GRBnewmodel(env, &model, NULL, numvars, obj.data(), lb.data(), ub.data(), vtype.data(), NULL), env, model);
	ckgb(GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MINIMIZE), env, model);
	int numconstrs = 4;
	int numnz = 7;
	vector<int> cbeg = {0, 1, 3, 5, 7};
	vector<int> cind = {1, 0, 1, 0, 1, 0, 1};
	vector<double> cval = {1, 1, 2, 1, 2, 3, 2};
	vector<char> sense = {GRB_LESS_EQUAL, GRB_GREATER_EQUAL, GRB_LESS_EQUAL, GRB_GREATER_EQUAL};
	vector<double> rhs = {7, 5, 15, 6};
	ckgb(GRBaddconstrs(model, numconstrs, numnz, cbeg.data(), cind.data(), cval.data(), sense.data(), rhs.data(), NULL), env, model);
	ckgb(GRBupdatemodel(model), env, model);
	ckgb(GRBoptimize(model), env, model);
	int status;
	ckgb(GRBgetintattr(model, GRB_INT_ATTR_STATUS, &status), env, model);
	if (status == GRB_OPTIMAL){
		vector<double> sol (numvars);
    	ckgb(GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, numvars, sol.data()), env, model);
		deb(sol);
	}
	if (model) GRBfreemodel(model);
	if (env) GRBfreeenv(env);
}

int main() {
	// testgb();
	// testNumeric();
	// analyzeAll();
	// test();
	testInit();
	// testOmp();
	// testTBB();
	// testHighs();
	// map<string, Option> ok = {{"ok","1"},{"ok1",1.5},{"ok2",false}};
	// deb(ok);
	// for (auto p : ok) deb(p.second.which());
}