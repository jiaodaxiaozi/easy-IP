// Petter Strandmark 2013–2014.

#include <catch.hpp>

#include <easy-ip.h>

using namespace std;

TEST_CASE("solve_boolean_infeasible")
{
	IP ip;
	auto x = ip.add_variable();
	auto y = ip.add_variable();
	ip.add_constraint(x + y >= 3);
	CHECK( ! ip.solve() );
}

TEST_CASE("solve_boolean_feasible")
{
	IP ip;
	auto x = ip.add_variable();
	auto y = ip.add_variable();
	ip.add_constraint(x + y >= 2);
	CHECK( ip.solve() );
}

TEST_CASE("solve_real_unbounded")
{
	IP ip;
	auto x = ip.add_variable(IP::Real, -1.0);
	auto y = ip.add_variable(IP::Real, -1.0);
	ip.add_constraint(x + y >= 3);
	CHECK( ! ip.solve() );
}

TEST_CASE("solve_real_feasible")
{
	IP ip;
	auto x = ip.add_variable(IP::Real, -1.0);
	auto y = ip.add_variable(IP::Real, -1.0);
	ip.set_bounds(-100, x, 100);
	ip.set_bounds(-100, y, 100);
	ip.add_constraint(x + y <= 3);
	CHECK( ip.solve() );
}

TEST_CASE("different_solvers")
{
	IP ip1, ip2;
	auto x = ip1.add_boolean();
	auto y = ip2.add_boolean();
	CHECK_THROWS(x + y);
	CHECK_THROWS(x - y);
	CHECK_THROWS(x || y);
	CHECK_THROWS(ip1.add_objective(y));
	CHECK_THROWS(ip2.set_bounds(0, x, 2));
	CHECK_THROWS(x == y);
	CHECK_THROWS(x >= y);
	CHECK_THROWS(x <= y);
	CHECK_THROWS(ip1.add_constraint(2.0 * y >= 8));
	CHECK_THROWS(ip2.add_constraint(x + x >= 8));
	ip1.solve();
	ip2.solve();
	CHECK_THROWS(ip1.get_solution(y));
	CHECK_THROWS(ip1.get_solution(!y));
}

auto create_soduku_IP(IP& ip, int n = 3)
	-> decltype(ip.add_boolean_cube(9, 9, 9))
{
	auto P = ip.add_boolean_cube(n*n, n*n, n*n);

	// Exactly one indicator equal to 1.
	for (int i = 0; i < n*n; ++i) {
		for (int j = 0; j < n*n; ++j) {
			Sum k_sum;
			for (int k = 0; k < n*n; ++k) {
				k_sum += P[i][j][k];
			}
			ip.add_constraint(k_sum == 1);

			// Advanced tip: One can use std::move to avoid a copy
			// here:
			//        ip.add_constraint(move(k_sum) == 1);
		}
	}

	// All rows have every number.
	for (int i = 0; i < n*n; ++i) {
		for (int k = 0; k < n*n; ++k) {
			Sum row_k_sum;
			for (int j = 0; j < n*n; ++j) {
				row_k_sum += P[i][j][k];
			}
			ip.add_constraint(row_k_sum == 1);
		}
	}

	// All columns have every number.
	for (int j = 0; j < n*n; ++j) {
		for (int k = 0; k < n*n; ++k) {
			Sum col_k_sum;
			for (int i = 0; i < n*n; ++i) {
				col_k_sum += P[i][j][k];
			}
			ip.add_constraint(col_k_sum == 1);
		}
	}

	// The n*n subsquares have every number.
	for (int i1 = 0; i1 < n; ++i1) {
		for (int j1 = 0; j1 < n; ++j1) {
			for (int k = 0; k < n*n; ++k) {
				Sum square_k_sum;
				for (int i2 = 0; i2 < n; ++i2) {
					for (int j2 = 0; j2 < n; ++j2) {
						square_k_sum += P[n*i1 + i2][n*j1 + j2][k];
					}
				}
				ip.add_constraint(square_k_sum == 1);
			}
		}
	}
	return P;
}

TEST_CASE("sudoku")
{
	using namespace std;

	IP ip;
	int n = 2;
	auto P = create_soduku_IP(ip, n);

	REQUIRE(ip.solve());

	vector<vector<int>> solution(n*n);

	cout << endl;
	for (int i = 0; i < n*n; ++i) {
		for (int j = 0; j < n*n; ++j) {
			solution[i].emplace_back();

			for (int k = 0; k < n*n; ++k) {
				if (P[i][j][k].bool_value()) {
					solution[i][j] = k + 1;
				}
			}
		}
	}

	for (int i = 0; i < n*n; ++i) {
		int row_sum = 0;
		int col_sum = 0;
		for (int j = 0; j < n*n; ++j) {
			row_sum += solution[i][j];
			col_sum += solution[j][i];
		}
		CHECK(row_sum == (n*n * (n*n + 1)) / 2);
		CHECK(col_sum == (n*n * (n*n + 1)) / 2);
	}
}

