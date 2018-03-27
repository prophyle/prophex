# ProPhex

[![Build Status](https://travis-ci.org/prophyle/prophex.svg?branch=master)](https://travis-ci.org/prophyle/prophex)

We introduce ProPhyle Index, a BWT-index based software for fast *k*-mer matching. It uses BWA[@BWA] as BWT-index implementation. ProPhyle Index is designed as a core computational part of [ProPhyle](https://prophyle.github.io), a phylogeny-based metagenomic classifier allowing fast and accurate read assignment.


## Prerequisities

* GCC 4.8+ or equivalent
* ZLib


## Getting started

```
git clone --recursive https://github.com/prophyle/prophex
cd prophex && make -j
```

### Alternative ways of installation

```
conda install prophex
```

# ProPhex commands

```
Usage:   prophex command [options]

Command: build         construct index
         query         query reads against index
         bwtdowngrade  downgrade .bwt to the old more compact format without OCC array
```

```
Usage:   prophex build <prefix>

Options: -k INT    length of k-mer
         -s        construct k-LCP and SA in parallel
         -i        sampling distance for SA
```

```
Usage:   prophex query [options] <prefix> <in.fq>

Options: -k INT    length of k-mer
         -u        use k-LCP for querying
         -v        output set of chromosomes for every k-mer
         -p        do not check whether k-mer is on border of two contigs, and show such k-mers in output
         -b        print sequences and base qualities
         -l STR    log file name to output statistics
         -t INT    number of threads [1]
         -r INT    total size of reads in one chunk [10000000 bp]
```

```
Usage:   prophex bwtdowngrade input.bwt output.bwt
```


## Quick example

### Create BWA index for index.fa:

```
./bwa index index.fa
```

### Create ProPhex index upon BWA index for k=25:

```
./prophex build -k 25 index.fa
```

### Query reads from reads.fq for k=25 using kLCP array with 4 threads:

```
./prophex query -k 25 -u -t 4 index.fa index.fq
```


## Output format

Matches are reported in an extended
[Kraken format](http://ccb.jhu.edu/software/kraken/MANUAL.html#output-format).
ProPhex produces a tab-delimited file with the following columns:

1. Category (unused, `U` as a legacy value)
2. Sequence name
3. Final decision (unused, `0` as a legacy value)
4. Sequence length
5. Assigned k-mers. Space-delimited list of blocks with the same assignment of
   the following format: comma-delimited list of sets (or `A` for ambiguous, or
   `0` for no matches), colon, length. Example: `2157,393595:1 393595:1 0:16`
6. Bases (optional)
7. Base qualities (optional)


## FAQs

> Can I remove duplicate k-mers from the index in order to use less memory when querying?

Yes, duplicate k-mers can be removed, e.g., using
[BCalm](https://github.com/GATB/bcalm). A ProPhex index can then be built from
the obtained unitigs.


## Issues

Please use [Github issues](https://github.com/prophyle/prophex/issues).


## Changelog

See [Releases](https://github.com/prophyle/prophex/releases).


## Licence

[MIT](https://github.com/prophyle/prophex/blob/master/LICENSE)


## Authors

Kamil Salikhov \<salikhov.kamil@gmail.com\>

[Karel Brinda](http://brinda.cz) \<kbrinda@hsph.harvard.edu\>

Simone Pignotti \<pignottisimone@gmail.com\>

[Gregory Kucherov](http://igm.univ-mlv.fr/~koutcher/) \<gregory.kucherov@univ-mlv.fr\>

