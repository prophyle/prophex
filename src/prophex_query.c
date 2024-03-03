#include "prophex_query.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include "bwa.h"
#include "bwa_utils.h"
#include "bwase.h"
#include "contig_node_translator.h"
#include "klcp.h"
#include "kseq.h"
#include "kstring.h"
#include "utils.h"
KSEQ_DECLARE(gzFile)

// maximum possible number of suffix array positions we can store
#define MAX_POSSIBLE_SA_POSITIONS 1000000
// maximum length of the string representing the output for the read
#define MAX_STREAK_LENGTH 10000000
// soft limit for the string representing the output for the read
#define MAX_SOFT_STREAK_LENGTH 9000000

void* kopen(const char* fn, int* _fd);
int kclose(void* a);

int calculate_sa_interval(const bwt_t* bwt, int len, const ubyte_t* str, uint64_t* k, uint64_t* l, int start_pos) {
	bwtint_t ok, ol;
	int i;
	for (i = start_pos; i < start_pos + len; ++i) {
		ubyte_t c = str[i];
		if (c > 3) {
			*k = 1;
			*l = 0;
			return i - start_pos;
		}
		if (c < 4) {
			bwt_2occ(bwt, *k - 1, *l, c, &ok, &ol);
			*k = bwt->L2[c] + ok + 1;
			*l = bwt->L2[c] + ol;
		}
		if (*k > *l) {
			return i - start_pos;
		}
	}
	return len;
}

int calculate_sa_interval_restart(const bwt_t* bwt, int len, const ubyte_t* str, uint64_t* k, uint64_t* l, int start_pos) {
	*k = 0;
	*l = bwt->seq_len;
	return calculate_sa_interval(bwt, len, str, k, l, start_pos);
}

int calculate_sa_interval_continue(const bwt_t* bwt, int len, const ubyte_t* str, uint64_t* k, uint64_t* l, uint64_t* decreased_k,
																	 uint64_t* increased_l, int start_pos, const klcp_t* klcp) {
	*k = decrease_sa_position(klcp, *k);
	*decreased_k = *k;
	*l = increase_sa_position(klcp, *l);
	*increased_l = *l;
	return calculate_sa_interval(bwt, len, str, k, l, start_pos);
}

size_t get_positions(const bwaidx_t* idx, bwt_position_t* positions, const int query_length, const uint64_t k, const uint64_t l) {
	uint64_t t;
	for (t = k; t <= l; ++t) {
		if (t - k >= MAX_POSSIBLE_SA_POSITIONS) {
			fprintf(stderr, "[prophex:%s] translation from SA-pos to seq-pos is truncated, too many (%llu) positions\n", __func__, l - k + 1);
			break;
		}
		int strand;
		uint64_t pos = bwa_sa2pos(idx->bns, idx->bwt, t, query_length, &strand);	// bwt_sa(bwt, t);
		positions[t - k].position = pos;
		positions[t - k].strand = strand;
		positions[t - k].rid = -1;
	}
	return (l - k + 1 < MAX_POSSIBLE_SA_POSITIONS ? l - k + 1 : MAX_POSSIBLE_SA_POSITIONS);
}

int is_position_on_border(const bwaidx_t* idx, bwt_position_t* position, int query_length) {
	return (position->position + query_length > idx->bns->l_pac ||
					(position->rid + 1 < idx->bns->n_seqs && position->position + query_length > idx->bns->anns[position->rid + 1].offset) ||
					position->position < idx->bns->anns[position->rid].offset);
}

void sort(int count, int** array) {
	int i;
	for (i = 1; i < count; ++i) {
		int x = (*array)[i];
		int j = i - 1;
		while (j >= 0 && (*array)[j] > x) {
			(*array)[j + 1] = (*array)[j];
			j--;
		}
		(*array)[j + 1] = x;
	}
}

