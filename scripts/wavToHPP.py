#!/usr/bin/env python3

from wave import open
from sys import argv
from array import array


if __name__ == "__main__":
    if len(argv) < 2:
        print("Pass in a wav file!")
        exit(1)
    with open(argv[1], 'r') as w:
        frames = array('h', w.readframes(-1))

        print("const array<float,"+str(len(frames))+"> ", end='')

        variable_name = argv[1].split("/")[-1].split(".")[0]
        print(variable_name, end='')

        print(" = {{")

        float_strings = map(lambda i: "{:10.6f}f".format(i / (2**15)), frames)
        print( ",".join(float_strings), end='' )

        print("}};")
