#include <gtest/gtest.h>
#include <glog/logging.h>

#include "Hex.h"

using namespace Game;

#if 0
TEST(HexTest, Conversion) {
	const auto NROWS = 13;
	for (auto row = 0; row < NROWS; ++row) {
		for (auto col = 0; col < NROWS; ++col) {
			OffsetCoord oc1(col, row);
			auto hex = oc1.ToHex();
			auto oc2 = hex.ToOffsetCoord();
			EXPECT_EQ(oc1, oc2);
			EXPECT_EQ(hex, oc2.ToHex());
		}
	}
}
#endif

#if 1
TEST(HexTest, Neighbors) {
	Hex hex(OffsetCoord(1, 1).ToHex());
	VLOG(1) << "Hex " << hex << " OffsetCoord " << hex.ToOffsetCoord();
	hex.ForEachNeighbor([this] (const Hex& neighbor) {
		VLOG(1) << "Neighbor " << neighbor << " OffsetCoord " << neighbor.ToOffsetCoord();
		return true;
	});
}
#endif

#if 0
TEST(GameTest, NoWinner_Alternate) {
	HexBoard board(7);

	auto player = Player::kPlayer1;
	for (auto c = 0; c < 7; ++c) {
		for (auto r = 0; r < 7; ++r) {
			if (player == Player::kPlayer1) {
				auto o = OffsetCoord(c, r);
				VLOG(1) << "Setting Hex " << o.ToHex() << " Offset " << o;
				EXPECT_TRUE(board.IsFree(OffsetCoord(c, r)));
				board.PlayerPlayed(OffsetCoord(c, r), player);

				VLOG(1) << board;
				assert(not board.IsGameOver(player));
				assert(not board.HasWinner());
			}

			player = NextPlayer(player);
		}
	}
}
#endif

#if 0
TEST(GameTest, Player2Winner) {
	const auto NROWS = 7;
	const auto player = Player::kPlayer2;

	for (auto r = 0; r < NROWS; ++r) {
		HexBoard board(NROWS);
		for (auto c = 0; c < NROWS; ++c) {
			EXPECT_TRUE(board.IsFree(OffsetCoord(c, r)));
			EXPECT_FALSE(board.IsGameOver(player));
			EXPECT_FALSE(board.HasWinner());
			board.PlayerPlayed(OffsetCoord(c, r), player);
		}

		EXPECT_TRUE(board.HasWinner());
		EXPECT_EQ(board.GetWinner(), Player::kPlayer2);
	}
}
#endif

#if 0
TEST(GameTest, Player1Winner) {
	const auto NROWS = 7;
	const auto player = Player::kPlayer1;

	for (auto c = 0; c < NROWS; ++c) {
		HexBoard board(NROWS);
		for (auto r = 0; r < NROWS; ++r) {
			EXPECT_TRUE(board.IsFree(OffsetCoord(c, r)));
			EXPECT_FALSE(board.IsGameOver(player));
			EXPECT_FALSE(board.HasWinner());
			board.PlayerPlayed(OffsetCoord(c, r), player);
		}

		VLOG(1) << board;
		EXPECT_TRUE(board.HasWinner());
		EXPECT_EQ(board.GetWinner(), Player::kPlayer1);
	}
}
#endif