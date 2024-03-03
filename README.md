# ProPhex

[![Build Status](https://travis-ci.org/prophyle/prophex.svg?branch=master)](https://travis-ci.org/prophyle/prophex)
[![Bioconda](https://img.shields.io/badge/install%20with-bioconda-brightgreen.svg?style=flat-square)](https://anaconda.org/bioconda/prophex)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1247431.svg)](http://doi.org/10.5281/zenodo.1247431)

ProPhex is an efficient k-mer index with a small memory footprint. It uses the
[BWA](http://github.com/lh3/bwa) implementation of the BWT-index. ProPhex is
designed as a core computational component of
[ProPhyle](https://prophyle.github.io), a phylogeny-based metagenomic
classifier allowing fast and accurate read assignment.


<!-- vim-markdown-toc GFM -->

* [Getting started](#getting-started)
  * [Alternative ways of installation](#alternative-ways-of-installation)
* [Quick example](#quick-example)
* [ProPhex commands](#prophex-commands)
* [Output format](#output-format)
* [FAQs](#faqs)
* [Issues](#issues)
* [Changelog](#changelog)
* [Licence](#licence)
* [Authors](#authors)

<!-- vim-markdown-toc -->




## Getting started

```
git clone https://github.com/prophyle/prophex
cd prophex && make -j
```

### Alternative ways of installation

```
conda install prophex
```



## Quick example


```
# Build a ProPhex index
./prophex index -k 25 index.fa

# Query reads from reads.fq for k=25 (with k-LCP)
./prophex query -k 25 -u -t 4 index.fa index.fq

# Query reads from reads.fq for k=20 (with 4 threads and without k-LCP)
./prophex query -k 20 index.fa index.fq

```




## ProPhex commands
<!---
USAGE-BEGIN
-->
```
Program: prophex (an exact k-mer index)
Version: 0.2.0
Authors: Kamil Salikhov, Karel Brinda, Simone Pignotti, Gregory Kucherov
Contact: kamil.salikhov@univ-mlv.fr, karel.brinda@gmail.com

Usage:   prophex <command> [options]

Command: index           index sequences in the FASTA format
         query           query k-mers

         klcp            construct an additional k-LCP array
         bwtdowngrade    remove OCC from .bwt
         bwt2fa          reconstruct .fa from .fa.bwt

```

```
Usage:   prophex index [options] <idxbase>
Options: -k INT    k-mer length for k-LCP
         -s        construct k-LCP and SA in parallel
         -i        sampling distance for SA
         -h        print help message

```

```
Usage:   prophex query [options] <idxbase> <in.fq>

Options: -k INT    k-mer length
         -u        use k-LCP for querying
         -b        append sequences and base qualities to the output
         -l STR    log file name to output statistics
         -t INT    number of threads [1]
         -h        print help message

```

```
Usage:   prophex klcp [options] <idxbase>

Options: -k INT    k-mer length
         -s        construct also SA, in parallel to k-LCP
         -i        sampling distance for SA
         -h        print help message

```

```
Usage:   prophex bwtdowngrade <input.bwt> <output.bwt>
         -h        print help message

```

```
Usage:   prophex bwt2fa <idxbase> <output.fa>
         -h        print help message

```

<!---USAGE-END
-->

## Output format

Matches are reported in the form of a tab-delimited file with the following
columns:

1. Sequence name
2. Sequence length
3. Assigned k-mers. Space-delimited list of k-mer blocks matching the same
   k-mer sets. The list is of the following format: comma-delimited list of
   k-mer sets (`~` for an ambiguous nucleotide name `*` for no k-mer matches),
   colon, the number of k-mers in the block. Example: `2157,393595:1 393595:1
   *:16` (the first k-mer assigned to the k-mer sets `2157` and `393595`, the
   second k-mer assigned to `393595`, and the subsequent 16 k-mers do not match
   anything)
4. Bases (optional)
5. Base qualities (optional)


## FAQs

> Can I remove duplicate k-mers from the index in order to use less memory when querying?

Yes, duplicate k-mers can be removed using
[ProphAsm](https://github.com/prophyle/prophasm), which assembles contigs by
greedy enumeration of disjoint paths in the associated de-Bruijn graph.
[BCalm](https://github.com/GATB/bcalm) is another tool that can be used with
ProPhex. Compared to ProPhex, BCalm has a smaller memory footprint. On the
other hand, the resulting FASTA file can be significantly bigger (when
assemblying, BCalm stops at every branching k-mer).



## Issues

Please use [Github issues](https://github.com/prophyle/prophex/issues).


## Changelog

See [Releases](https://github.com/prophyle/prophex/releases).


## Licence

[MIT](https://github.com/prophyle/prophex/blob/master/LICENSE.txt)


## Authors

Kamil Salikhov \<salikhov.kamil@gmail.com\>

[Karel Brinda](http://brinda.eu) \<karel.brinda@inria.fr\>

Simone Pignotti \<pignottisimone@gmail.com\>

[Gregory Kucherov](http://igm.univ-mlv.fr/~koutcher/) \<gregory.kucherov@univ-mlv.fr\>


