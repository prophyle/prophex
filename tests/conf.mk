SHELL=/bin/bash -e -u -o pipefail

.SECONDARY:

IND=../../prophyle_index

FQ=../simulation_bacteria.1000.fq
FA=index.fa

d=$(shell pwd)
$(info )
$(info Directory: $(d))
$(info )