size_t get_nodes_from_positions(const bwaidx_t* idx, const int query_length, const int positions_cnt, bwt_position_t* positions, int32_t* seen_nodes,
																int8_t** seen_nodes_marks, int skip_positions_on_border) {
	size_t nodes_cnt = 0;
	int i;
	for (i = 0; i < positions_cnt; ++i) {
		uint64_t pos = positions[i].position;
		if (pos == (uint64_t)-1) {
			continue;
		}
		int rid = positions[i].rid;
		if (rid == -1 || is_position_on_border(idx, &(positions[i]), query_length)) {
			rid = bns_pos2rid(idx->bns, pos);
			positions[i].rid = rid;
		}
		int node = get_node_from_contig(rid);
		positions[i].node = node;
		int seen = (*seen_nodes_marks)[node];
		if (!seen && node != -1 && (!skip_positions_on_border || !is_position_on_border(idx, &(positions[i]), query_length))) {
			seen_nodes[nodes_cnt] = node;
			++nodes_cnt;
			(*seen_nodes_marks)[node] = 1;
		}
	}
	int r;
	for (r = 0; r < nodes_cnt; ++r) {
		(*seen_nodes_marks)[seen_nodes[r]] = 0;
	}
	sort(nodes_cnt, &seen_nodes);
	return nodes_cnt;
}

void output_old(int* seen_nodes, const int nodes_cnt) {
	fprintf(stdout, "%d ", nodes_cnt);
	int r;
	for (r = 0; r < nodes_cnt; ++r) {
		fprintf(stdout, "%s ", get_node_name(seen_nodes[r]));
	}
	fprintf(stdout, "\n");
}

void strncat_with_check(char* str, char* str_to_append, int* str_length, int str_to_append_length, int length_limit) {
	if (*str_length >= length_limit) {
		fprintf(stderr, "[prophex:%s] too long output string, more than %d symbols\n", __func__, length_limit);
	} else {
		strncat(str, str_to_append, length_limit - str_to_append_length);
		*str_length += str_to_append_length;
		if (*str_length > length_limit) {
			*str_length = length_limit;
		}
	}
}

void construct_streaks(char** all_streaks, char** current_streak, int* seen_nodes, int nodes_cnt, int streak_size, int is_ambiguous_streak,
		int* is_first_streak, int is_filtered_streak) {
	if (*is_first_streak) {
		*all_streaks[0] = '\0';
	}
	*current_streak[0] = '\0';
	int current_streak_approximate_length = 0;
	if (is_ambiguous_streak) {
		strcat(*current_streak, "A:");
		current_streak_approximate_length += 2;
	} else if (is_filtered_streak) {
		strcat(*current_streak, "X:");
		current_streak_approximate_length += 2;
	} else if (nodes_cnt > 0) {
		int r;
		for (r = 0; r < nodes_cnt - 1; ++r) {
			strncat_with_check(*current_streak, get_node_name(seen_nodes[r]), &current_streak_approximate_length, get_node_name_length(seen_nodes[r]),
					MAX_SOFT_STREAK_LENGTH);
			strncat_with_check(*current_streak, ",", &current_streak_approximate_length, 1, MAX_SOFT_STREAK_LENGTH);
		}
		strncat_with_check(*current_streak, get_node_name(seen_nodes[nodes_cnt - 1]), &current_streak_approximate_length,
											 get_node_name_length(seen_nodes[nodes_cnt - 1]), MAX_SOFT_STREAK_LENGTH);
		strncat_with_check(*current_streak, ":", &current_streak_approximate_length, 1, MAX_SOFT_STREAK_LENGTH);
	} else {
		strncat_with_check(*current_streak, "0:", &current_streak_approximate_length, 2, MAX_SOFT_STREAK_LENGTH);
	}
	sprintf(*current_streak + strlen(*current_streak), "%d", streak_size);
	current_streak_approximate_length += 3;
	if (*is_first_streak) {
		if (current_streak_approximate_length <= MAX_STREAK_LENGTH) {
			strcpy(*all_streaks, *current_streak);
		} else {
			strncpy(*all_streaks, *current_streak, MAX_STREAK_LENGTH);
		}
	} else {
		strncat_with_check(*current_streak, " ", &current_streak_approximate_length, 1, MAX_SOFT_STREAK_LENGTH);
		strncat_with_check(*current_streak, *all_streaks, &current_streak_approximate_length, strlen(*all_streaks), MAX_SOFT_STREAK_LENGTH);
		char* tmp = *all_streaks;
		*all_streaks = *current_streak;
		*current_streak = tmp;
	}
	*is_first_streak = 0;
}

