#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include "blowfish.h"
#include "io.h"

int main(int argc, char** argv) {
    if (argc != 7) {
        printf("Usage: %s key_file key_length input_file output_file mode(0 - encrypt, other integer - decrypt) omp_num_threads(0 - don't use openmp)\n", argv[0]);
        return 1;
    }

    char *key_file = argv[1];
    int key_size = atoi(argv[2]);
    char *input_file = argv[3];
    char *output_file = argv[4];
    int mode = atoi(argv[5]);
    int omp_num_threads = atoi(argv[6]);
    int input_file_size, input_length, file_saving_err = 0;
    double start_time, end_time;

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    uint8_t *key = (uint8_t *)read_file_chunk(key_file, key_size);
    key_expansion(key, 16);

    char *input_array = NULL;
    if (rank == 0) {
        input_file_size = read_entire_file(input_file, &input_array);
        input_length = input_file_size;
        if (is_padding_needed(input_file_size)) {
            input_array = add_padding(input_array, input_file_size, &input_length);
        }
        start_time = MPI_Wtime();
    }

    MPI_Bcast(&input_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int chunk_size = (input_length + size - 1) / size;
    int remainder = chunk_size % 8;
    if (remainder != 0) {
        chunk_size += (8 - remainder);
    }
    int *sendcounts = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (i == size - 1) ? (input_length - chunk_size * (size - 1)) : chunk_size;
        displs[i] = i * chunk_size;
    }
    int local_input_array_size = sendcounts[rank];
    char *local_input_array = (char*)malloc(local_input_array_size * sizeof(char));
    MPI_Scatterv(input_array, sendcounts, displs, MPI_CHAR, local_input_array, local_input_array_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    char *local_output_array = (char*)malloc(local_input_array_size * sizeof(char));
    if (mode) {
        if (omp_num_threads > 0) {
            omp_set_dynamic(0);
            omp_set_num_threads(omp_num_threads);
            blowfish_decrypt_string_openmp(local_input_array, local_input_array_size, local_output_array);
        } else {
            blowfish_decrypt_string(local_input_array, local_input_array_size, local_output_array);
        }
    } else {
        if (omp_num_threads > 0) {
            omp_set_dynamic(0);
            omp_set_num_threads(omp_num_threads);
            blowfish_encrypt_string_openmp(local_input_array, local_input_array_size, local_output_array);
        } else {
            blowfish_encrypt_string(local_input_array, local_input_array_size, local_output_array);
        }
    }

    char *output_array = NULL;
    if (rank == 0) {
        output_array = (char*)malloc(input_length * sizeof(char));
    }
    MPI_Gatherv(local_output_array, local_input_array_size, MPI_CHAR, output_array, sendcounts, displs, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Execution Time: %f seconds\n", end_time - start_time);
        file_saving_err = save_to_file(output_file, output_array, input_file_size);
        free(output_array);
        free(input_array);
    }

    free(local_input_array);
    free(local_output_array);
    free(sendcounts);
    free(displs);

    MPI_Finalize();

    if (file_saving_err) {
        fprintf(stderr, "Failed to save output data to file: %s\n", output_file);
        return -1;
    }
    return 0;
}
