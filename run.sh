set -xe

cmake -S . -B build
cmake --build build --parallel
./build/DZMKII
