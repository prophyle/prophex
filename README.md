# ProPhex

[![Build Status](https://travis-ci.org/prophyle/prophex.svg?branch=master)](https://travis-ci.org/prophyle/prophex)

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

# Quick example

## Create BWA index for fasta:

```
./bwa in

## Issues

Please use [Github issues](https://github.com/prophyle/prophex/issues).


## Changelog

See [Releases](https://github.com/prophyle/prophex/releases).


## Licence

[MIT](https://github.com/prophyle/prophex/blob/master/LICENSE)


## Author

Kamil Salikhov \<kamil.salikhov@univ-mlv.fr\>
