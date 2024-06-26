#!/usr/bin/env python3
from sdk import RandomPlayer
from sdk import Game
from peter import MyPlayer
from peter2 import MyPlayer2

import importlib
import datetime

if __name__ == '__main__':
    player1 = MyPlayer('X')
    player2 = MyPlayer('O')
    print('-----------------')
    game = Game(player1, player2)

    start = datetime.datetime.now()
    result = game.run()
    end = datetime.datetime.now()
    spent = (end - start).total_seconds()
    print(result["time_info"])
    resultStr = result["result"]
    print(f"{resultStr}, time spent = {spent} seconds")