TEST_CASE("move_constructor")
{
	IP ip;
	auto x = ip.add_variable(IP::Real, -1.0);
	auto y = ip.add_variable(IP::Real, -1.0);
	ip.set_bounds(-100, x, 100);
	ip.set_bounds(-100, y, 100);
	ip.add_constraint(x + y <= 3);

	IP ip2 = std::move(ip);

	CHECK( ip2.solve() );
}

TEST_CASE("add_variable_as_booleans")
{
	IP ip;
	auto x = ip.add_variable_as_booleans(-3, 3);
	auto y = ip.add_variable_as_booleans({-3, -2, 3, 2, 1});
	ip.add_objective(x - y);
	CHECK(ip.solve());

	CHECK(x.value() == -3);
	CHECK(y.value() == 3);
}

TEST_CASE("add_max_consequtive_constraints")
{
	IP ip;
	vector<Sum> x;
	for (int i = 1; i <= 10; ++i) {
		x.emplace_back(ip.add_boolean(-1.0));
	}
	ip.add_max_consequtive_constraints(5, x);
	int num_solutions = 0;
	ip.solve();
	do {
		num_solutions++;
	} while (ip.next_solution());
	REQUIRE(num_solutions == 2);
	CHECK(x[0].value() == 1);
	CHECK(x[1].value() == 1);
	CHECK(x[2].value() == 1);
	CHECK(x[3].value() == 1);
	//CHECK(x[4].value() == ?);
	//CHECK(x[5].value() == ?);
	CHECK(x[6].value() == 1);
	CHECK(x[7].value() == 1);
	CHECK(x[8].value() == 1);
	CHECK(x[9].value() == 1);
}

TEST_CASE("add_min_consequtive_constraints-2-false")
{
	{
		IP ip;
		vector<Sum> x;
		Sum x_sum = 0;
		for (int i = 1; i <= 3; ++i) {
			x.emplace_back(ip.add_boolean());
			x_sum += x.back();
		}
		ip.add_constraint(x_sum == 2);
		ip.add_min_consequtive_constraints(2, x, false);
		int num_solutions = 0;
		REQUIRE(ip.solve(nullptr, true));
		do {
			num_solutions++;
		} while (ip.next_solution());
		REQUIRE(num_solutions == 2);
	}

	{
		IP ip;
		ip.set_external_solver(IP::Minisat);  // Avoid Cbc crash in old versions.
		vector<Sum> x;
		Sum x_sum = 0;
		for (int i = 1; i <= 5; ++i) {
			x.emplace_back(ip.add_boolean());
			x_sum += x.back();
		}
		ip.add_constraint(x_sum == 4);
		ip.add_min_consequtive_constraints(2, x, false);
		int num_solutions = 0;
		REQUIRE(ip.solve(nullptr, true));
		do {
			num_solutions++;
		} while (ip.next_solution());
		REQUIRE(num_solutions == 3);
	}
}

TEST_CASE("add_min_consequtive_constraints-2-true")
{
	IP ip;
	ip.set_external_solver(IP::Minisat);  // Avoid Cbc crash in old versions.
	vector<Sum> x;
	Sum x_sum = 0;
	for (int i = 1; i <= 3; ++i) {
		x.emplace_back(ip.add_boolean());
		x_sum += x.back();
	}
	ip.add_constraint(x_sum == 2);
	ip.add_min_consequtive_constraints(2, x, true);
	int num_solutions = 0;
	REQUIRE(ip.solve(nullptr, true));
	do {
		num_solutions++;
	} while (ip.next_solution());
	REQUIRE(num_solutions == 3);
}

TEST_CASE("add_min_consequtive_constraints-3-false")
{
	IP ip;
	ip.set_external_solver(IP::Minisat);  // Avoid Cbc crash in old versions.
	vector<Sum> x;
	Sum x_sum = 0;
	for (int i = 1; i <= 5; ++i) {
		x.emplace_back(ip.add_boolean());
		x_sum += x.back();
	}
	ip.add_constraint(x_sum == 3);
	ip.add_min_consequtive_constraints(3, x, false);
	int num_solutions = 0;
	REQUIRE(ip.solve(nullptr, true));
	do {
		num_solutions++;
	} while (ip.next_solution());
	CHECK(num_solutions == 3);
}

TEST_CASE("add_min_consequtive_constraints-3-true")
{
	IP ip;
	ip.set_external_solver(IP::Minisat);  // Avoid Cbc crash in old versions.
	vector<Sum> x;
	Sum x_sum = 0;
	for (int i = 1; i <= 5; ++i) {
		x.emplace_back(ip.add_boolean());
		x_sum += x.back();
	}
	ip.add_constraint(x_sum == 3);
	ip.add_min_consequtive_constraints(3, x, true);
	int num_solutions = 0;
	REQUIRE(ip.solve(nullptr, true));
	do {
		num_solutions++;
	} while (ip.next_solution());
	REQUIRE(num_solutions == 5);
}

