#pragma once

#include <vector>
#include <cstdint>
#include <unordered_set>

namespace Game {

class OffsetCoord;
enum class Player : uint8_t {
	kFree,
	kPlayer1,
	kPlayer2,
};

std::string PlayerToString(Player player);
Player NextPlayer(Player player);

class Hex {
public:
	Hex(int16_t r, int16_t q, int16_t s);

	Hex operator + (const Hex& rhs) const;
	Hex operator - (const Hex& rhs) const;

	bool operator == (const Hex& rhs) const;
	bool operator != (const Hex& rhs) const;

	OffsetCoord ToOffsetCoord() const;

	void SetPlayer(Player player) const;
	Player GetPlayer() const;
	int16_t GetRow() const;
	int16_t GetCol() const;

	template <typename Lambda>
	void ForEachNeighbor(Lambda&& func) const {
		for (const auto d : kDirections) {
			auto h = *this + d;
			if (not func(h)) {
				break;
			}
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const Hex& o);;
public:
	static const std::vector<Hex> kDirections;
	static const std::vector<Hex> kDiagonals;
	static const int16_t kOdd = -1;
	static const int16_t kEven = 1;

public:
	friend class HexHash;
	friend class HexEqual;
private:
	const int16_t q_;
	const int16_t r_;
	const int16_t s_;
	mutable Player player_{Player::kFree};
};

class HexHash {
public:
	size_t operator() (const Hex& hex) const;
};

class HexEqual {
public:
	bool operator() (const Hex& lhs, Hex& rhs) const;
};

class OffsetCoord {
public:
	OffsetCoord(int16_t col, int16_t row);
	bool operator == (const OffsetCoord& rhs) const;
	bool operator != (const OffsetCoord& rhs) const;

	Hex ToHex() const;
	friend std::ostream& operator<<(std::ostream& os, const OffsetCoord& o);;
private:
	const int16_t col_;
	const int16_t row_;
};

class HexBoard {
public:
	HexBoard(uint16_t nrows);
	void Display() const;

	bool PlayerPlayed(const OffsetCoord& offset, Player player);
	bool IsFree(const OffsetCoord& offset) const;
	bool HasWinner() const;
	Player GetWinner() const;
	bool IsGameOver(Player current) const;

	friend std::ostream& operator<<(std::ostream& os, const HexBoard& dt);
private:
	bool IsPlayer1Winner(const Hex& hex, uint16_t matched,
		Player player = Player::kPlayer1) const;
	bool IsPlayer2Winner(const Hex& hex, uint16_t matched,
		Player player = Player::kPlayer2) const;
	bool HasValidMove() const;
	void SetWinner(Player player) const;
private:
	uint16_t nrows_;

	std::unordered_set<Hex, HexHash, HexEqual> board_;

	struct  {
		const uint16_t moves_possible_;
		uint16_t       moves_played_{0};
	};

	mutable Player winner_{Player::kFree};
};
}