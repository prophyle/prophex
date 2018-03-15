SHELL=/bin/bash -e -o pipefail

.SECONDARY:

IND=../../prophex
BWA=../../src/bwa/bwa
F2K=../fa_to_kmers.py
AK=../all_kmers.py

FQ=../simulation_bacteria.1000.fq
FA=index.fa
1STEP=../1step_match.py

d=$(shell pwd)
$(info )
$(info Directory: $(d))
$(info )
