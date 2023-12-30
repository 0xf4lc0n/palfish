#/bin/sh

count_unique_file_hashes() {
    echo $(sha1sum $1 | cut -d " " -f1 | sort | uniq | wc -l)
}

if [ -z $4 ]; then
    echo "Usage: ./test.sh key_size(in bytes) file_size(in megabytes) max_num_of_openmp_threads max_num_of_mpi_processes" && exit
fi

key_size="$1"
file_size="$2"
max_num_of_openmp_threads="$3"
max_num_of_mpi_processes="$4"

current_dir="$(pwd)"
tmp_dir="$current_dir/tmp"
src_dir="$current_dir/src"
target_dir="$current_dir/target"
obj_dir="$current_dir/obj"

blowfish="$target_dir/blowfish"
blowfish_mpi="$target_dir/blowfish_mpi"

key_file="$tmp_dir/key.txt"
file="$tmp_dir/file.txt"
encrypted_file_mpi="$tmp_dir/encrypted_mpi.txt"
decrypted_file_mpi="$tmp_dir/decrypted_mpi.txt"

make clean &> /dev/null
make &> /dev/null
mpicc "$src_dir/mpi_impl.c" -o "$blowfish_mpi" "$obj_dir/blowfish.o" "$obj_dir/io.o" -fopenmp  &> /dev/null
chmod +x "$blowfish"
chmod +x "$blowfish_mpi"

if [ $? -ne 0 ]; then
    echo "Cannot compile project" && exit
fi

mkdir -p "$tmp_dir"
rm -rf "$tmp_dir/"*

./generate.sh key "$key_size" > "$key_file"
./generate.sh file "$file_size" > "$file"

# test sequential implementation
"$blowfish" "$key_file" "$key_size" "$file" 0 0

# test openmp implementation
for num_of_threads in $(seq 2 $max_num_of_openmp_threads); do
    echo "OpenMP implementation (num_of_threads: $num_of_threads)"
    "$blowfish" "$key_file" "$key_size" "$file" 1 "$num_of_threads"
done

# test mpi implementation
for num_of_processes in $(seq 2 $max_num_of_mpi_processes); do
    echo -n "MPI encryption (num_of_processes: $num_of_processes): "
    mpirun -np $num_of_processes "$blowfish_mpi" "$key_file" "$key_size" "$file" "$encrypted_file_mpi" 0 0
    echo -n "MPI decryption (num_of_processes: $num_of_processes): "
    mpirun -np $num_of_processes "$blowfish_mpi" "$key_file" "$key_size" "$encrypted_file_mpi" "$decrypted_file_mpi" 1 0

    if [ $(count_unique_file_hashes "$tmp_dir/encrypted*") -ne 1 ]; then
    echo "Not all encrypted files are valid" && exit
    fi

    if [ $(count_unique_file_hashes "$tmp_dir/decrypted* $file") -ne 1 ]; then
        echo "Not all decrypted files are valid" && exit
    fi
done

# test hybrid implementation
for num_of_processes in $(seq 2 $max_num_of_mpi_processes); do
    for num_of_threads in $(seq 2 $max_num_of_openmp_threads); do
        echo -n "Hybrid encryption (num_of_processes: $num_of_processes, num_of_threads: $num_of_threads): "
        mpirun -np $num_of_processes "$blowfish_mpi" "$key_file" "$key_size" "$file" "$encrypted_file_mpi" 0 $num_of_threads
        echo -n "Hybrid decryption (num_of_processes: $num_of_processes, num_of_threads: $num_of_threads): "
        mpirun -np $num_of_processes "$blowfish_mpi" "$key_file" "$key_size" "$encrypted_file_mpi" "$decrypted_file_mpi" 1 $num_of_threads

        if [ $(count_unique_file_hashes "$tmp_dir/encrypted*") -ne 1 ]; then
            echo "Not all encrypted files are valid" && exit
        fi

        if [ $(count_unique_file_hashes "$tmp_dir/decrypted* $file") -ne 1 ]; then
            echo "Not all decrypted files are valid" && exit
        fi
    done
done


