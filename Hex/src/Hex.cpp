#include <vector>
#include <cstdint>
#include <iostream>

#include <cassert>

#include "Hex.h"

namespace Game {

const std::vector<Hex> Hex::kDirections = {
	Hex(1, 0),
	Hex(1, -1),
	Hex(0, -1),
	Hex(-1, 0),
	Hex(-1, 1),
	Hex(0, 1)
};

const std::vector<Hex> Hex::kDiagonals = {
	Hex(2, -1),
	Hex(1, -2),
	Hex(-1, -1),
	Hex(-2, 1),
	Hex(-1, 2),
	Hex(1, 1)
};

std::string PlayerToString(Player player) {
	switch (player) {
	default:
		return ".";
	case Player::kPlayer1:
		return "X";
	case Player::kPlayer2:
		return "O";
	}
}

Player NextPlayer(Player player) {
	assert(player != Player::kFree);
	if (player == Player::kPlayer1) {
		return Player::kPlayer2;
	}
	return Player::kPlayer1;
}

std::ostream& operator<<(std::ostream& os, const Hex& hex) {
	os << "HEX q " << hex.q_ << " r " << hex.r_ << " s " << hex.s_;
	return os;
}

Hex::Hex(int16_t q, int16_t r) : q_(q), r_(r), s_(-q - r) {

}

Hex Hex::operator + (const Hex& rhs) const {
	return Hex(q_ + rhs.q_, r_ + rhs.r_);
}

Hex Hex::operator - (const Hex& rhs) const {
	return Hex(q_ - rhs.q_, r_ - rhs.r_);
}

bool Hex::operator == (const Hex& rhs) const {
	return q_ == rhs.q_ and r_ == rhs.r_ and s_ == rhs.s_;
}

bool Hex::operator != (const Hex& rhs) const {
	return not (*this == rhs);
}

int16_t Hex::GetRow() const {
	return r_;
}

int16_t Hex::GetCol() const {
	return q_;
}

void Hex::SetPlayer(Player player) const {
	assert(player_ == Player::kFree);
	player_ = player;
}

Player Hex::GetPlayer() const {
	return player_;
}

size_t HexHash::operator() (const Hex& hex) const {
	std::hash<int> int_hash;
	size_t hq = int_hash(hex.q_);
	size_t hr = int_hash(hex.r_);
	return hq ^ (hr + 0x9e3779b9 + (hq << 6) + (hq >> 2));
}

bool HexEqual::operator() (const Hex& lhs, Hex& rhs) const {
	return lhs.q_ == rhs.q_ and lhs.r_ == rhs.r_ and lhs.s_ == rhs.s_;
}

#if 0
OffsetCoord Hex::ToOffsetCoord() const {
	int16_t col = q_ + int16_t((r_ + Hex::kOdd * (r_ & 1)) / 2);
	int16_t row = r_;
	return OffsetCoord(col, row);
}

std::ostream& operator<<(std::ostream& os, const OffsetCoord& offset) {
	os << "OffsetCoord col " << offset.col_ << " row " << offset.row_;
	return os;
}

OffsetCoord::OffsetCoord(int16_t col, int16_t row) : col_(col), row_(row) {

}

bool OffsetCoord::operator == (const OffsetCoord& rhs) const {
	return row_ == rhs.row_ and col_ == rhs.col_;
}

bool OffsetCoord::operator != (const OffsetCoord& rhs) const {
	return not (*this == rhs);
}


Hex OffsetCoord::ToHex() const {
	int q = col_ - int16_t((row_ + Hex::kOdd * (row_ & 1)) / 2);
	int r = row_;
	int s = -q - r;
	return Hex(q, r, s);
}
#endif

HexBoard::HexBoard(uint16_t nrows) : nrows_(nrows), moves_possible_(nrows * nrows) {
	for (auto row = 0u; row < nrows; ++row) {
		for (auto col = 0u; col < nrows; ++col) {
			board_.emplace(Hex(col, row));
		}
	}
}

void HexBoard::Display() const {
	std::string print;
	for (auto row = 0; row < nrows_; ++row) {
		print.clear();
		print.append(std::string(row * 2, ' '));
		for (auto col = 0; col < nrows_; ++col) {
			auto it = board_.find(Hex(col, row));
			assert(it != board_.end());


			auto player = col == 0 ? PlayerToString(it->GetPlayer()) :
				std::string(" - ") + PlayerToString(it->GetPlayer());

			print.append(player);
		}
		print.append(1, '\n');

		if (row != nrows_ - 1) {
			print.append(std::string(row * 2 + 1, ' '));
			print.append(1, '\\');
			print.append(1, ' ');

			std::string tmp("/ \\ ");
			for (auto i = 0; i < nrows_ - 1; ++i) {
				print.append(tmp);
			}
		}
		std::cout << print << std::endl;
	}
}

std::ostream& operator<<(std::ostream& os, const HexBoard& board) {
	board.Display();
	return os;
}

bool HexBoard::PlayerPlayed(const Hex& hex, Player player) {
	if (not IsFree(hex)) {
		return false;
	}

	auto it = board_.find(hex);
	assert(it != board_.end() and it->GetPlayer() == Player::kFree);
	it->SetPlayer(player);
	++moves_played_;
	IsGameOver(player);
	return true;
}

bool HexBoard::HasValidMove() const {
	return moves_played_ == moves_possible_;
}

bool HexBoard::IsFree(const Hex& hex) const {
	auto it = board_.find(hex);
	return it != board_.end() and it->GetPlayer() == Player::kFree;
}

bool HexBoard::HasWinner() const {
	return winner_ != Player::kFree;
}

Player HexBoard::GetWinner() const {
	return winner_;
}

void HexBoard::SetWinner(Player player) const {
	assert(not HasWinner());
	winner_ = player;
}

bool HexBoard::IsPlayer1Winner(const Hex& hex, uint16_t matched, Player player) const {
	assert(player == Player::kPlayer1 && not HasWinner());

	hex.ForEachNeighbor([&] (const Hex& neighbor) mutable {
		if (neighbor.GetRow() <= hex.GetRow()) {
			return true;
		}

		auto it = this->board_.find(neighbor);
		if (it == this->board_.end() or it->GetPlayer() != player) {
			return true;
		}

		if (matched >= this->nrows_ - 1) {
			this->SetWinner(player);
			return false;
		}

		this->IsPlayer1Winner(neighbor, matched + 1, player);
		return not this->HasWinner();
	});
	return HasWinner() == true and GetWinner() == player;
}

bool HexBoard::IsPlayer2Winner(const Hex& hex, uint16_t matched, Player player) const {
	assert(player == Player::kPlayer2 && not HasWinner());

	hex.ForEachNeighbor([&] (const Hex& neighbor) mutable {
		if (neighbor.GetCol() <= hex.GetCol()) {
			return true;
		}

		auto it = this->board_.find(neighbor);
		if (it == this->board_.end() or it->GetPlayer() != player) {
			return true;
		}

		if (matched >= this->nrows_ - 1) {
			this->SetWinner(player);
			return false;
		}

		this->IsPlayer2Winner(neighbor, matched + 1, player);
		return not this->HasWinner();
	});
	return HasWinner() == true and GetWinner() == player;
}

bool HexBoard::IsGameOver(Player current) const {
	assert(current != Player::kFree);

	if (HasWinner()) {
		return true;
	}

	switch (current) {
	default:
		assert(0);
		return false;
	case Player::kPlayer1: {
		for (auto col = 0; col < nrows_; ++col) {
			if (HasWinner()) {
				break;
			}

			auto it = board_.find(Hex(col, 0));
			assert(it != board_.end());
			if (it->GetPlayer() != Player::kPlayer1) {
				continue;
			}

			IsPlayer1Winner(*it, 1);
		}
		break;
	}
	case Player::kPlayer2: {
		for (auto row = 0; row < nrows_; ++row) {
			if (HasWinner()) {
				break;
			}
			auto it = board_.find(Hex(0, row));
			assert(it != board_.end());
			if (it->GetPlayer() != Player::kPlayer2) {
				continue;
			}

			IsPlayer2Winner(*it, 1);
		}
		break;
	}
	}

	if (HasWinner()) {
		return true;
	}

	return HasValidMove();
}

}