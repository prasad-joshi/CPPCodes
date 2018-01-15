#include <random>

#include <gtest/gtest.h>
#include <glog/logging.h>

#include "Hex.h"

using namespace Game;

TEST(HexTest, Neighbors) {
	Hex hex(1, 1);
	VLOG(1) << "Hex " << hex;
	hex.ForEachNeighbor([this] (const Hex& neighbor) {
		VLOG(1) << "Neighbor " << neighbor;
		return true;
	});
}

TEST(GameTest, RandomTest) {
	HexBoard board(7);

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, 7);

    auto player = Player::kPlayer1;
	while (not board.IsGameOver(player)) {
		auto c = distr(eng);
		auto r = distr(eng);

		Hex hex(c, r);
		if (not board.IsFree(hex)) {
			continue;
		}

		VLOG(1) << "Player " << PlayerToString(player) << " playing " << hex;
		board.PlayerPlayed(hex, player);
		player = NextPlayer(player);

		VLOG(1) << board;
		//::sleep(2);
	}

	if (not board.HasWinner()) {
		VLOG(1) << "Game Drawn";
	} else {
		VLOG(1) << "Winner " << PlayerToString(board.GetWinner());
	}
}

TEST(GameTest, NoWinner_Alternate) {
	HexBoard board(7);

	auto player = Player::kPlayer1;
	for (auto c = 0; c < 7; ++c) {
		for (auto r = 0; r < 7; ++r) {
			Hex hex(c, r);
			VLOG(1) << "Setting Hex " << hex;
			EXPECT_TRUE(board.IsFree(hex));
			board.PlayerPlayed(hex, player);

			VLOG(1) << board;
			if (board.HasWinner()) {
				break;
			}
			player = NextPlayer(player);
		}

		if (board.HasWinner()) {
			break;
		}
	}

	EXPECT_TRUE(board.HasWinner());
	EXPECT_TRUE(board.GetWinner() == Player::kPlayer1);
}

TEST(GameTest, Player2Winner) {
	const auto NROWS = 7;
	const auto player = Player::kPlayer2;

	for (auto r = 0; r < NROWS; ++r) {
		HexBoard board(NROWS);
		for (auto c = 0; c < NROWS; ++c) {
			Hex hex(c, r);
			EXPECT_TRUE(board.IsFree(hex));
			EXPECT_FALSE(board.IsGameOver(player));
			EXPECT_FALSE(board.HasWinner());
			board.PlayerPlayed(hex, player);
		}

		VLOG(2) << board;
		EXPECT_TRUE(board.HasWinner());
		EXPECT_EQ(board.GetWinner(), Player::kPlayer2);
	}
}

TEST(GameTest, Player1Winner) {
	const auto NROWS = 7;
	const auto player = Player::kPlayer1;

	for (auto c = 0; c < NROWS; ++c) {
		HexBoard board(NROWS);
		for (auto r = 0; r < NROWS; ++r) {
			Hex hex(c, r);
			EXPECT_TRUE(board.IsFree(hex));
			EXPECT_FALSE(board.IsGameOver(player));
			EXPECT_FALSE(board.HasWinner());
			board.PlayerPlayed(hex, player);
		}

		VLOG(2) << board;
		EXPECT_TRUE(board.HasWinner());
		EXPECT_EQ(board.GetWinner(), Player::kPlayer1);
	}
}