void print_streaks(char* streaks) { fprintf(stdout, "%s", streaks); }

void shift_positions_by_one(const bwaidx_t* idx, int positions_cnt, bwt_position_t* positions, int query_length, uint64_t k, uint64_t l) {
	int i;
	for (i = 0; i < positions_cnt; ++i) {
		if (positions[i].position == (uint64_t)-1) {
			continue;
		}
		if (positions[i].strand == 0) {
			positions[i].position++;
		} else {
			positions[i].position--;
		}
	}
}

int equal(int a_cnt, const int* a, int b_cnt, const int* b) {
	if (a_cnt != b_cnt) {
		return 0;
	}
	int i;
	for (i = 0; i < a_cnt; ++i) {
		if (a[i] != b[i]) {
			return 0;
		}
	}
	return 1;
}

void print_read(FILE* f, const bseq1_t* p) {
	int j;
	for (j = (int)p->l_seq - 1; j >= 0; j--) {
		fprintf(f, "%c", "ACGTN"[p->seq[j]]);
	}
}

void print_read_qual(FILE* f, const bseq1_t* p) {
	if (p->qual) {
		int j;
		for (j = 0; j < (int)p->l_seq; j++) {
			fprintf(f, "%c", p->qual[j]);
		}
	} else {
		fprintf(f, "*");
	}
}

prophex_worker_t* prophex_worker_init(const bwaidx_t* idx, int32_t seqs_cnt, const bseq1_t* seqs, const prophex_opt_t* opt, const klcp_t* klcp) {
	prophex_worker_t* prophex_worker = malloc(1 * sizeof(prophex_worker_t));
	prophex_worker->idx = idx;
	prophex_worker->seqs = seqs;
	prophex_worker->opt = opt;
	prophex_worker->klcp = klcp;
	prophex_worker->aux_data = malloc(opt->n_threads * sizeof(prophex_query_aux_t));
	int tid;
	for (tid = 0; tid < opt->n_threads; ++tid) {
		prophex_worker->aux_data[tid].positions = malloc(MAX_POSSIBLE_SA_POSITIONS * sizeof(bwt_position_t));
		prophex_worker->aux_data[tid].all_streaks = malloc(MAX_STREAK_LENGTH * sizeof(char));
		prophex_worker->aux_data[tid].current_streak = malloc(MAX_STREAK_LENGTH * sizeof(char));
		prophex_worker->aux_data[tid].seen_nodes = malloc(MAX_POSSIBLE_SA_POSITIONS * sizeof(int32_t));
		prophex_worker->aux_data[tid].prev_seen_nodes = malloc(MAX_POSSIBLE_SA_POSITIONS * sizeof(int32_t));
		prophex_worker->aux_data[tid].seen_nodes_marks = malloc(idx->bns->n_seqs * sizeof(int8_t));
		int index;
		for (index = 0; index < idx->bns->n_seqs; ++index) {
			prophex_worker->aux_data[tid].seen_nodes_marks[index] = 0;
		}
		prophex_worker->aux_data[tid].rids_computations = 0;
		prophex_worker->aux_data[tid].using_prev_rids = 0;
	}
	prophex_worker->seqs_cnt = seqs_cnt;
	prophex_worker->output = malloc(seqs_cnt * sizeof(char*));
	prophex_worker->passed = malloc(seqs_cnt * sizeof(int));
	int i = 0;
	for (i = 0; i < seqs_cnt; ++i) {
		prophex_worker->output[i] = NULL;
		prophex_worker->passed[i] = 0;
	}
	return prophex_worker;
}

