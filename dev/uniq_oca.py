"""Run this script on ocamorph's result to discard repeated analyses."""
import sys

for line in sys.stdin:
    fields = []
    uniqs  = set()
    for field in line.strip().split():
        if field not in uniqs:
            fields.append(field)
            uniqs.add(field)
    print "\t".join(fields)
