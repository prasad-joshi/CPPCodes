#include <vector>
#include <algorithm>
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
	os << "HEX(row=" << hex.r_ << ", col=" << hex.q_ << ")";
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
	return q_ == rhs.q_ and r_ == rhs.r_;
}

bool Hex::operator != (const Hex& rhs) const {
	return not (*this == rhs);
}

bool Hex::operator < (const Hex& rhs) const {
	return q_ < rhs.q_ and r_ < rhs.r_;
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
	return lhs.q_ == rhs.q_ and lhs.r_ == rhs.r_;
}

HexBoard::HexBoard(uint16_t nrows) : nrows_(nrows), moves_possible_(nrows * nrows) {
	for (auto row = 0u; row < nrows; ++row) {
		for (auto col = 0u; col < nrows; ++col) {
			board_.emplace(Hex(col, row));
		}
	}
}

void HexBoard::Display(std::ostream& os) const {
	os << "Board Size " << nrows_ << " X " << nrows_ << std::endl;
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
		os << print << std::endl;
	}
}

void HexBoard::Display() const {
	Display(std::cout);
}

std::ostream& operator<<(std::ostream& os, const HexBoard& board) {
	board.Display(os);
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

bool HexBoard::IsPlayerWinner(const Hex& current, Player player) const {
	assert(player != Player::kFree && not HasWinner());

	current.ForEachNeighbor([&] (const Hex& neighbor) mutable {
#if 1
		auto eit = this->evaluated_.find(neighbor);
		if (eit != this->evaluated_.end()) {
			return true;
		}
#else
		auto vit = std::find(winner_path_.begin(), winner_path_.end(), neighbor);
		if (vit != winner_path_.end()) {
			return true;
		}
#endif

		auto it = this->board_.find(neighbor);
		if (it == this->board_.end() or it->GetPlayer() != player) {
			return true;
		}
		this->winner_path_.emplace_back(*it);
		this->evaluated_.emplace(*it);

		switch (player) {
		default:
			assert(0);
			return false;
		case Player::kPlayer1:
			if (it->GetRow() >= this->nrows_ - 1) {
				this->SetWinner(player);
				return false;
			}
			break;
		case Player::kPlayer2:
			if (it->GetCol() >= this->nrows_ - 1) {
				this->SetWinner(player);
				return false;
			}
			break;
		}

		this->IsPlayerWinner(*it, player);
		if (not this->HasWinner()) {
			this->winner_path_.pop_back();
			return true;
		}
		return false;
	});
	return HasWinner() == true and GetWinner() == player;
}

std::vector<Hex> HexBoard::GetWinnerPath() const {
	auto path = winner_path_;
	std::sort(path.begin(), path.end(), [] (const Hex& lhs, const Hex& rhs) {
		return lhs < rhs;
	});
	return path;
}

bool HexBoard::IsGameOver(Player current) const {
	assert(current != Player::kFree);

	if (HasWinner()) {
		return true;
	}

	assert(winner_path_.empty());

	int16_t row = 0;
	int16_t col = 0;
	for (auto c = 0; c < nrows_; ++c) {
		evaluated_.clear();
		switch (current) {
		default:
			assert(0);
			return false;
		case Player::kPlayer1:
			if (c > 0) {
				++col;
			}
			break;
		case Player::kPlayer2:
			if (c > 0) {
				++row;
			}
		}

		assert(winner_path_.empty());

		auto it = board_.find(Hex(col, row));
		assert(it != board_.end());
		if (it->GetPlayer() != current) {
			continue;
		}

		winner_path_.emplace_back(*it);
		evaluated_.emplace(*it);
		IsPlayerWinner(*it, current);

		if (HasWinner()) {
			break;
		}
		assert(winner_path_.size() == 1);
		winner_path_.pop_back();
	}

	evaluated_.clear();
	if (HasWinner()) {
		assert(not winner_path_.empty());
		return true;
	}
	assert(winner_path_.empty());

	return HasValidMove();
}

}