void prophex_aux_data_destroy(prophex_query_aux_t* prophex_query_aux_data) {
	if (prophex_query_aux_data->positions) {
		free(prophex_query_aux_data->positions);
	}
	if (prophex_query_aux_data->all_streaks) {
		free(prophex_query_aux_data->all_streaks);
	}
	if (prophex_query_aux_data->current_streak) {
		free(prophex_query_aux_data->current_streak);
	}
	if (prophex_query_aux_data->seen_nodes) {
		free(prophex_query_aux_data->seen_nodes);
	}
	if (prophex_query_aux_data->prev_seen_nodes) {
		free(prophex_query_aux_data->prev_seen_nodes);
	}
	if (prophex_query_aux_data->seen_nodes_marks) {
		free(prophex_query_aux_data->seen_nodes_marks);
	}
}

void prophex_worker_destroy(prophex_worker_t* prophex_worker) {
	if (!prophex_worker) {
		return;
	}
	int i;
	for (i = 0; i < prophex_worker->opt->n_threads; ++i) {
		prophex_aux_data_destroy(&prophex_worker->aux_data[i]);
	}
	if (prophex_worker->aux_data) {
		free(prophex_worker->aux_data);
	}
	if (prophex_worker->output) {
		for (i = 0; i < prophex_worker->seqs_cnt; ++i) {
			if (prophex_worker->output[i]) {
				free(prophex_worker->output[i]);
			}
		}
		if (prophex_worker->output) {
			free(prophex_worker->output);
		}
	}
	if (prophex_worker->passed) {
		free(prophex_worker->passed);
	}
	free(prophex_worker);
}

