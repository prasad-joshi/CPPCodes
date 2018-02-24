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
	auto const NROWS = 101;
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(0, NROWS);

	for (auto i = 0; i < 5; ++i) {
		HexBoard board(NROWS);
		auto player = Player::kPlayer1;
		while (not board.IsGameOver(player)) {
			auto c = distr(eng);
			auto r = distr(eng);

			Hex hex(c, r);
			if (not board.IsFree(hex)) {
				continue;
			}

			// VLOG(1) << "Player " << PlayerToString(player) << " playing " << hex;
			board.PlayerPlayed(hex, player);
			player = NextPlayer(player);

			//VLOG(1) << board;
			//::sleep(2);
		}

		if (not board.HasWinner()) {
			VLOG(1) << board;
			assert(0);
		}

		/* make sure each hex contributes to path only once */
		std::unordered_set<Hex, HexHash, HexEqual> hex_set;
		for (const auto hex : board.GetWinnerPath()) {
			auto sit = hex_set.find(hex);
			EXPECT_EQ(sit, hex_set.end());
			hex_set.emplace(hex);
		}

		EXPECT_TRUE(board.HasWinner());
		VLOG(1) << "Test " << i << " Winner " << PlayerToString(board.GetWinner());
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
	/* make sure each hex contributes to path only once */
	std::unordered_set<Hex, HexHash, HexEqual> hex_set;
	for (const auto hex : board.GetWinnerPath()) {
		auto sit = hex_set.find(hex);
		EXPECT_EQ(sit, hex_set.end());
		hex_set.emplace(hex);
	}
}

TEST(GameTest, Player2Winner_Test1) {
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

		for (const auto hex : board.GetWinnerPath()) {
			EXPECT_EQ(hex.GetRow(), r);
		}

		/* make sure each hex contributes to path only once */
		std::unordered_set<Hex, HexHash, HexEqual> hex_set;
		for (const auto hex : board.GetWinnerPath()) {
			auto sit = hex_set.find(hex);
			EXPECT_EQ(sit, hex_set.end());
			hex_set.emplace(hex);
		}
	}
}

TEST(GameTest, Player1Winner_Test1) {
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
		assert(board.IsGameOver(player));
		EXPECT_TRUE(board.HasWinner());
		EXPECT_EQ(board.GetWinner(), player);

		for (const auto hex : board.GetWinnerPath()) {
			EXPECT_EQ(hex.GetCol(), c);
		}

		/* make sure each hex contributes to path only once */
		std::unordered_set<Hex, HexHash, HexEqual> hex_set;
		for (const auto hex : board.GetWinnerPath()) {
			auto sit = hex_set.find(hex);
			EXPECT_EQ(sit, hex_set.end());
			hex_set.emplace(hex);
		}
	}
}

TEST(GameTest, Player1Winner_Test2) {
	const auto NROWS = 5;
	const auto player = Player::kPlayer1;

	std::vector<Hex> moves;
	moves.emplace_back(0, 0);
	moves.emplace_back(0, 1);
	moves.emplace_back(1, 1);
	moves.emplace_back(1, 2);
	moves.emplace_back(0, 3);
	moves.emplace_back(4, 0);
	moves.emplace_back(2, 2);
	moves.emplace_back(2, 3);
	moves.emplace_back(3, 4);

	HexBoard board(NROWS);
	for (const auto hex : moves) {
		EXPECT_TRUE(board.IsFree(hex));
		EXPECT_FALSE(board.IsGameOver(player));
		EXPECT_FALSE(board.HasWinner());
		board.PlayerPlayed(hex, player);
		VLOG(1) << board;
	}
	Hex hex(2, 4);
	board.PlayerPlayed(hex, player);
	VLOG(1) << board;

	assert(board.IsGameOver(player));
	EXPECT_TRUE(board.IsGameOver(player));
	EXPECT_TRUE(board.HasWinner());
	EXPECT_EQ(board.GetWinner(), Player::kPlayer1);

	moves.emplace_back(hex);
	std::unordered_set<Hex, HexHash, HexEqual> hex_set;
	for (const auto hex : board.GetWinnerPath()) {
		auto it = std::find(moves.begin(), moves.end(), hex);
		EXPECT_NE(it, moves.end());

		auto sit = hex_set.find(hex);
		EXPECT_EQ(sit, hex_set.end());
		hex_set.emplace(hex);
	}
	/* few moves may not contribute to path */
	EXPECT_GE(moves.size(), hex_set.size());
	EXPECT_GE(hex_set.size(), NROWS);
}

TEST(GameTest, Player2Winner_Test2) {
	const auto NROWS = 7;
	const auto player = Player::kPlayer2;

	std::vector<Hex> moves;
	moves.emplace_back(0, 0);
	moves.emplace_back(1, 0);
	moves.emplace_back(2, 0);
	moves.emplace_back(3, 0);
	moves.emplace_back(4, 0);
	moves.emplace_back(5, 0);
	moves.emplace_back(4, 1);
	moves.emplace_back(3, 2);
	moves.emplace_back(3, 3);
	moves.emplace_back(4, 3);
	moves.emplace_back(5, 2);
	// moves.emplace_back(6, 2);

	HexBoard board(NROWS);
	for (const auto hex : moves) {
		EXPECT_TRUE(board.IsFree(hex));
		EXPECT_FALSE(board.IsGameOver(player));
		EXPECT_FALSE(board.HasWinner());
		board.PlayerPlayed(hex, player);
		VLOG(1) << board;
	}
	Hex hex(6, 2);
	board.PlayerPlayed(hex, player);
	VLOG(1) << board;

	assert(board.IsGameOver(player));
	EXPECT_TRUE(board.IsGameOver(player));
	EXPECT_TRUE(board.HasWinner());
	EXPECT_EQ(board.GetWinner(), player);

	moves.emplace_back(hex);
	std::unordered_set<Hex, HexHash, HexEqual> hex_set;
	for (const auto hex : board.GetWinnerPath()) {
		auto it = std::find(moves.begin(), moves.end(), hex);
		EXPECT_NE(it, moves.end());

		auto sit = hex_set.find(hex);
		EXPECT_EQ(sit, hex_set.end());
		hex_set.emplace(hex);
	}
	/* few moves may not contribute to path */
	EXPECT_GE(moves.size(), hex_set.size());
	EXPECT_GE(hex_set.size(), NROWS);
}