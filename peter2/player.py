import os
import subprocess
from subprocess import PIPE, STDOUT, Popen
import time

class EgaroucidBuilder:
    def __init__(self, color, path):
        self.path = path + "/Egaroucid-console"
        self.egaroucid_path = self.path + "/bin/Egaroucid_for_Console.out"
        self.egaroucid_eval_path = self.path + "/bin/resources/eval.egev"
        self.egaroucid_book_path = self.path + "/bin/resources/book.egbk2"

        if color == 'X':
            self.egaroucid_level = "21"
            self.mode = "1"
        else:
            self.egaroucid_level = "21"
            self.mode = "0"


class EgaroucidPlayer:
    def __init__(self, color):
        self.color = color

        builder = EgaroucidBuilder(color, os.path.dirname(os.path.realpath(__file__)))
        egaroucid_exec = builder.egaroucid_path \
            + " -q -eval " + builder.egaroucid_eval_path \
            + " -book " + builder.egaroucid_book_path \
            + " -level " + builder.egaroucid_level \
            + " -mode " + builder.mode
        print(self.color, egaroucid_exec)
        self.egaroucid = Popen(egaroucid_exec, shell=True, stdout=PIPE, stdin=PIPE, stderr=STDOUT)
        if color == 'X':
            self.step_index = 2
        else:
            self.step_index = 3
            self.__read_stdout()

    def __delete__(self):
        self.egaroucid.terminate()

    def __write_stdin(self, command):
        self.egaroucid.stdin.write(str.encode(command + "\n"))
        self.egaroucid.stdin.flush()

    def __read_stdout(self):
        out = b''
        while True:
            next_b = self.egaroucid.stdout.read(1)
            if next_b == b'>':
                break
            out += next_b
        #print(self.color, out)
        return out.decode("utf-8")

    def get_move(self, board):
        t1 = time.time()

        total_steps = len(board.move_history)
        i = self.step_index + 1
        while i < total_steps - 1:
            prev_node = board.move_history[i]
            print(i, self.color, '>>', prev_node['current_color'], prev_node['action'])
            self.__write_stdin(prev_node['action'])
            self.__read_stdout()
            i += 1

        if total_steps > 4:
            prev_node = board.move_history[-1]
            print(total_steps - 1, self.color, '>', prev_node['current_color'], prev_node['action'])
            if total_steps - 1 == self.step_index:
                self.__write_stdin("PS")
            else:
                self.__write_stdin("play " + prev_node['action'])

        #print(board.move_history)
        #board.display()

        action = self.__read_stdout().strip().upper()
        t2 = time.time()
        t = t2 - t1
        if t > 5:
            print(self.color, t)

        self.step_index = total_steps
        print(self.step_index, self.color, action)
        return action
