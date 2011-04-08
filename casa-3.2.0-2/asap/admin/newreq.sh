#!/bin/sh -f

touch reqsum.tex
latex requirements2
\rm reqsum.tex
./log2req.pl -purge requirements2.log > reqsum.tex
latex requirements2
