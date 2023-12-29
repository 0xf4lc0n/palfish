#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include "blowfish.h"
#include "io.h"


void print_arr(char *array, int size) {
    for (int i = 0; i < size; i++) {
        printf("%c ", array[i]);

    }
    printf("\n");
}

int main(int argc, char** argv) {
    if (argc != 5) {
        return 1;
    }

    char *key_file = argv[1];
    char key_size = atoi(argv[2]);
    char *file_to_encrpyt = argv[3];
    int file_size, input_len;

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    uint8_t *key = (uint8_t *)read_file_chunk(key_file, key_size);
    // Initialize Blowfish with the key
    key_expansion(key, 16);

    char *array = NULL;
    if (rank == 0) {
        array = (char*)malloc(file_size * sizeof(char));
        file_size = read_entire_file(file_to_encrpyt, &array);
        input_len = file_size;
               printf("File size2222: %d\n", strlen(array));
        if (is_padding_needed(array)) {
            array = add_padding(array, &input_len);
        }

        printf("File size: %d\n", file_size);
        printf("Input len: %d\n", input_len);
        printf("Orginial array: ");
        print_in_hex(array, input_len);
    }

    MPI_Bcast(&input_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int chunk_size = input_len / size;
    char *local_array = (char*)malloc(chunk_size * sizeof(char));
    MPI_Scatter(array, chunk_size, MPI_CHAR, local_array, chunk_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    char *new_array = (char*)malloc(chunk_size * sizeof(char));
    blowfish_encrypt_string(local_array, chunk_size, new_array);

    if (rank == 0) {
        char *modified_array = (char*)malloc(input_len * sizeof(char));
        MPI_Gather(new_array, chunk_size, MPI_CHAR, modified_array, chunk_size, MPI_CHAR, 0, MPI_COMM_WORLD);

        printf("Modified Array: ");
        print_in_hex(modified_array, input_len);

        int err = save_to_file("mpi_result.txt", modified_array, file_size);
        if (err != 0) {
            fprintf(stderr, "Failed to save message to the file\n");
            return -1;
        }

        free(modified_array);
        free(array);
    } else {
        MPI_Gather(new_array, chunk_size, MPI_CHAR, NULL, 0, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    free(local_array);
    free(new_array);

    MPI_Finalize();
    return 0;
}