void process_sequence(void* data, int seq_index, int tid) {
	prophex_worker_t* prophex_worker = (prophex_worker_t*)data;
	const bwaidx_t* idx = prophex_worker->idx;
	bseq1_t seq = prophex_worker->seqs[seq_index];
	const prophex_opt_t* opt = prophex_worker->opt;
	const klcp_t* klcp = prophex_worker->klcp;
	prophex_query_aux_t aux_data = prophex_worker->aux_data[tid];
	char* current_streak = aux_data.current_streak;
	char* all_streaks = aux_data.all_streaks;
	int32_t* seen_nodes = aux_data.seen_nodes;
	int32_t* prev_seen_nodes = aux_data.prev_seen_nodes;
	int8_t* seen_nodes_marks = aux_data.seen_nodes_marks;
	int i;

	for (i = 0; i < seq.l_seq; ++i) // convert to 2-bit encoding if we have not done so
		seq.seq[i] = seq.seq[i] < 4 ? seq.seq[i] : nst_nt4_table[(int)seq.seq[i]];
	seq_reverse(seq.l_seq, seq.seq, 0);

	if (opt->output_old) {
		fprintf(stdout, "#");
		print_read(stdout, &seq);
		fprintf(stdout, "\n");
	}
	bwt_t* bwt = idx->bwt;
	uint64_t k = 0, l = 0, prev_k = 1, prev_l = 0;
	int current_streak_size = 0;
	int prev_nodes_count = 0;
	int start_pos = 0;
	size_t positions_cnt = 0;
	uint64_t decreased_k = 1;
	uint64_t increased_l = 0;
	int is_first_streak = 1;
	int last_ambiguous_index = 0 - opt->kmer_length;
	int is_ambiguous_streak = 0;
	int ambiguous_streak_just_ended = 0;
	int match_number_to_pass = 0;
	int matches = 0;

	if (opt->match_proportion >= 0) {
		match_number_to_pass = opt->match_proportion * (seq.l_seq - opt->kmer_length + 1);
	} else {
		match_number_to_pass = opt->match_number;
	}
	if (start_pos + opt->kmer_length > seq.l_seq) {
		if (opt->output) {
			prophex_worker->output[seq_index] = malloc(5 * sizeof(char));
			strncpy(prophex_worker->output[seq_index], "0:0", 5);
		}
	} else {
		int index = 0;
		for (index = 0; index < opt->kmer_length; ++index) {
			if (seq.seq[index] > 3) {
				last_ambiguous_index = index;
			}
		}
		while (start_pos + opt->kmer_length <= seq.l_seq) {
			int end_pos = start_pos + opt->kmer_length - 1;
			if (!opt->is_query && matches + seq.l_seq - end_pos < match_number_to_pass) {
				break;
			}
			if (!opt->is_query && matches >= match_number_to_pass) {
				break;
			}
			if (opt->output) {
				if (start_pos > 0 && seq.seq[end_pos] > 3) {
					last_ambiguous_index = end_pos;
				}
				if (end_pos - last_ambiguous_index < opt->kmer_length) {
					if (!is_ambiguous_streak) {
						construct_streaks(&all_streaks, &current_streak, prev_seen_nodes, prev_nodes_count, current_streak_size, is_ambiguous_streak,
							&is_first_streak, 0);
						is_ambiguous_streak = 1;
						current_streak_size = 1;
					} else {
						current_streak_size++;
					}
					start_pos++;
					continue;
				} else {
					if (is_ambiguous_streak && current_streak_size > 0) {
						construct_streaks(&all_streaks, &current_streak, prev_seen_nodes, prev_nodes_count, current_streak_size, is_ambiguous_streak,
							&is_first_streak, 0);
						is_ambiguous_streak = 0;
						current_streak_size = 0;
					}
				}
				if (end_pos - last_ambiguous_index == opt->kmer_length) {
					k = 0;
					l = 0;
					prev_k = 1;
					prev_l = 0;
					prev_nodes_count = 0;
					ambiguous_streak_just_ended = 1;
				} else {
					ambiguous_streak_just_ended = 0;
				}
			}
			if (start_pos == 0 || ambiguous_streak_just_ended) {
				k = 0;
				l = 0;
				calculate_sa_interval_restart(bwt, opt->kmer_length, seq.seq, &k, &l, start_pos);
			} else {
				if (opt->use_klcp && k <= l) {
					calculate_sa_interval_continue(bwt, 1, seq.seq, &k, &l, &decreased_k, &increased_l, start_pos + opt->kmer_length - 1, klcp);
				} else {
					k = 0;
					l = 0;
					calculate_sa_interval_restart(bwt, opt->kmer_length, seq.seq, &k, &l, start_pos);
				}
			}
			int nodes_cnt = 0;
			if (k <= l) {
				if (prev_l - prev_k == l - k && increased_l - decreased_k == l - k) {
					aux_data.using_prev_rids++;
					shift_positions_by_one(idx, positions_cnt, aux_data.positions, opt->kmer_length, k, l);
				} else {
					aux_data.rids_computations++;
					positions_cnt = get_positions(idx, aux_data.positions, opt->kmer_length, k, l);
				}
				nodes_cnt = get_nodes_from_positions(idx, opt->kmer_length, positions_cnt, aux_data.positions, seen_nodes, &seen_nodes_marks,
						opt->skip_positions_on_border);
				if (nodes_cnt > 0) {
					matches++;
				}
			}
			if (opt->output_old) {
				output_old(seen_nodes, nodes_cnt);
			} else if (opt->output) {
				if (start_pos == 0 || ambiguous_streak_just_ended || (equal(nodes_cnt, seen_nodes, prev_nodes_count, prev_seen_nodes))) {
					current_streak_size++;
				} else {
					construct_streaks(&all_streaks, &current_streak, prev_seen_nodes, prev_nodes_count, current_streak_size, is_ambiguous_streak,
							&is_first_streak, 0);
					current_streak_size = 1;
				}
			}
			int* tmp = seen_nodes;
			seen_nodes = prev_seen_nodes;
			prev_seen_nodes = tmp;
			prev_nodes_count = nodes_cnt;
			prev_k = k;
			prev_l = l;
			start_pos++;
		}
		if (current_streak_size > 0) {
			construct_streaks(&all_streaks, &current_streak, prev_seen_nodes, prev_nodes_count, current_streak_size, is_ambiguous_streak,
					&is_first_streak, 0);
		}
		if (opt->output) {
			if (!opt->is_query) {
				if (matches >= match_number_to_pass) {
          prophex_worker->passed[seq_index] = 1;
				}
				current_streak_size = seq.l_seq - start_pos - opt->kmer_length + 1;
        if (current_streak_size > 0) {
				  is_ambiguous_streak = 0;
				  int is_filtered_streak = 1;
				  construct_streaks(&all_streaks, &current_streak, prev_seen_nodes, prev_nodes_count, current_streak_size, is_ambiguous_streak, &is_first_streak, is_filtered_streak);
        }
			}
			size_t all_streaks_length = strlen(all_streaks);
			prophex_worker->output[seq_index] = malloc((all_streaks_length + 1) * sizeof(char));
			strncpy(prophex_worker->output[seq_index], all_streaks, all_streaks_length + 1);
		}
	}
}

