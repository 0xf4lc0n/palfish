#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include "blowfish.h"
#include "io.h"

int main(int argc, char** argv) {
    if (argc != 7) {
        printf("Usage: %s key_file key_length input_file input_file_size output_file mode(0 - encrypt, other integer - decrypt) use_open_mp(1 = true)\n", argv[0]);
        return 1;
    }

    char *key_file = argv[1];
    int key_size = atoi(argv[2]);
    char *input_file = argv[3];
    char *output_file = argv[4];
    int mode = atoi(argv[5]);
    int use_open_mp = atoi(argv[6]);
    int file_size, input_len, err = 0;
    double start_time, end_time;

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    uint8_t *key = (uint8_t *)read_file_chunk(key_file, key_size);
    key_expansion(key, 16);

    char *array = NULL;
    if (rank == 0) {
        array = (char*)malloc(file_size * sizeof(char));
        file_size = read_entire_file(input_file, &array);
        input_len = file_size;
        if (is_padding_needed(array)) {
            array = add_padding(array, &input_len);
        }
        start_time = MPI_Wtime();
    }

    MPI_Bcast(&input_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int chunk_size = (input_len + size - 1) / size;
    int remainder = chunk_size % 8;
    if (remainder != 0) {
        chunk_size += (8 - remainder);
    }
    int *sendcounts = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        sendcounts[i] = (i == size - 1) ? (input_len - chunk_size * (size - 1)) : chunk_size;
        displs[i] = i * chunk_size;
    }
    int actual_size = sendcounts[rank];
    char *local_array = (char*)malloc(actual_size * sizeof(char));
    MPI_Scatterv(array, sendcounts, displs, MPI_CHAR, local_array, actual_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    char *new_array = (char*)malloc(actual_size * sizeof(char));
    if (mode) {
        if (use_open_mp) {
            blowfish_decrypt_string_openmp(local_array, actual_size, new_array);
        } else {
            blowfish_decrypt_string(local_array, actual_size, new_array);
        }
    } else {
        if (use_open_mp) {
            blowfish_encrypt_string_openmp(local_array, actual_size, new_array);
        } else {
            blowfish_encrypt_string(local_array, actual_size, new_array);
        }
    }

    char *modified_array = NULL;
    if (rank == 0) {
        modified_array = (char*)malloc(input_len * sizeof(char));
    }
    MPI_Gatherv(new_array, actual_size, MPI_CHAR, modified_array, sendcounts, displs, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Execution Time: %f seconds\n", end_time - start_time);
        err = save_to_file(output_file, modified_array, file_size);
        free(modified_array);
        free(array);
    }

    free(local_array);
    free(new_array);
    free(sendcounts);
    free(displs);

    MPI_Finalize();

    if (err != 0) {
        fprintf(stderr, "Failed to save message to the file\n");
        return -1;
    }
    return 0;
}
