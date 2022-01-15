#!/usr/bin/env python3

from wave import open
from sys import argv
from array import array


if __name__ == "__main__":
    if len(argv) < 2:
        print("Pass in a wav file!")
        exit(1)
    with open(argv[1], 'r') as w:
        variable_name = argv[1].split("/")[-1].split(".")[0].upper()

        frames = array('h', w.readframes(-1))

        print("#ifndef", variable_name+"_HPP")
        print("#define", variable_name+"_HPP")
        print()
        print("#include <array>")
        print()
        print("const int", variable_name+"_LENGTH =", str(len(frames))+";")
        print()

        print("const std::array<float,"+str(len(frames))+"> ", end='')

        print(variable_name+"_SAMPLES = {{", end='')

        float_strings = map(lambda i: "{:10.6f}f".format(i / (2**15)), frames)
        print( ",".join(float_strings), end='' )

        print("}};")
        print("#endif")
