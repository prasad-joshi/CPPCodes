#include <iostream>
#include <fstream>
#include <chrono>

#include "CityDB.h"
#include "city.h"

using std::string;
using std::make_pair;
using std::cout;
using std::endl;
using std::vector;

#define NRECORDS 10000

template <typename T>
class randomize {
private:
	std::mt19937 eng_{std::random_device{}()};
public:
	template <typename U = T>
	typename std::enable_if<std::is_integral<U>::value, U>::type
	getRandomValue(U min, U max) {
		return std::uniform_int_distribution<U>{min, max}(eng_);
	}

	template <typename U = T>
	typename std::enable_if<std::is_floating_point<U>::value, U>::type
	getRandomValue(U min, U max) {
		return std::uniform_real_distribution<U>{min, max}(eng_);
	}

	template <typename U = T>
	typename std::enable_if<std::is_same<string, U>::value, U>::type
	getRandomValue(int length) {
		string rc;
		rc.reserve(length);
		for (auto i = 0; i < length; i++) {
			auto c = std::uniform_int_distribution<int>{0, 51}(eng_);
			if (c < 26) {
				rc += (char) ('a' + c);
			} else {
				c -= 26;
				rc += (char) ('A' + c);
			}
		}

		return rc;
	}
};

#define SWAP(x, y) do { \
	auto __t__ = x; \
	x = y;          \
	y = __t__;      \
} while(0)

void benchmark_query(CityDB &db, const string &query) {
	size_t         size;
	const uint64_t ntimes = 10;

	auto s = std::chrono::high_resolution_clock::now();
	for (auto i = 0; i < ntimes; i++) {
		auto rc = db.query(query);
		if (i == 0) {
			size = rc.size();
		} else {
			assert(rc.size() == size);
		}
	}
	auto e  = std::chrono::high_resolution_clock::now();
	auto rt = std::chrono::duration_cast<std::chrono::nanoseconds>(e-s).count();
	cout << "query(" << query << ")," << size << "," << rt/ntimes << endl;
}

void benchmark_insert(CityDB &db) {
	const uint64_t    nrecords = NRECORDS;
	randomize<double> randTemp;
	randomize<double> randHum;
	randomize<string> randCity;

	auto s = std::chrono::high_resolution_clock::now();
	for (auto i = 0; i < nrecords; i++) {
		auto tl = randTemp.getRandomValue(-20.0, 50.0);
		auto th = randTemp.getRandomValue(-20.0, 50.0);
		auto hl = randHum.getRandomValue(10, 70);
		auto hh = randHum.getRandomValue(10, 70);
		auto n = randCity.getRandomValue(10);

		if (tl > th) {
			SWAP(tl, th);
		}

		if (hl > hh) {
			SWAP(hl, hh);
		}

		City c{n, make_pair(tl, th), make_pair(hl, hh)};
		db.addCity(c);
	}
	auto e  = std::chrono::high_resolution_clock::now();
	auto rt = std::chrono::duration_cast<std::chrono::nanoseconds>(e-s).count();

	cout << "insert(" << nrecords << "),0," << rt << endl;


}

void benchmark() {
	CityDB db;

	cout <<"Operation,Result,Time\n";
	benchmark_insert(db);

	/* QUERY */
	string q("T < 0");
	benchmark_query(db, q);

	q.assign("T < 0 && H < 20");
	benchmark_query(db, q);

	q.assign("T < 0 || H < 20");
	benchmark_query(db, q);

	q.assign("T > -50");
	benchmark_query(db, q);

	q.assign("T > 50");
	benchmark_query(db, q);

	for (auto i = -1; i < 2; i++) {
		string q("T == ");
		q += std::to_string(i);
		benchmark_query(db, q);
	}

	for (auto i = 10; i < 13; i++) {
		string q("H == ");
		q += std::to_string(i);
		benchmark_query(db, q);
	}

	for (auto t = -1; t < 2; t++) {
		for (auto h = 10; h < 12; h++) {
			string q = "T == " + std::to_string(t) + " && " + "H == " + std::to_string(h);
			benchmark_query(db, q);
		}
	}

	for (auto t = -1; t < 2; t++) {
		for (auto h = 10; h < 12; h++) {
			string q = "T == " + std::to_string(t) + " || " + "H == " + std::to_string(h);
			benchmark_query(db, q);
		}
	}

	/* SAVE */
	auto s = std::chrono::high_resolution_clock::now();
	db.save("/tmp/DB.dat");
	auto e = std::chrono::high_resolution_clock::now();
	auto rt = std::chrono::duration_cast<std::chrono::nanoseconds>(e-s).count();
	cout << "serialize(),0," << rt << endl;

	/* LOAD */
	CityDB db1;
	s = std::chrono::high_resolution_clock::now();
	db1.load("/tmp/DB.dat");
	e = std::chrono::high_resolution_clock::now();
	rt = std::chrono::duration_cast<std::chrono::nanoseconds>(e-s).count();
	cout << "deserialize(),0," << rt << endl;
}

static void test() {
	CityDB db;

	City pune{"Pune", make_pair(10, 30), make_pair(35, 50)};
	db.addCity(pune);

	City cpur{"cpur", make_pair(28, 39), make_pair(35, 40)};
	db.addCity(cpur);

	City kpur{"kpur", make_pair(11, 31), make_pair(45, 55)};
	db.addCity(kpur);

	City npur{"npur", make_pair(25, 42), make_pair(45, 50)};
	db.addCity(npur);

	City xpur{"xpur", make_pair(35, 45), make_pair(10, 60)};
	db.addCity(xpur);

	City tpur{"tpur", make_pair(25, 51), make_pair(45, 85)};
	db.addCity(tpur);

	db.save("/tmp/DB.dat");

	cout << "After Loading DB.\n";
	CityDB db1;
	db1.load("/tmp/DB.dat");

	string q("T < 0 && H > -100");
	const auto rc = db.query(q);
	for (const auto &e : rc) {
		cout << e.to_string() << endl;
	}
#if 0
	temp_index.serialize("/tmp/TempIndex.dat");
	cout << "Before removal.\n";
	display(temp_index);

	remove_city(temp_index, tpur);
	cout << "After removal tpur " << endl;
	display(temp_index);

	remove_city(temp_index, xpur);
	cout << "After removal xpur " << endl;
	display(temp_index);

	temp_index.add(std::make_pair(xrange, xset));
	cout << "After adding xpur " << endl;
	display(temp_index);

	temp_index.add(std::make_pair(trange, tset));
	cout << "After adding tpur " << endl;
	display(temp_index);

	remove_city(temp_index, xpur);
	cout << "After removal xpur " << endl;
	display(temp_index);

	remove_city(temp_index, tpur);
	cout << "After removal tpur " << endl;
	display(temp_index);

	serialize(temp_index);
#endif
}

int main() {
	benchmark();
	return 0;
}