TEST_CASE("add_min_consequtive_constraints-5-false")
{
	IP ip;
	ip.set_external_solver(IP::Minisat);  // Avoid Cbc crash in old versions.
	vector<Sum> x;
	Sum x_sum = 0;
	for (int i = 1; i <= 12; ++i) {
		x.emplace_back(ip.add_boolean());
		x_sum += x.back();
	}
	ip.add_constraint(x_sum == 6);
	ip.add_min_consequtive_constraints(5, x, false);
	int num_solutions = 0;
	REQUIRE(ip.solve(nullptr, true));
	do {
		num_solutions++;
	} while (ip.next_solution());
	CHECK(num_solutions == 7);
}

TEST_CASE("add_min_consequtive_constraints-5-true")
{
	IP ip;
	ip.set_external_solver(IP::Minisat);  // Avoid Cbc crash in old versions.
	vector<Sum> x;
	Sum x_sum = 0;
	for (int i = 1; i <= 12; ++i) {
		x.emplace_back(ip.add_boolean());
		x_sum += x.back();
	}
	ip.add_constraint(x_sum == 3);
	ip.add_min_consequtive_constraints(5, x, true);
	int num_solutions = 0;
	REQUIRE(ip.solve(nullptr, true));
	do {
		num_solutions++;
	} while (ip.next_solution());
	REQUIRE(num_solutions == 4);
}

TEST_CASE("add_min_max_consequtive_constraints-4")
{
	IP ip;
	ip.set_external_solver(IP::Minisat);  // Avoid Cbc crash in old versions.
	vector<Sum> x;
	Sum x_sum = 0;
	for (int i = 1; i <= 12; ++i) {
		x.emplace_back(ip.add_boolean());
		x_sum += x.back();
	}
	ip.add_min_consequtive_constraints(4, x, false);
	ip.add_max_consequtive_constraints(4, x);

	int num_solutions = 0;
	REQUIRE(ip.solve(nullptr, true));
	do {
		CHECK(x_sum.value() <= 8);
		int consequtive = 0;
		for (auto& xx : x) {
			if (xx.value() > 0.5) {
				consequtive++;
			}
			else {
				consequtive = 0;
			}
			CHECK(consequtive <= 4);
		}
		num_solutions++;
	} while (ip.next_solution());
	CHECK(num_solutions == 20);
}

TEST_CASE("add_min_max_consequtive_constraints-5")
{
	IP ip;
	ip.set_external_solver(IP::Minisat);  // Avoid Cbc crash in old versions.
	vector<Sum> x;
	Sum x_sum = 0;
	for (int i = 1; i <= 12; ++i) {
		x.emplace_back(ip.add_boolean());
		x_sum += x.back();
	}
	ip.add_min_consequtive_constraints(5, x, false);
	ip.add_max_consequtive_constraints(5, x);
	int num_solutions = 0;
	REQUIRE(ip.solve(nullptr, true));
	do {
		int consequtive = 0;
		for (auto& xx : x) {
			if (xx.value() > 0.5) {
			}
			else {
				consequtive = 0;
			}
			CHECK(consequtive <= 5);
		}
		num_solutions++;
	} while (ip.next_solution());
	CHECK(num_solutions == 12);
}

TEST_CASE("solve_relaxation")
{
	IP ip;
	auto x = ip.add_boolean();
	auto y = ip.add_boolean();
	Sum objective = -x - y;
	ip.add_objective(objective);
	ip.add_constraint(2*x <= 1);
	ip.add_constraint(2*y <= 1);
	REQUIRE(ip.solve_relaxation());
	CHECK(Approx(objective.value()) == -1);
	CHECK(Approx(x.value()) == 0.5);
	CHECK(Approx(y.value()) == 0.5);
	REQUIRE(ip.solve());
	CHECK(Approx(objective.value()) == 0);
	CHECK(Approx(x.value()) == 0);
	CHECK(Approx(y.value()) == 0);
}

TEST_CASE("trivial_constraint")
{
	IP ip;
	Sum s = 0;
	CHECK(ip.add_constraint(s <= 1) == 0);
	CHECK_THROWS(ip.add_constraint(s <= -1));
	CHECK_THROWS(ip.add_constraint(-1, s, -1));
	CHECK_THROWS(ip.add_constraint(1, s, 1));
	auto x = ip.add_boolean();
	CHECK(ip.add_constraint(x <= 1) == 1);
}

TEST_CASE("sum_without_variables")
{
	Sum s = 4;
	CHECK(s.value() == 4);
}
