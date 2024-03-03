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
	// BWA options
	int mode;
	int trim_qual;

	// Index options
	int construct_sa_parallel;

	// Common query and filter options
	int is_query;
	int n_threads;
	int use_klcp;
	int kmer_length;
	int output;
	int output_read_qual;
	int skip_positions_on_border;
	int need_log;
	char* log_file_name;
	int read_chunk_size;

	// Query options
	int output_old;

	// Filter options
	double match_proportion;
	int match_number;
	char* passed_reads_file_name;
	char* filtered_reads_file_name;
} prophex_opt_t;

prophex_opt_t* prophex_init_opt();

#endif  // PROPHEX_UTILS_H
