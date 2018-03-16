# ProPhex

[![Build Status](https://travis-ci.org/prophyle/prophex.svg?branch=master)](https://travis-ci.org/prophyle/prophex)

We introduce ProPhyle Index, a BWT-index based software for fast *k*-mer matching. It uses BWA[@BWA] as BWT-index implementation. ProPhyle Index is designed as a core computational part of [ProPhyle](https://prophyle.github.io), a phylogeny-based metagenomic classifier allowing fast and accurate read assignment.

## Prerequisities

* GCC 4.8+ or equivalent
* ZLib


## Getting started

```
git clone https://github.com/prophyle/prophex
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

# Quick example

## Create BWA index for index.fa:

```
./bwa index index.fa
```

## Create ProPhex index upon BWA index for k=25:

```
./prophex build -k 25 index.fa
```

## Query reads from reads.fq for k=25 using kLCP array with 4 threads:

```
./prophex query -k 25 -u -t 4 index.fa index.fq
```

## Issues

Please use [Github issues](https://github.com/prophyle/prophex/issues).


## Changelog

See [Releases](https://github.com/prophyle/prophex/releases).


## Licence

[MIT](https://github.com/prophyle/prophex/blob/master/LICENSE)


## Author

Kamil Salikhov \<salikhov.kamil@gmail.com\>
