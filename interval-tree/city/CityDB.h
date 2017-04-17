#ifndef __CITY_DB_H__
#define __CITY_DB_H__

#include <map>
#include <vector>
#include <string>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp> 
#include <boost/serialization/string.hpp> 

#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/icl/interval_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/icl/split_interval_map.hpp>

#include "city.h"
#include "parser.h"
#include "Stack.h"

using namespace boost::icl;
using std::map;
using std::vector;
using std::string;

using CityPrimaryKey = uint64_t;
using CitySet    = boost::container::flat_set<CityPrimaryKey>;
using BoostIndex = boost::icl::interval_map<int64_t, CitySet>;

enum class IndexAttribute {
	TEMPERATURE,
	HUMIDITY,
};

class Index {
private:
	BoostIndex     impl_;
	IndexAttribute attr_;
public:
	Index(IndexAttribute attr) : attr_(attr) {

	}

	void addCity(const CityPrimaryKey key, int64_t low, int64_t high) {
		CitySet cset{key};
		auto crange = interval<int64_t>::right_open(low, high);
		impl_.add(std::make_pair(crange, cset));
	}

	const CitySet query(int64_t low, int64_t high) {
		auto    w = interval<int64_t>::right_open(low, high);
		auto    m = impl_ & w;
		CitySet rc;
		for (const auto &e : m) {
			rc += e.second;
		}
		return rc;
	}
#if 0
	void removeCity(const City &c) {
		uint64_t l, h;
		getRange(c, l, h);

		auto w  = interval<uint64_t>::right_open(l, h);
		auto lb = impl_.lower_bound(w);
		if (lb == impl_.end()) {
			return;
		}

		std::vector<discrete_interval<uint64_t>> empty;

		auto ub = impl_.upper_bound(w);
		for (auto it = lb; it != ub; it++) {
			for (auto sit = it->second.begin(); sit != it->second.end(); sit++) {
				if (sit->getName() == c.getName()) {
					it->second.erase(sit);
					break;
				}
			}
			if (it->second.size() == 0) {
				empty.push_back(it->first);
			}
		}

		for (const auto &e : empty) {
			impl_.erase(e);
		}
	}

	void display() {
		cout << impl_ << endl;
	}

    friend class boost::serialization::access; 
	void serialize(const string &filename) {
		std::ofstream wf{filename};
		boost::archive::xml_oarchive oa(wf);

		string s;
		switch (attr_) {
		case IndexAttribute::TEMPERATURE:
			s = "TEMPERATURE_INDEX";
			break;
		case IndexAttribute::HUMIDITY:
			s = "HUMIDITY_INDEX";
			break;
		}
		oa << boost::serialization::make_nvp(s.c_str(), impl_);
		wf.flush();
		wf.close();
	}
#endif
};

class CityDB {
private:
	std::map<string, City>           cityObjectDB_;
	std::map<CityPrimaryKey, string> cityDB_; 
	CityPrimaryKey                   key_;

	Index                  tempIndex_;
	Index                  humIndex_;

	int64_t                lowestTemp_{std::numeric_limits<int64_t>::max()};
	int64_t                highestTemp_{std::numeric_limits<int64_t>::min()};
	int64_t                lowestHum_{std::numeric_limits<int64_t>::max()};
	int64_t                highestHum_{std::numeric_limits<int64_t>::min()};

private:
	std::vector<string>    keywords_{"T", "H"};
	Stack<Token>           tokenStack_;
	Stack<CitySet>         resultStack_;
private:
	friend class boost::serialization::access;
	void serialize(const string &file) {
		std::ofstream wf{file};
		boost::archive::xml_oarchive oa(wf);

		oa << BOOST_SERIALIZATION_NVP(cityObjectDB_);

		wf.flush();
		wf.close();
	}

	void deserialize(const string &file) {
		std::ifstream rf{file};
		boost::archive::xml_iarchive ia(rf);

		ia >> BOOST_SERIALIZATION_NVP(cityObjectDB_);

		rf.close();

		/* start indices */
		assert(key_ == 0);
		for (const auto &e : cityObjectDB_) {
			const string &n{e.first};
			auto k = getPrimaryKey();
			cityDB_.insert(make_pair(k, n));
			indexCreate(k, e.second);
		}
	}

