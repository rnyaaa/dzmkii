set -xe

cmake -S . -B build
cmake --build build
./build/DZMKII
