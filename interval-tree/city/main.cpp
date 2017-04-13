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

typedef boost::container::flat_set<City>  TempSet;
typedef boost::icl::interval_map<uint64_t, TempSet> ranges;

void Query(const ranges &tree) {
	auto window  = interval<uint64_t>::right_open(15, 26);
	auto matched = tree & window;

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

void display(const ranges &tree) {
	cout << tree << endl;
}

void remove_city(ranges &tree, const City &city) {
	auto w = interval<uint64_t>::right_open(city.getLowTemperature(), city.getHighTemperature());

	auto l = tree.lower_bound(w);
	if (l == tree.end()) {
		return;
	}

	std::vector<discrete_interval<uint64_t>> empty;

	auto u = tree.upper_bound(w);
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
		tree.erase(e);
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

void serialize(const ranges &tree) {
	cout << "\n===== Current tree : ======= \n";
	display(tree);
	cout << "================================= \n";	

	std::ofstream wf{"/tmp/interval.dat"};
	boost::archive::xml_oarchive oa(wf);

	oa << tree;
	wf.flush();
	wf.close();

	cout << "\n===== Deserialized tree : ======= \n";
	std::ifstream rf{"/tmp/interval.dat"};
	boost::archive::xml_iarchive ia(rf);
	ranges t2;
	ia >> t2;
	display(t2);
	cout << "================================= \n";
}

int main() {
	ranges tree;

	City cpur{"cpur", 28, 39};
	TempSet cset{cpur};
	auto crange = interval<uint64_t>::right_open(cpur.getLowTemperature(), cpur.getHighTemperature());
	tree.add(std::make_pair(crange, cset));

	City pune{"pune", 10, 30};
	TempSet pset{pune};
	auto prange = interval<uint64_t>::right_open(pune.getLowTemperature(), pune.getHighTemperature());
	auto ppair  = std::make_pair(prange, pset);
	tree.add(ppair);

	City kpur{"kpur", 11, 31};
	TempSet kset{kpur};
	auto krange = interval<uint64_t>::right_open(kpur.getLowTemperature(), kpur.getHighTemperature());
	tree.add(std::make_pair(krange, kset));

	City npur{"npur", 25, 42};
	TempSet nset{npur};
	auto nrange = interval<uint64_t>::right_open(npur.getLowTemperature(), npur.getHighTemperature());
	tree.add(std::make_pair(nrange, nset));

	City xpur{"xpur", 35, 45};
	TempSet xset{xpur};
	auto xrange = interval<uint64_t>::right_open(xpur.getLowTemperature(), xpur.getHighTemperature());
	tree.add(std::make_pair(xrange, xset));

	City tpur{"tpur", 25, 51};
	TempSet tset{tpur};
	auto trange = interval<uint64_t>::right_open(tpur.getLowTemperature(), tpur.getHighTemperature());
	tree.add(std::make_pair(trange, tset));

	serialize(tree);

	cout << "Before removal.\n";
	display(tree);

	remove_city(tree, tpur);
	cout << "After removal tpur " << endl;
	display(tree);

	remove_city(tree, xpur);
	cout << "After removal xpur " << endl;
	display(tree);

	tree.add(std::make_pair(xrange, xset));
	cout << "After adding xpur " << endl;
	display(tree);

	tree.add(std::make_pair(trange, tset));
	cout << "After adding tpur " << endl;
	display(tree);

	remove_city(tree, xpur);
	cout << "After removal xpur " << endl;
	display(tree);

	remove_city(tree, tpur);
	cout << "After removal tpur " << endl;
	display(tree);

	serialize(tree);
	return 0;
}
