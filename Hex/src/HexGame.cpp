#include <iostream>
#include <cassert>

#include "Hex.h"

using namespace Game;
int main() {
	auto quit = false;
	while (not quit) {
		HexBoard board(7);
		Player player = Player::kPlayer1;
		while (not board.IsGameOver(player) and not quit) {
			std::cout << board << std::endl;
			std::cout << "Current player " << PlayerToString(player) << std::endl;
			if (player == Player::kPlayer1) {
				std::cout << "Chain " << PlayerToString(player)
					<< " from top to bottom " << std::endl;
			} else {
				std::cout << "Chain " << PlayerToString(player)
					<< " from left to right " << std::endl;
			}

			int16_t row, col;
			bool first = true;
			do {
				if (not first) {
					std::cout << "(" << row << "," << col << ") is not free"
						<< std::endl;
				}
				std::cout << "Enter row and col [(-1 -1) to quit] : ";
				std::cin >> row >> col;
				first = false;
				if (row == -1) {
					quit = true;
				}
			} while (not board.IsFree(Hex(col, row)) and not quit);

			if (not quit) {
				board.PlayerPlayed(Hex(col, row), player);
				player = NextPlayer(player);
			}
		}

		if (not quit) {
			if (not board.HasWinner()) {
				std::cout << "Game drawn" << std::endl;
			} else {
				std::cout << PlayerToString(board.GetWinner())
					<< " Won!!!" << std::endl;
			}

			std::string input;
			std::cout << "Quit [y/n]? ";
			std::cin >> input;

			if (input.front() == 'y') {
				quit = true;
			}
		}
	}
	return 0;
}