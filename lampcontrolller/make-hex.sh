#!/bin/sh
for i in $(seq 0 9); do
	gpasm -a inhx8m lc.asm -o lc.$i.hex -D IDX=$i
done
