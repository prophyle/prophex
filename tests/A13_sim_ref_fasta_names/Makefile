.PHONY: all clean diff

include ../conf.mk
K=3

all: diff

diff:
	$(IND) index $(FA)
	$(IND) klcp -k $(K) $(FA)
	$(IND) query -k $(K) $(FA) reads.fq > _obtained_matches.txt
	diff -c _obtained_matches.txt expected_matches.txt

clean:
	rm -f _* $(FA).*
