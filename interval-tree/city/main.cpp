#include <iostream>
#include <fstream>
#include <set>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/set.hpp>

#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/icl/interval_map.hpp>
#include <boost/icl/split_interval_map.hpp>

using namespace boost::icl;
using std::string;
using std::make_pair;
using std::cout;
using std::endl;
using std::set;

class City {
private:
	string   city_; /* city name */
	uint64_t low_;  /* low temperature on perticular day */
	uint64_t high_; /* high temperature on perticular day */
public:
	City() : City("", 0, 0) { }

	City(string city, uint64_t low, uint64_t high) :
			city_(city), low_(low), high_(high) { }

	bool operator== (const City &rhs) const {
		return (low_ == rhs.low_ && high_ == rhs.high_ && city_ == rhs.city_);
	}

	bool operator< (const City &rhs) const {
		return low_ <= rhs.low_;
	}

    friend class boost::serialization::access; 

    template <typename Archive> 
    void serialize(Archive &ar, const unsigned int version) { 
        ar & BOOST_SERIALIZATION_NVP(city_);
        ar & BOOST_SERIALIZATION_NVP(low_);
        ar & BOOST_SERIALIZATION_NVP(high_);
    } 
};

typedef std::set<City> CitySet;
typedef boost::icl::interval_map<uint64_t, CitySet> ranges;

namespace boost { namespace serialization {
	/*
	 * Key in interval_map is discrete_interval -- serialize it.
	 */
	template <typename Archive, typename K>
	void save(Archive& ar, boost::icl::discrete_interval<K> const& di, unsigned) {
		auto const& bb = di.bounds().bits();
		auto const& l  = di.lower();
		auto const& u  = di.upper();

		ar << BOOST_SERIALIZATION_NVP(bb)
		   << BOOST_SERIALIZATION_NVP(l)
		   << BOOST_SERIALIZATION_NVP(u);
	}

	template <typename Archive, typename K>
	void serialize(Archive& ar, boost::icl::discrete_interval<K>& di, unsigned v) {
		boost::serialization::split_free(ar, di, v);
	}

	/*
	 * Serialize interval_map
	 */
	template <typename Archive, typename K, typename V>
	void save(Archive& ar, boost::icl::interval_map<K, V> const& im, unsigned) {
		auto sz = im.iterative_size();
		ar << BOOST_SERIALIZATION_NVP(sz);

		for (const auto &kv : im) {
			auto &di = kv.first;
			auto &v  = kv.second;
			ar << boost::serialization::make_nvp("INTERVAL", di);
			ar << boost::serialization::make_nvp("SET", v);
		}
	}

	template <typename Archive, typename K, typename V>
	void serialize(Archive& ar, boost::icl::interval_map<K, V>& is, unsigned v) {
		boost::serialization::split_free(ar, is, v);
	}
}
}

void serialize(const ranges &tree) {
	std::ofstream wf{"/tmp/interval.dat"};
	boost::archive::xml_oarchive oa(wf);

	oa << BOOST_SERIALIZATION_NVP(tree);
	wf.flush();
	wf.close();
}

int main() {
	ranges tree;

	City    cpur{"cpur", 28, 39};
	CitySet cset{cpur};
	auto    crange = interval<uint64_t>::right_open(28, 39);
	tree.add(std::make_pair(crange, cset));

	serialize(tree);
	return 0;
}