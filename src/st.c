#include <stdio.h>
#include "file_reader.h"
#include "parsers/simple-fasta-parser.h"
#include "parsers/simple-fastq-parser.h"


int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "%s genome reads", argv[0]);
        return 1;
    }
    const char *genome_fname = argv[1];
    const char *reads_fname = argv[2];

    char *fasta_str = read_file(genome_fname);
    char *reads_str = read_file(reads_fname);

    struct Fasta **fastas = parse_fasta(fasta_str);

    while (reads_str[0] != '\0') {
        char *fastq_header = read_fastq_head(&reads_str);
        char *pattern = read_fastq_pattern(&reads_str);

        struct Fasta **start_of_fastas = fastas;
        while (*fastas != NULL) {
            char *header = (*fastas)->fasta_head;
            char *sequence = (*fastas)->fasta_sequence;

            int match_index = 0;
            int pattern_len = 0;
            printf("%s\t%s\t%d\t%dM\t%s\n", fastq_header, header, match_index, pattern_len, pattern);

            fastas++;
        }
        fastas = start_of_fastas;
    }

    return 0;
}


