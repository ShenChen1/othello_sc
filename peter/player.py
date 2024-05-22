import os
import subprocess
from subprocess import PIPE, STDOUT, Popen
import time
import zipfile

def unzip_file(zip_path, extract_path):
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(extract_path)

class EdaxBuilder:
    def __init__(self, color, path):
        self.path = path + "/edax-reversi"
        self.edax_path = self.path + "/bin/lEdax-x64-modern"
        self.edax_eval_path = self.path + "/data/eval.dat"
        self.edax_book_path = self.path + "/data/edax_book.dat"

        self.edax_peter_book_path = self.path + "/data/peter_book.dat"
        if os.path.exists(self.edax_peter_book_path):
            self.edax_book_path = self.path + "/data/mybook.dat"
            unzip_file(self.edax_peter_book_path, os.path.dirname(self.edax_book_path))

        if color == 'X':
            self.edax_level = "32"
            self.mode = "1"
        else:
            self.edax_level = "32"
            self.mode = "0"

    def build(self):
        if os.path.exists(self.edax_path):
            return

        current_dir = os.getcwd()
        os.chdir(self.path + "/src")
        ret = subprocess.call("make clean build ARCH=x64-modern COMP=gcc OS=linux >/dev/null 2>&1", shell=True)
        os.chdir(current_dir)

        if ret:
            raise Exception("Failed to build edax")

class EdaxPlayer:
    def __init__(self, color):
        self.color = color

        builder = EdaxBuilder(color, os.path.dirname(os.path.realpath(__file__)))
        builder.build()

        edax_exec = builder.edax_path \
            + " -q -eval-file " + builder.edax_eval_path \
            + " -book-file " + builder.edax_book_path \
            + " -level " + builder.edax_level \
            + " -noise " + builder.edax_level \
            + " -mode " + builder.mode \
            + " -move-time 5.9 " \
            + " -ponder on "
        print(self.color, edax_exec)
        self.edax = Popen(edax_exec, shell=True, stdout=PIPE, stdin=PIPE, stderr=STDOUT)
        self.step_index = 2 if color == 'X' else 3
        time.sleep(5)

    def __delete__(self):
        self.edax.terminate()

    def __write_stdin(self, command):
        self.edax.stdin.write(str.encode(command + "\n"))
        self.edax.stdin.flush()

    def __read_stdout(self):
        out = b''
        while True:
            next_b = self.edax.stdout.read(1)
            if next_b == b'>':
                if (len(out) > 0 and out[-1] == ord('\n')):
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
            #print(i, self.color, '>>', prev_node['current_color'], prev_node['action'])
            self.__write_stdin(prev_node['action'])
            self.__read_stdout()
            i += 1

        if total_steps > 4:
            prev_node = board.move_history[-1]
            #print(total_steps - 1, self.color, '>', prev_node['current_color'], prev_node['action'])
            if total_steps - 1 == self.step_index:
                self.__write_stdin("PS")
            else:
                self.__write_stdin(prev_node['action'])

        #print(board.move_history)
        #board.display()

        action = self.__read_stdout().split("Edax plays ")[-1][:2]
        t2 = time.time()
        t = t2 - t1
        if t > 5:
            print(self.color, t)

        self.step_index = total_steps
        #print(self.step_index, self.color, action)
        return action
