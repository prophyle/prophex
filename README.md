# ProPhex

[![Build Status](https://travis-ci.org/prophyle/prophex.svg?branch=master)](https://travis-ci.org/prophyle/prophex)

ProPhex is an efficient k-mer index with a small memory footprint. It uses the
[BWA](http://github.com/lh3/bwa) implementation of the BWT-index. ProPhex is
designed as a core computational component of
[ProPhyle](https://prophyle.github.io), a phylogeny-based metagenomic
classifier allowing fast and accurate read assignment.


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
<!---
USAGE-BEGIN
-->
```
Program: prophex (a lossless k-mer index)
Version: 0.1.0
Authors: Kamil Salikhov, Karel Brinda, Simone Pignotti, Gregory Kucherov
Contact: kamil.salikhov@univ-mlv.fr

Usage:   prophex command [options]

Command: index           construct a BWA index and k-LCP
         query           query reads against index

         klcp            construct an additional k-LCP
         bwtdowngrade    downgrade .bwt to the old, more compact format without Occ
         bwt2fa          reconstruct FASTA from BWT

```

```
Usage:   prophex index <prefix>
Options: -k INT    k-mer length for k-LCP
         -s        construct k-LCP and SA in parallel
         -i        sampling distance for SA
         -h        print help message

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
         -h        print help message

```

```
Usage:   prophex klcp <prefix>

Options: -k INT    length of k-mer
         -s        construct k-LCP and SA in parallel
         -i        sampling distance for SA
         -h        print help message

```

```
Usage:   prophex bwtdowngrade input.bwt output.bwt
         -h        print help message

```

```
Usage:   prophex bwt2fa input.fa output.fa
         -h        print help message

```

<!---USAGE-END
-->

## Quick example

**Build a BWA index**

```
./prophex index index.fa
```

**Build a k-LCP array for k=25**

```
./prophex klcp -k 25 index.fa
```

**Query reads from reads.fq for k=25 using the k-LCP array with 4 threads:**

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
5. Assigned k-mers. Space-delimited list of k-mer blocks with the same assignments. The list is of
   the following format: comma-delimited list of sets (or `A` for ambiguous, or
   `0` for no matches), colon, length. Example: `2157,393595:1 393595:1 0:16` (the first k-mer assigned to the nodes `2157` and `393595`, the second k-mer assigned to `393595`, the subsequent 16 k-mers unassigned)
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

[MIT](https://github.com/prophyle/prophex/blob/master/LICENSE.txt)


## Authors

Kamil Salikhov \<salikhov.kamil@gmail.com\>

[Karel Brinda](http://brinda.cz) \<kbrinda@hsph.harvard.edu\>

Simone Pignotti \<pignottisimone@gmail.com\>

[Gregory Kucherov](http://igm.univ-mlv.fr/~koutcher/) \<gregory.kucherov@univ-mlv.fr\>