void process_sequences(const bwaidx_t* idx, int n_seqs, bseq1_t* seqs, const prophex_opt_t* opt, const klcp_t* klcp) {
	extern void kt_for(int n_threads, void (*func)(void*, int, int), void* data, int n);
	bwase_initialize();
	prophex_worker_t* prophex_worker = prophex_worker_init(idx, n_seqs, seqs, opt, klcp);
	kt_for(opt->n_threads, process_sequence, prophex_worker, n_seqs);
	int i;
	FILE* passed_reads_file;
	FILE* filtered_reads_file;
	if (!opt->is_query) {
		if (opt->passed_reads_file_name) {
			passed_reads_file = fopen(opt->passed_reads_file_name, "w");
		}
		if (opt->filtered_reads_file_name) {
			filtered_reads_file = fopen(opt->filtered_reads_file_name, "w");
		}
	}
	for (i = 0; i < n_seqs; ++i) {
		bseq1_t* seq = seqs + i;
		if (opt->output) {
			if (opt->is_query) {
				fprintf(stdout, "U");
			} else {
				if (prophex_worker->passed[i]) {
					fprintf(stdout, "C");
					if (opt->passed_reads_file_name) {
						fprintf(passed_reads_file, "@%s\n", seq->name);
						print_read(passed_reads_file, seq);
						fprintf(passed_reads_file, "\n+\n");
						print_read_qual(passed_reads_file, seq);
						fprintf(passed_reads_file, "\n");
					}
				} else {
					fprintf(stdout, "U");
					if (opt->filtered_reads_file_name) {
						fprintf(filtered_reads_file, "%s\n", seq->name);
						print_read(filtered_reads_file, seq);
						fprintf(filtered_reads_file, "\n+\n");
						print_read_qual(filtered_reads_file, seq);
						fprintf(filtered_reads_file, "\n");
					}
				}
			}
			fprintf(stdout, "\t%s\t0\t%d\t", seq->name, seq->l_seq);
			print_streaks(prophex_worker->output[i]);
			if (opt->output_read_qual) {
				fprintf(stdout, "\t");
				print_read(stdout, seq);
				fprintf(stdout, "\t");
				print_read_qual(stdout, seq);
			}
			fprintf(stdout, "\n");
		}
	}
	if (opt->filtered_reads_file_name) {
		fclose(filtered_reads_file);
	}
	if (opt->passed_reads_file_name) {
		fclose(passed_reads_file);
	}
	prophex_worker_destroy(prophex_worker);
}

void destroy_reads(int n_seqs, bseq1_t* seqs) {
	int i;
	if (!seqs) {
		return;
	}
	for (i = 0; i < n_seqs; ++i) {
		bseq1_t* seq = seqs + i;
		if (seq->name) {
			free(seq->name);
		}
		if (seq->comment) {
			free(seq->comment);
		}
		if (seq->seq) {
			free(seq->seq);
		}
		if (seq->qual) {
			free(seq->qual);
		}
	}
	free(seqs);
}

