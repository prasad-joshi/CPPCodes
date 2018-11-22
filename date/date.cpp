#include <numeric>
#include <iostream>

#include "date.h"

using std::cout;
using std::endl;

Date::Date(uint16_t year, Date::Month month, uint8_t day) : year_(year),
		month_(month), day_(day) {
	if (not IsValid()) {
		/* TODO throw exception */
	}
}

Date::Date(uint16_t year, Date::Month month) : Date(year, month, 1) {

}

Date::Date(uint16_t year) : Date(year, Date::JAN, 1) {

}

Date::Date() : Date(1980, Date::Month::JAN, 1) {

}

Date::Date(const Date& rhs) {
	year_  = rhs.year_;
	month_ = rhs.month_;
	day_   = rhs.day_;
}

bool Date::IsValid() const {
	return true;
}

bool Date::CheckLeapYear(uint16_t year) const {
	if (year % 400 == 0) {
		return true;
	} else if (year % 100 == 0) {
		return false;
	} else if (year % 4 == 0) {
		return true;
	}
	return false;
}

bool Date::IsLeapYear() const {
	return CheckLeapYear(year_);
}

uint32_t Date::DaysInYear(uint16_t year) const {
	return Date::kDaysInYear + CheckLeapYear(year);
}

uint8_t Date::DaysInMonth(Date::Month month) const {
	return Date::kDaysInMonth[month] +
			(month == Date::Month::FEB ? IsLeapYear() : 0);
}

uint16_t Date::ElapsedDays() const {
	uint16_t days = 0;
	for (int m = Date::Month::JAN; m < month_; ++m) {
		days += DaysInMonth((Date::Month) m);
	}
	return days + day_;
}

uint16_t Date::RemainingDays() const {
	return DaysInYear(year_) - ElapsedDays();
}

bool Date::operator < (const Date& rhs) const {
	if (year_ < rhs.year_) {
		return true;
	} else if (month_ < rhs.month_) {
		return true;
	} else if (day_ < rhs.day_) {
		return true;
	}
	return false;
}

bool Date::operator > (const Date& rhs) const {
	if (year_ > rhs.year_) {
		return true;
	} else if (month_ > rhs.month_) {
		return true;
	} else if (day_ > rhs.day_) {
		return true;
	}
	return false;
}

bool Date::operator == (const Date& rhs) const {
	if (year_ == rhs.year_ && month_ == rhs.month_ && day_ == rhs.day_) {
		return true;
	}
	return false;
}

int32_t Date::operator - (const Date& rhs) const {
	if (*this == rhs) {
		return 0;
	}

	if (year_ == rhs.year_) {
		return ElapsedDays() - rhs.ElapsedDays();
	}

	if (*this > rhs) {
		return -(rhs - *this);
	}

	int32_t d = 0;
	for (auto y = year_+1; y < rhs.year_; ++y) {
		d += DaysInYear(y);
	}
	return -(d + RemainingDays() + rhs.ElapsedDays());
}

int main() {
	Date d1(2020, Date::Month::MARCH, 2);
	Date d2(2020, Date::Month::MARCH, 2);
	Date d3(2020, Date::Month::MARCH, 1);
	std::cout << d1.ElapsedDays() << std::endl;
	std::cout << "d1 - d2 " << d1 - d2 << std::endl;
	std::cout << "d2 - d1 " << d2 - d1 << std::endl;
	std::cout << "d1 - d3 " << d1 - d3 << std::endl;
	std::cout << "d3 - d1 " << d3 - d1 << std::endl;

	Date d4(2019, Date::Month::JAN, 1);
	Date d5(2020, Date::Month::JAN, 1);
	std::cout << "d4 - d5 " << d4 - d5 << std::endl;
	std::cout << "d5 - d4 " << d5 - d4 << std::endl;

	std::cout << "kDaysInYear " << Date::kDaysInYear << std::endl;
	std::cout << "d4.RemainingDays " << d4.RemainingDays() << std::endl;
	std::cout << "d4.ElapsedDays " << d4.ElapsedDays() << std::endl;
	std::cout << "d5.RemainingDays " << d5.RemainingDays() << std::endl;
	std::cout << "d5.ElapsedDays " << d5.ElapsedDays() << std::endl;

	Date d6(2020, Date::Month::MARCH, 1);
	std::cout << "d4 - d6 " << d4 - d6 << std::endl;
	std::cout << "d6 - d4 " << d6 - d4 << std::endl;
	return 0;
}
