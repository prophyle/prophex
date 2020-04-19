#include "contig_translator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#define MAX_kmersetS_COUNT 100000
#define MAX_CONTIGS_COUNT 200000000

static int contig_to_kmerset[MAX_CONTIGS_COUNT];
static char* kmerset_names[MAX_kmersetS_COUNT];
static int kmerset_name_lengths[MAX_kmersetS_COUNT];
static int kmersets_count = 0;
static int contigs_count = 0;

int get_kmerset_from_contig(int contig) {
	if (contig < 0 || contig >= contigs_count) {
		fprintf(stderr, "[prophex:%s] contig %d is outside of range [%d, %d]\n", __func__, contig, 0, contigs_count - 1);
	}
	return contig_to_kmerset[contig];
}

char* get_kmerset_name(int kmerset) { return kmerset_names[kmerset]; }

int get_kmerset_name_length(int kmerset) { return kmerset_name_lengths[kmerset]; }

void add_contig(char* contig, int contig_number) {
	xassert(contigs_count < MAX_CONTIGS_COUNT,
	        "[prophex] there are more than MAX_CONTIGS_COUNT contigs, try to increase MAX_CONTIGS_COUNT in contig_kmerset_translator.c\n");
	contigs_count++;
	const char* ch = strchr(contig, '@');
	int index = 0;
	if (ch == NULL) {
		index = strlen(contig);
	} else {
		index = ch - contig;
	}
	contig[index] = '\0';
	if (kmersets_count == 0 || strcmp(contig, kmerset_names[kmersets_count - 1])) {
		char* kmerset_name = malloc((index + 1) * sizeof(char));
		memcpy(kmerset_name, contig, index);
		kmerset_name[index] = '\0';
		xassert(kmersets_count < MAX_kmersetS_COUNT,
		        "[prophex] there are more than MAX_kmersetS_COUNT kmersets, try to increase MAX_kmersetS_COUNT in contig_kmerset_translator.c\n");
		kmerset_names[kmersets_count] = kmerset_name;
		kmerset_name_lengths[kmersets_count] = strlen(kmerset_name);
		contig_to_kmerset[contig_number] = kmersets_count;
		kmersets_count++;
	} else {
		contig_to_kmerset[contig_number] = kmersets_count - 1;
	}
}