void query(const char* prefix, const char* fn_fa, const prophex_opt_t* opt) {
	extern bwa_seqio_t* bwa_open_reads(int mode, const char* fn_fa);

	int n_seqs;
	bseq1_t* seqs;
	bwaidx_t* idx;
	int i;
	FILE* log_file;
	gzFile fp = 0;
	void* ko = 0;

	if (opt->need_log) {
		log_file = fopen(opt->log_file_name, "w");
	} else {
		log_file = stderr;
	}

	if ((idx = bwa_idx_load_partial(prefix, BWA_IDX_ALL, opt->need_log, log_file)) == 0) {
		fprintf(stderr, "[prophex:%s] Couldn't load idx from %s\n", __func__, prefix);
		return;
	}

	// If fa2pac was called only for doubled string, then set bns->l_pac = bwt->seq_len, as it is for forward-only string
	idx->bns->l_pac = idx->bwt->seq_len / 2;

	bwa_destroy_unused_fields(idx);

	double ctime, rtime;
	ctime = cputime();
	rtime = realtime();
	klcp_t* klcp = malloc(sizeof(klcp_t));
	klcp->klcp = malloc(sizeof(bitarray_t));

	if (opt->use_klcp) {
		char* fn = malloc((strlen(prefix) + 10) * sizeof(char));
		strcpy(fn, prefix);
		strcat(fn, ".");
		char* kmer_length_str = malloc(5 * sizeof(char));
		sprintf(kmer_length_str, "%d", opt->kmer_length);
		strcat(fn, kmer_length_str);
		strcat(fn, ".klcp");
		klcp_restore(fn, klcp);
		free(fn);
		fprintf(log_file, "klcp_loading\t%.2fs\n", realtime() - rtime);
	}
	float total_time = 0;
	int64_t total_seqs = 0;
	ctime = cputime();
	rtime = realtime();
	int64_t total_kmers_count = 0;
	int fd;
	ko = kopen(fn_fa, &fd);
	if (ko == 0) {
		if (bwa_verbose >= 1)
			fprintf(stderr, "[E::%s] fail to open file `%s'.\n", __func__, fn_fa);
		return;
	}
	fp = gzdopen(fd, "r");
	kseq_t* ks = kseq_init(fp);

	while ((seqs = bseq_read(opt->read_chunk_size, &n_seqs, ks, NULL)) != 0) {
		process_sequences(idx, n_seqs, seqs, opt, klcp);
		total_seqs += n_seqs;
		for (i = 0; i < n_seqs; ++i) {
			int seq_kmers_count = seqs[i].l_seq - opt->kmer_length + 1;
			if (seq_kmers_count > 0) {
				total_kmers_count += seq_kmers_count;
			}
		}
		destroy_reads(n_seqs, seqs);
	}
	total_time = realtime() - rtime;

	fprintf(stderr, "[prophex:%s] match time: %.2f sec\n", __func__, total_time);
	fprintf(stderr, "[prophex::%s] Processed %llu reads in %.3f CPU sec, %.3f real sec\n", __func__, total_seqs, cputime() - ctime,
					realtime() - rtime);
	if (opt->need_log) {
		fprintf(log_file, "matching_time\t%.2fs\n", total_time);
		fprintf(log_file, "reads\t%" PRId64 "\n", total_seqs);
		fprintf(log_file, "kmers\t%" PRId64 "\n", total_kmers_count);
		fprintf(log_file, "rpm\t%" PRId64 "\n", (int64_t)(round(total_seqs * 60.0 / total_time)));
		fprintf(log_file, "kpm\t%" PRId64 "\n", (int64_t)(round(total_kmers_count * 60.0 / total_time)));
	}
	if (opt->need_log) {
		fclose(log_file);
	}
	if (opt->use_klcp) {
		destroy_klcp(klcp);
	} else {
		free(klcp->klcp);
		free(klcp);
	}

	bwa_idx_destroy_without_bns_name_and_anno(idx);
	kseq_destroy(ks);
	err_gzclose(fp);
	kclose(ko);
}
