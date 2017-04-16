#include <iostream>
#include <fstream>

#include "CityDB.h"
#include "city.h"

using std::string;
using std::make_pair;
using std::cout;
using std::endl;
using std::vector;

int main() {
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
	return 0;
}
