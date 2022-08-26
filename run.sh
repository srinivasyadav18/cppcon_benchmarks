echo $CPU_FAMILY
PROJECT_ROOT=$(pwd)
BUILD_DIR=$PROJECT_ROOT/build

BENCHMARKS_RESULT_DIR=$PROJECT_ROOT/benchmarks_results/$CPU_FAMILY
mkdir -p $BENCHMARKS_RESULT_DIR

rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

git clone https://github.com/STEllAR-GROUP/hpx.git

# Load cmake, ninja, gcc 11, hwloc, tcmalloc/gperftools, boost
module load gcc/11.2.1 hwloc gperftools boost/1.78.0-release

# Edit hpx configuration as needed
cmake -GNinja -S hpx -B hpx_build \
    -DHPX_WITH_MALLOC=tcmalloc \
    -DCMAKE_BUILD_TYPE=Release \
    -DHPX_WITH_FETCH_ASIO=ON \
    -DHPX_WITH_COMPILER_WARNINGS=On \
    -DHPX_WITH_COMPILER_WARNINGS_AS_ERRORS=On \
    -DHPX_WITH_CXX_STANDARD=20 \
    -DHPX_WITH_EXAMPLES=OFF \
    -DHPX_WITH_DATAPAR_BACKEND=STD_EXPERIMENTAL_SIMD

# Build HPX
cd hpx_build && ninja && cd ..

HPX_DIR=$(pwd)/hpx_build cmake -GNinja -S .. -B benchmarks -DCMAKE_BUILD_TYPE=Release -DSIMD_END=25

cd benchmarks
ninja
ctest
python3 $PROJECT_ROOT/generate_plots.py
python3 $PROJECT_ROOT/generate_plot.py
cd ..
cp -r benchmarks/* $BENCHMARKS_RESULT_DIR/
