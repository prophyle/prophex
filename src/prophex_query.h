/*
    prophex query command implementation.
    Author: Kamil Salikhov <salikhov.kamil@gmail.com>
    Licence: MIT
*/

#ifndef PROPHEX_QUERY_H
#define PROPHEX_QUERY_H

#include <stdint.h>
#include "bwa.h"
#include "bwt.h"
#include "bwtaln.h"
#include "klcp.h"
#include "prophex_utils.h"

typedef struct {
	uint64_t position;
	int strand;
	int rid;
	int node;
} bwt_position_t;

typedef struct {
	bwt_position_t* positions;
	char* all_streaks;
	char* current_streak;
	int32_t* seen_nodes;
	int32_t* prev_seen_nodes;
	int8_t* seen_nodes_marks;
	int rids_computations;
	int using_prev_rids;
} prophex_query_aux_t;

typedef struct {
	const bwaidx_t* idx;
	const klcp_t* klcp;
	const prophex_opt_t* opt;
	const bseq1_t* seqs;
	prophex_query_aux_t* aux_data;
	int32_t seqs_cnt;
	char** output;
	int* passed;
} prophex_worker_t;

void query(const char* prefix, const char* fn_fa, const prophex_opt_t* opt);

#endif  // PROPHEX_QUERY_H
