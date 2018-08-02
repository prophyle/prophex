include ../conf.mk

K=12
M=0.0 0.1 0.5 1.0
N=1 5 10

.PHONY: all clean $(M) $(N)

all: $(addprefix m,$M) $(addprefix n,$N)

m%: _index.%.complete
	$(IND) filter -k $(K) -u -m $* _index.$*.fa $(FQ) > _obtained.$*.txt
	diff -c _obtained.$*.txt expected.$*.txt

n%: _index.%.complete
	$(IND) filter -k $(K) -u -n $* _index.$*.fa $(FQ) > _obtained.$*.txt
	diff -c _obtained.$*.txt expected.$*.txt

_index.%.complete:
	$(BWA) index -p _index.$*.fa $(FA)
	$(IND) klcp -k $(K) _index.$*.fa
	touch $@

clean:
	rm -f _*
