/*
  Correspondance between contig_id in BWA and kmerset_name in taxonomic tree.
  Author: Kamil Salikhov <salikhov.kamil@gmail.com>
  Licence: MIT
*/

#ifndef CONTIG_TRANSLATOR_H
#define CONTIG_TRANSLATOR_H

#include <stdint.h>

int get_kmerset_from_contig(int contig);
char* get_kmerset_name(int kmerset);
int get_kmerset_name_length(int kmerset);
void add_contig(char* contig, int contig_number);

#endif  // CONTIG_TRANSLATOR_H
