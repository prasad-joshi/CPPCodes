#include <iostream>
#include <fstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
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

#include "city.h"

using std::string;
using std::make_pair;
using namespace boost::icl;
using std::cout;
using std::endl;


typedef boost::container::flat_set<City>  CitySet;
typedef boost::icl::interval_map<uint64_t, CitySet> Index;

void Query(const Index &index) {
	auto window  = interval<uint64_t>::right_open(15, 26);
	auto matched = index & window;

	cout << "Result " << endl;
	for (const auto &e : matched) {
		auto r = e.first;
		auto l = r.lower();
		auto u = r.upper();

		cout << l << " " << u << endl;
		for (const auto &t : e.second) {
			cout << t.to_string() << endl;
		}
	}
}

void display(const Index &index) {
	cout << index << endl;
}

void remove_city(Index &index, const City &city) {
	auto w = interval<uint64_t>::right_open(city.getLowTemperature(), city.getHighTemperature());

	auto l = index.lower_bound(w);
	if (l == index.end()) {
		return;
	}

	std::vector<discrete_interval<uint64_t>> empty;

	auto u = index.upper_bound(w);
	for (auto it = l; it != u; it++) {
		for (auto sit = it->second.begin(); sit != it->second.end(); sit++) {
			if (*sit == city) {
				it->second.erase(sit);
				break;
			}
		}
		if (it->second.size() == 0) {
			empty.push_back(it->first);
		}
	}

	for (const auto &e : empty) {
		index.erase(e);
	}
}

namespace boost { namespace serialization {
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

void serialize(const Index &index) {
	cout << "\n===== Current index : ======= \n";
	display(index);
	cout << "================================= \n";	

	std::ofstream wf{"/tmp/interval.dat"};
	boost::archive::text_oarchive oa(wf);

	oa << BOOST_SERIALIZATION_NVP(index);
	wf.flush();
	wf.close();

	cout << "\n===== Deserialized index : ======= \n";
	std::ifstream rf{"/tmp/interval.dat"};
	boost::archive::text_iarchive ia(rf);
	Index t2;
	ia >> BOOST_SERIALIZATION_NVP(t2);
	display(t2);
	cout << "================================= \n";
}

int main() {
	Index temp_index;

	City cpur{"cpur", 28, 39};
	CitySet cset{cpur};
	auto crange = interval<uint64_t>::right_open(cpur.getLowTemperature(), cpur.getHighTemperature());
	temp_index.add(std::make_pair(crange, cset));

	City pune{"pune", 10, 30};
	CitySet pset{pune};
	auto prange = interval<uint64_t>::right_open(pune.getLowTemperature(), pune.getHighTemperature());
	auto ppair  = std::make_pair(prange, pset);
	temp_index.add(ppair);

	City kpur{"kpur", 11, 31};
	CitySet kset{kpur};
	auto krange = interval<uint64_t>::right_open(kpur.getLowTemperature(), kpur.getHighTemperature());
	temp_index.add(std::make_pair(krange, kset));

	City npur{"npur", 25, 42};
	CitySet nset{npur};
	auto nrange = interval<uint64_t>::right_open(npur.getLowTemperature(), npur.getHighTemperature());
	temp_index.add(std::make_pair(nrange, nset));

	City xpur{"xpur", 35, 45};
	CitySet xset{xpur};
	auto xrange = interval<uint64_t>::right_open(xpur.getLowTemperature(), xpur.getHighTemperature());
	temp_index.add(std::make_pair(xrange, xset));

	City tpur{"tpur", 25, 51};
	CitySet tset{tpur};
	auto trange = interval<uint64_t>::right_open(tpur.getLowTemperature(), tpur.getHighTemperature());
	temp_index.add(std::make_pair(trange, tset));

	serialize(temp_index);

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
	return 0;
}
