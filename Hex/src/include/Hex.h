#pragma once

#include <vector>
#include <cstdint>
#include <unordered_set>

namespace Game {

enum class Player : uint8_t {
	kFree,
	kPlayer1,
	kPlayer2,
};

std::string PlayerToString(Player player);
Player NextPlayer(Player player);

class Hex {
public:
	Hex(int16_t r, int16_t q);

	Hex operator + (const Hex& rhs) const;
	Hex operator - (const Hex& rhs) const;

	bool operator == (const Hex& rhs) const;
	bool operator != (const Hex& rhs) const;

	bool operator < (const Hex& rhs) const;

	//OffsetCoord ToOffsetCoord() const;

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
	int16_t q_;
	int16_t r_;
	int16_t s_;
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

class HexBoard {
public:
	HexBoard(uint16_t nrows);
	void Display(std::ostream& os) const;
	void Display() const;

	bool PlayerPlayed(const Hex& hex, Player player);
	bool IsFree(const Hex& hex) const;
	bool HasWinner() const;
	Player GetWinner() const;
	bool IsGameOver(Player current) const;
	std::vector<Hex> GetWinnerPath() const;

	friend std::ostream& operator<<(std::ostream& os, const HexBoard& dt);
private:
	bool IsPlayerWinner(const Hex& current, Player player) const;
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
	mutable std::vector<Hex> winner_path_;
	mutable std::unordered_set<Hex, HexHash, HexEqual> evaluated_;
};
}