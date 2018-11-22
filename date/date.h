#ifndef __DATE_H__
#define __DATE_H__

#include <array>
#include <cstdint>

template <typename T, std::size_t N>
constexpr uint32_t Accumulate(std::array<T, N> const& array) {
	uint32_t sum{};
	for (std::size_t i = 0; i < N; ++i) {
		sum += array[i];
	}
	return sum;
}

class Date {
public:
	enum Month {
		JAN,
		FEB,
		MARCH,
		APRIl,
		MAY,
		JUNE,
		JULY,
		AUGUST,
		SEPTEMBER,
		OCT,
		NOV,
		DEC,
	};
public:
	Date();
	Date(uint16_t year);
	Date(uint16_t year, Month month);
	Date(uint16_t year, Month month, uint8_t day);
	Date(const Date& date);
	int32_t operator - (const Date& date) const;
	bool operator < (const Date& date) const;
	bool operator > (const Date& date) const;
	bool operator == (const Date& rhs) const;
public:
	bool IsValid() const;
	bool CheckLeapYear(uint16_t year) const;
	bool IsLeapYear() const;
	uint32_t DaysInYear(uint16_t year) const;
	uint8_t DaysInMonth(Month month) const;
	uint16_t ElapsedDays() const;
	uint16_t RemainingDays() const;
private:
	uint16_t year_;
	Month    month_;
	uint8_t  day_;

public:
	static constexpr std::array<uint8_t, 12> kDaysInMonth = {
		31, /* JAN */
		28, /* FEB */
		31, /* March */
		30, /* April */
		31, /* May */
		30, /* june */
		31, /* july */
		31, /* aug */
		30, /* sept */
		31, // oct
		30, // nov
		31, // dec
	};

	static constexpr uint16_t kDaysInYear = Accumulate(kDaysInMonth);
};

#endif
