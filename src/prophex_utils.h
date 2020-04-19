/*
    Structure for prophex options.
    Author: Kamil Salikhov <salikhov.kamil@gmail.com>
    Licence: MIT
*/

#ifndef PROPHEX_UTILS_H
#define PROPHEX_UTILS_H

#include <stdint.h>
#include <stdlib.h>

#include "bwtaln.h"

// maximum total number base pairs in reads in one chunk
#define READ_CHUNK_SIZE 10000000

typedef struct {
	int mode;
	int n_threads;
	int trim_qual;
	int use_klcp;
	int kmer_length;
	int output;
	int output_old;
	int output_read_qual;
	int skip_after_fail;
	int skip_positions_on_border;
	int need_log;
	char* log_file_name;
	int construct_sa_parallel;
	int read_chunk_size;
} prophex_opt_t;

prophex_opt_t* prophex_init_opt();

#endif  // PROPHEX_UTILS_H
