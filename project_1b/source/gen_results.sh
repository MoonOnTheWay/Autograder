#!/bin/bash
mkdir -p results
touch results
for prog; do
  [ -x "$prog" ] || { echo -e "unknown progam $prog\nUsage: $0 <progname1> ..."; exit 1; }
  < "tests-$prog" xargs -n1 -I'{}' sh -c "'$PWD/$prog' -q {} > 'results/${prog}-{}.txt'"
done
