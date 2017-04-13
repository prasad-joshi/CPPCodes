#include <iostream>
#include <fstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/set.hpp>

#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/icl/interval_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/icl/split_interval_map.hpp>

using namespace boost::icl;
using std::string;
using std::make_pair;
using std::cout;
using std::endl;

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

typedef boost::container::flat_set<City>  TempSet;
typedef boost::icl::interval_map<uint64_t, TempSet> ranges;

namespace boost { namespace serialization {
	/*
	 * Value in interval_map is boost flat_set -- serialize each element in flat_set
	 */
	template<typename Archive, typename Key, typename Compare, typename Allocator>
	void save(Archive & ar,
    			const boost::container::flat_set<Key, Compare, Allocator> &t,
    			const unsigned int) {
		collection_size_type count(t.size());
		ar << BOOST_SERIALIZATION_NVP(count);
		for (auto it = t.begin(), end = t.end(); it != end; ++it) {
			ar << boost::serialization::make_nvp("item", (*it));
		}
	}

	template<typename Archive, typename Key, typename Compare, typename Allocator>
	void load(Archive & ar,
				boost::container::flat_set<Key, Compare, Allocator> &t,
				const unsigned int) {
		collection_size_type count;
		ar >> BOOST_SERIALIZATION_NVP(count);
		t.clear();
		t.reserve(count);
		while (count-- > 0) {
			Key item;
			ar >> boost::serialization::make_nvp("item", item);
			t.emplace(std::move(item));
		}
	}

	template<typename Archive, typename Key, typename Compare, typename Allocator>
	void serialize(Archive & ar,
				boost::container::flat_set<Key, Compare, Allocator> &t,
				const unsigned int file_version) {
		boost::serialization::split_free(ar, t, file_version);
	}

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
	void load(Archive& ar, boost::icl::discrete_interval<K>& di, unsigned) {
		auto bb = di.bounds().bits();
		K l, u;

		ar >> BOOST_SERIALIZATION_NVP(bb)
		   >> BOOST_SERIALIZATION_NVP(l)
		   >> BOOST_SERIALIZATION_NVP(u);

		di = boost::icl::discrete_interval<K>(l, u, boost::icl::interval_bounds(bb));
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
	void load(Archive& ar, boost::icl::interval_map<K, V>& im, unsigned) {
		im.clear();

		size_t sz;
		ar >> BOOST_SERIALIZATION_NVP(sz);

		for (auto c = sz; c > 0; c--) {
			boost::icl::discrete_interval<K> di;
			V v;
			ar >> boost::serialization::make_nvp("INTERVAL", di);
			ar >> boost::serialization::make_nvp("SET", v);

			im.insert(std::make_pair(di, v));
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

	oa << tree;
	wf.flush();
	wf.close();

	std::ifstream rf{"/tmp/interval.dat"};
	boost::archive::xml_iarchive ia(rf);
	ranges t2;
	ia >> t2;
}

int main() {
	ranges tree;

	City    cpur{"cpur", 28, 39};
	TempSet cset{cpur};
	auto    crange = interval<uint64_t>::right_open(28, 39);
	tree.add(std::make_pair(crange, cset));

	serialize(tree);
	return 0;
}