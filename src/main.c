/*
    prophex main file, implements build and query commands.
    Author: Kamil Salikhov <salikhov.kamil@gmail.com>
    Licence: MIT
    Examples:
        build prophex index for k = 20, suffix array and klcp simultaneously:
            prophex build -k 20 -s index.fa
        query reads for k=20 using rolling window search with 10 threads, writing output in results.txt:
            prophex query -u -k 20 -t 10 index.fa reads.fq > results.txt
*/

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bwa.h"
#include "bwa_utils.h"
#include "prophex_build.h"
#include "prophex_query.h"
#include "version.h"

static int usage() {
	fprintf(stderr, "\n");
	fprintf(stderr, "Program: prophex (a lossless k-mer index)\n");
	fprintf(stderr, "Version: %s\n", VERSION);
	fprintf(stderr, "Authors: Kamil Salikhov, Karel Brinda, Simone Pignotti, Gregory Kucherov\n");
	fprintf(stderr, "Contact: kamil.salikhov@univ-mlv.fr, karel.brinda@gmail.com\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:   prophex <command> [options]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Command: index           construct a BWA index and k-LCP\n");
	fprintf(stderr, "         query           query reads against index\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "         klcp            construct an additional k-LCP\n");
	fprintf(stderr, "         bwtdowngrade    downgrade .bwt to the old, more compact format without Occ\n");
	fprintf(stderr, "         bwt2fa          reconstruct FASTA from BWT\n");
	fprintf(stderr, "\n");
	return 1;
}

static int usage_klcp() {
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:   prophex klcp [options] <idxbase>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options: -k INT    length of k-mer\n");
	fprintf(stderr, "         -s        construct k-LCP and SA in parallel\n");
	fprintf(stderr, "         -i        sampling distance for SA\n");
	fprintf(stderr, "         -h        print help message\n");
	fprintf(stderr, "\n");
	return 1;
}

static int usage_index() {
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:   prophex index [options] <idxbase>\n");
	fprintf(stderr, "Options: -k INT    k-mer length for k-LCP\n");
	fprintf(stderr, "         -s        construct k-LCP and SA in parallel\n");
	fprintf(stderr, "         -i        sampling distance for SA\n");
	fprintf(stderr, "         -h        print help message\n");
	fprintf(stderr, "\n");
	return 1;
}

static int usage_bwtdowngrade() {
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:   prophex bwtdowngrade <input.bwt> <output.bwt>\n");
	fprintf(stderr, "         -h        print help message\n");
	fprintf(stderr, "\n");
	return 1;
}

static int usage_bwt2fa() {
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:   prophex bwt2fa <idxbase> <output.fa>\n");
	fprintf(stderr, "         -h        print help message\n");
	fprintf(stderr, "\n");
	return 1;
}

static int usage_query(int threads) {
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage:   prophex query [options] <idxbase> <in.fq>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options: -k INT    length of k-mer\n");
	fprintf(stderr, "         -u        use k-LCP for querying\n");
	fprintf(stderr, "         -v        output set of chromosomes for every k-mer\n");
	fprintf(stderr, "         -p        do not check whether k-mer is on border of two contigs, and show such k-mers in output\n");
	fprintf(stderr, "         -b        print sequences and base qualities\n");
	fprintf(stderr, "         -l STR    log file name to output statistics\n");
	fprintf(stderr, "         -t INT    number of threads [%d]\n", threads);
	fprintf(stderr, "         -h        print help message\n");
	fprintf(stderr, "\n");
	return 1;
}

int prophex_query(int argc, char *argv[]) {
	int c;
	prophex_opt_t *opt;
	char *prefix;
	int usage = 0;
	opt = prophex_init_opt();
	while ((c = getopt(argc, argv, "l:psuvk:bt:h")) >= 0) {
		switch (c) {
			case 'v': {
				opt->output_old = 1;
				opt->output = 0;
			} break;
			case 'u':
				opt->use_klcp = 1;
				break;
			case 'k':
				opt->kmer_length = atoi(optarg);
				break;
			case 's':
				opt->skip_after_fail = 1;
				break;
			case 'p':
				opt->skip_positions_on_border = 0;
				break;
			case 'l': {
				opt->need_log = 1;
				opt->log_file_name = optarg;
				break;
			}
			case 'b':
				opt->output_read_qual = 1;
				break;
			case 't':
				opt->n_threads = atoi(optarg);
				break;
			case 'h':
				usage = 1;
				break;
			default:
				return 1;
		}
	}
	if (usage) {
		usage_query(opt->n_threads);
		return 0;
	}
	if (opt->output_old && opt->n_threads > 1) {
		fprintf(stderr, "[prophex:%s] -v option can be used only with one thread (-t 1)\n", __func__);
		return 1;
	}

	if (optind + 2 > argc) {
		usage_query(opt->n_threads);
		return 1;
	}
	if ((prefix = bwa_idx_infer_prefix(argv[optind])) == 0) {
		fprintf(stderr, "[prophex:%s] fail to locate the index %s\n", __func__, argv[optind]);
		free(opt);
		return 1;
	}
	query(prefix, argv[optind + 1], opt);
	free(opt);
	free(prefix);
	return 0;
}

int prophex_klcp(int argc, char *argv[]) {
	int c;
	prophex_opt_t *opt;
	char *prefix;
	opt = prophex_init_opt();
	int sa_intv = 32;
	int usage = 0;
	while ((c = getopt(argc, argv, "si:k:h")) >= 0) {
		switch (c) {
			case 'k':
				opt->kmer_length = atoi(optarg);
				break;
			case 'i':
				sa_intv = atoi(optarg);
				break;
			case 's':
				opt->construct_sa_parallel = 1;
				break;
			case 'h':
				usage = 1;
				break;
			default:
				return 1;
		}
	}
	if (usage) {
		usage_klcp();
		return 0;
	}
	if (optind + 1 > argc) {
		usage_klcp();
		return 1;
	}
	if ((prefix = bwa_idx_infer_prefix(argv[optind])) == 0) {
		fprintf(stderr, "[prophex:%s] fail to locate the index %s\n", __func__, argv[optind]);
		return 1;
	}
	build_klcp(prefix, opt, sa_intv);
	free(prefix);
	return 0;
}

int bwa_fa2pac(int argc, char *argv[]);
int bwa_pac2bwt(int argc, char *argv[]);
int bwt_bwtgen_main(int argc, char *argv[]);
int bwa_bwtupdate(int argc, char *argv[]);
int bwa_bwt2sa(int argc, char *argv[]);

int prophex_index(int argc, char *argv[]) {
	int c;
	prophex_opt_t *opt;
	opt = prophex_init_opt();
	int sa_intv = 32;
	int usage = 0;
	while ((c = getopt(argc, argv, "si:k:h")) >= 0) {
		switch (c) {
			case 'k':
				opt->kmer_length = atoi(optarg);
				break;
			case 'i':
				sa_intv = atoi(optarg);
				break;
			case 's':
				opt->construct_sa_parallel = 1;
				break;
			case 'h':
				usage = 1;
				break;
			default:
				return 1;
		}
	}
	if (usage) {
		usage_index();
		return 0;
	}
	if (optind + 1 > argc) {
		usage_index();
		return 1;
	}
	char *prefix = malloc(strlen(argv[optind]) * sizeof(char));
	strcpy(prefix, argv[optind]);

	char *arguments[3];
	arguments[0] = malloc(10 * sizeof(char));
	arguments[1] = malloc((strlen(prefix) + 10) * sizeof(char));
	arguments[2] = malloc((strlen(prefix) + 10) * sizeof(char));
	strcpy(arguments[0], "fa2pac");
	strcpy(arguments[1], prefix);
	strcpy(arguments[2], prefix);
	optind = 1;
	bwa_fa2pac(3, arguments);
	strcpy(arguments[0], "pac2bwt");
	strcat(arguments[1], ".pac");
	strcat(arguments[2], ".bwt");
	optind = 1;
	bwt_bwtgen_main(3, arguments);
	strcpy(arguments[0], "bwtupdate");
	strcpy(arguments[1], prefix);
	strcat(arguments[1], ".bwt");
	optind = 1;
	bwa_bwtupdate(2, arguments);
	if (opt->construct_sa_parallel) {
		build_klcp(prefix, opt, sa_intv);
	} else {
		strcpy(arguments[0], "bwt2sa");
		strcpy(arguments[1], prefix);
		strcat(arguments[1], ".bwt");
		strcpy(arguments[2], prefix);
		strcat(arguments[2], ".sa");
		optind = 1;
		bwa_bwt2sa(3, arguments);
	}
	free(prefix);
	return 0;
}

int prophex_bwtdowngrade(int argc, char *argv[]) {
	int c;
	int usage = 0;
	while ((c = getopt(argc, argv, "h")) >= 0) {
		switch (c) {
			case 'h':
				usage = 1;
				break;
			default:
				return 1;
		}
	}
	if (usage) {
		usage_bwtdowngrade();
		return 0;
	}
	if (argc < 3) {
		return usage_bwtdowngrade();
	}
	return bwtdowngrade(argv[1], argv[2]);
}

int prophex_bwt2fa(int argc, char *argv[]) {
	int c;
	int usage = 0;
	while ((c = getopt(argc, argv, "h")) >= 0) {
		switch (c) {
			case 'h':
				usage = 1;
				break;
			default:
				return 1;
		}
	}
	if (usage) {
		usage_bwt2fa();
		return 0;
	}
	if (argc < 3) {
		return usage_bwt2fa();
	}
	return bwt2fa(argv[1], argv[2]);
}

int main(int argc, char *argv[]) {
	int ret = 0;
	if (argc < 2) {
		usage();
		return 0;
	}
	if (strcmp(argv[1], "klcp") == 0)
		ret = prophex_klcp(argc - 1, argv + 1);
	else if (strcmp(argv[1], "query") == 0)
		ret = prophex_query(argc - 1, argv + 1);
	else if (strcmp(argv[1], "index") == 0)
		ret = prophex_index(argc - 1, argv + 1);
	else if (strcmp(argv[1], "bwtdowngrade") == 0)
		ret = prophex_bwtdowngrade(argc - 1, argv + 1);
	else if (strcmp(argv[1], "bwt2fa") == 0)
		ret = prophex_bwt2fa(argc - 1, argv + 1);
	else
		return usage();

	return ret;
}
