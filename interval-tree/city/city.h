#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include <string>
#include <iostream>
#include <utility>

using std::string;
using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::make_pair;
using std::to_string;

using Temperature = std::pair<int64_t, int64_t>;
using Humidity = std::pair<int64_t, int64_t>;

class City {
private:
	string         city_;
	Temperature    temp_;
	Humidity       hum_;
public:
	City() : City("", make_pair(0,0), make_pair(0,0)) {

	}

	City(string &city, Temperature temp, Humidity hum) :
			city_(city), temp_(temp), hum_(hum) {

	}

	City(string &&city, Temperature temp, Humidity hum) :
			city_(std::move(city)), temp_(temp), hum_(hum) {

	}

	City(const City &t) {
		city_ = t.city_;
		temp_ = t.temp_;
		hum_  = t.hum_;
	}

	const string &getName() const {
		return city_;
	}

	int64_t getLowTemperature() const {
		return temp_.first;
	}

	int64_t getHighTemperature() const {
		return temp_.second;
	}

	int64_t getLowHumidity() const {
		return hum_.first;
	}

	int64_t getHighHumidity() const {
		return hum_.second;
	}

	string to_string() const {
		return "[" + city_ + " TEMP(" + ::to_string(temp_.first) + " " +
				::to_string(temp_.second) + ")" + " HUM(" + ::to_string(hum_.first) + 
				" " + ::to_string(hum_.second) + ")]";
	}

    friend class boost::serialization::access; 

    template <typename Archive> 
    void serialize(Archive &ar, const unsigned int version) { 
        ar & BOOST_SERIALIZATION_NVP(city_);
        ar & BOOST_SERIALIZATION_NVP(temp_);
        ar & BOOST_SERIALIZATION_NVP(hum_);
    } 

	friend ostream& operator<<(ostream& os, const City& t);
};

ostream& operator<<(ostream& os, const City& t) {
	os << t.to_string();
	return os;
}

#endif