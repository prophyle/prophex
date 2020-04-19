#include "prophex_utils.h"

#include <stdlib.h>

prophex_opt_t* prophex_init_opt() {
	prophex_opt_t* o;
	o = (prophex_opt_t*)calloc(1, sizeof(prophex_opt_t));
	o->mode = BWA_MODE_GAPE | BWA_MODE_COMPREAD;
	o->n_threads = 1;
	o->trim_qual = 0;
	o->kmer_length = 14;
	o->use_klcp = 0;
	o->output = 1;
	o->output_read_qual = 0;
	o->output_old = 0;
	o->skip_positions_on_border = 1;
	o->construct_sa_parallel = 0;
	o->need_log = 0;
	o->log_file_name = NULL;
	o->read_chunk_size = READ_CHUNK_SIZE;
	return o;
}
