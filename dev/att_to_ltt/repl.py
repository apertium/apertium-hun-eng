#!/usr/bin/python
# vim: set fileencoding=utf-8 :

import sys

EPSILON = u"ε".encode('utf-8')

def from_hfst(symbol):
    if symbol == "@0@":
        return EPSILON
    elif symbol == "@_SPACE_@":
        return " "
    else:
        return symbol

with open(sys.argv[1], 'r') as inf:
    repl = dict(tuple(line.strip().split()) for line in inf)

with open(sys.argv[2], 'r') as inf:
    for line in inf:
        fields = line.strip().split()
        fields[0] = repl[fields[0]]
        if len(fields) > 2:
            fields[1] = repl[fields[1]]
            fields[2] = from_hfst(fields[2])
            fields[3] = from_hfst(fields[3])
            fields[4] = ""
        else:
            fields[1] = ""
        print "\t".join(fields)