	void indexCreate( const CityPrimaryKey key, const City &c) {
		auto l = c.getLowTemperature();
		auto h = c.getHighTemperature();
		tempIndex_.addCity(key, l, h);
		if (lowestTemp_ > l) {
			lowestTemp_ = l;
		}
		if (highestTemp_ < h) {
			highestTemp_ = h;
		}

		l = c.getLowHumidity();
		h = c.getHighHumidity();
		humIndex_.addCity(key, l, h);
		if (lowestHum_ > l) {
			lowestHum_ = l;
		}
		if (highestHum_ < h) {
			highestHum_ = h;
		}
	}

	const CitySet query(const Token &k, const Token &op, const Token &v) {
		Index   *indexp;
		int64_t low;
		int64_t high;

		if (k.getValue() == "T") {
			indexp = &tempIndex_;
			low    = lowestTemp_;
			high   = highestTemp_;
		} else {
			indexp = &humIndex_;
			low    = lowestHum_;
			high   = highestHum_;
		}

		int64_t val = std::stoll(v.getValue());

		switch (op.tokenType()) {
		default:
			assert(0);
			break;
		case EQ:
			low  = val;
			high = val + 1;
			break;
		case NEQ:
			assert(0);
			break;
		case LT:
			high = val;
			break;
		case LE:
			high = val + 1;
			break;
		case GT:
			low = val;
			break;
		case GE:
			low = val + 1;
			break;
		}

		if (low > high) {
			/* return empty result */
			return CitySet{};
		}
		return indexp->query(low, high);
	}

	void queryOperator(const Token& op) {
		assert(op.isOperator());
		auto l = tokenStack_.pop();
		auto r = tokenStack_.pop();
		assert((l.isKeyWord() || r.isKeyWord()) &&
				(l.isOperand() || r.isOperand()));

		Token k;
		Token v;
		if (l.isKeyWord()) {
			k = l;
			v = r;
		} else {
			k = r;
			v = l;
		}

		switch (v.getValue()[0]) {
		case '-':
		case '+':
			assert(std::isdigit(v.getValue()[1]));
			break;
		default:
			assert(std::isdigit(v.getValue()[0]));
			break;
		}

		resultStack_.push(query(k, op, v));
	}

	void resultJoin(const Token &op) {
		assert(!resultStack_.isEmpty());
		auto r = resultStack_.pop();
		assert(!resultStack_.isEmpty());
		auto l = resultStack_.pop();

		switch (op.tokenType()) {
		defult:
			assert(0);
			break;
		case AND:
			resultStack_.push(l & r);
			break;
		case OR:
			resultStack_.push(l + r);
			break;
		}
		assert(!resultStack_.isEmpty());
	}

	CityPrimaryKey getPrimaryKey() {
		return ++key_;
	}

public:
	CityDB() :
		tempIndex_(IndexAttribute::TEMPERATURE),
		humIndex_(IndexAttribute::HUMIDITY), key_(0) {

	}

	bool addCity(const City &c) {
		const string &n{c.getName()};

		auto it = cityObjectDB_.find(n);
		if (it != cityObjectDB_.end()) {
			return false;
		}

		auto k = getPrimaryKey();

		cityObjectDB_.insert(make_pair(n, c));
		cityDB_.insert(make_pair(k, n));
		indexCreate(k, c);
	}

	std::vector<City> query(const string& query) {
		auto postfix = recursive_decent(query, keywords_);
		for (const auto &e : postfix) {
			if (e.isOperand()) {
				tokenStack_.push(e);
				continue;
			}

			switch (e.tokenType()) {
			default:
				e.dump();
				assert(0);
				break;
			case AND:
			case OR:
				resultJoin(e);
				break;
			case EQ:
			case NEQ:
			case LT:
			case LE:
			case GT:
			case GE:
				queryOperator(e);
				break;
			}
		}

		assert(resultStack_.size() == 1);
		std::vector<City> rc;
		for (const auto &e : resultStack_.pop()) {
			auto dbit = cityDB_.find(e);
			auto obit = cityObjectDB_.find(dbit->second);
			rc.push_back(obit->second);
		}
		assert(resultStack_.isEmpty());
		return rc;
	}

	void save(const string &file) {
		serialize(file);
	}

	void load(const string &file) {
		deserialize(file);
	}
};

#endif