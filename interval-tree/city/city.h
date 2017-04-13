#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::endl;
using std::ostream;

class City {
private:
	string   city_;
	uint64_t low_;
	uint64_t high_;
public:
	City() : City("", 0, 0) {

	}

	City(string city, uint64_t low, uint64_t high) :
			city_(city), low_(low), high_(high) {

	}

	City(const City &t) {
		city_ = t.city_;
		low_  = t.low_;
		high_ = t.high_;
	}

	const string &getCity() const {
		return city_;
	}

	uint64_t getLowTemperature() const {
		return low_;
	}

	uint64_t getHighTemperature() const {
		return high_;
	}

	string to_string() const {
		return "[" + city_ + "," + std::to_string(low_) + "," + std::to_string(high_) + "]";
	}

	bool operator== (const City &rhs) const {
		bool rc = false;

		if (low_ == rhs.low_ && high_ == rhs.high_ && city_ == rhs.city_) {
			rc = true;
		}
		return rc;
	}

	bool operator< (const City &rhs) const {
		bool rc = false;
		if (low_ <= rhs.low_) {
			rc = true;
		}
		return rc;
	}

    friend class boost::serialization::access; 

    template <typename Archive> 
    void serialize(Archive &ar, const unsigned int version) { 
        ar & BOOST_SERIALIZATION_NVP(city_);
        ar & BOOST_SERIALIZATION_NVP(low_);
        ar & BOOST_SERIALIZATION_NVP(high_);
    } 

	friend ostream& operator<<(ostream& os, const City& t);
};

ostream& operator<<(ostream& os, const City& t) {
	os << t.to_string();
	return os;
}

#endif
