/*
  Correspondance between contig_id in BWA and node_name in taxonomic tree.
  Author: Kamil Salikhov <salikhov.kamil@gmail.com>
  Licence: MIT
*/

#ifndef CONTIG_TRANSLATOR_H
#define CONTIG_TRANSLATOR_H

#include <stdint.h>

int get_kmerset_from_contig(int contig);
char* get_kmerset_name(int node);
int get_kmerset_name_length(int node);
void add_contig(char* contig, int contig_number);

#endif  // CONTIG_NODE_TRANSLATOR_H
