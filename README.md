# hpx-mpi

## Configuration of HPX

### Configuration on JURECA ([info](https://www.fz-juelich.de/ias/jsc/EN/Expertise/Supercomputers/JURECA/Configuration/Configuration_node.html))

#### HPX version
HPX 1.3.0 stable

#### Module loaded
```bash
module load GCC
module load ParaStationMPI/5.2.2-1
module load hwloc/2.0.3
module load jemalloc
module load CMake/3.14.0
module load Boost/1.69.0-Python-2.7.16
```

#### Configuration

```bash
cmake ../.. -DCMAKE_MODULE_PATH=/p/project/cslai/wu/source/hpx_1.3.0 \
	    -DCMAKE_C_COMPILER=/usr/local/software/jureca/Stages/2019a/software/GCCcore/8.3.0/bin/gcc \
	    -DCMAKE_CXX_COMPILER=/usr/local/software/jureca/Stages/2019a/software/GCCcore/8.3.0/bin/g++ \
	    -DCMAKE_BUILD_TYPE=Release \
	    -DCMAKE_INSTALL_PREFIX=/p/project/cslai/wu/source/hpx_1.3.0/build/Release-mpi \
	    -DHWLOC_ROOT=/usr/local/software/jureca/Stages/2019a/software/hwloc/2.0.3-GCCcore-8.3.0 \
            -DHPX_WITH_MALLOC=jemalloc \
	    -DHPX_WITH_TESTS=OFF \
	    -DHPX_WITH_EXAMPLES=OFF \
	    -DHPX_WITH_THREAD_IDLE_RATES=ON \
	    -DHPX_WITH_PARCELPORT_MPI=ON 
```

### Compile and run 

#### Compile
```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/p/project/cslai/wu/source/hpx_1.3.0/build/Release-mpi -DCMAKE_BUILD_TYPE=Release
```
#### (Correct) Run
```bash
srun -n 4 ./hello-world/hello.exe --hpx:run-hpx-main
```

#### SUMMA Prototype

MPI version:

```bash
srun -N ${PROCS} -n ${PROCS} ./SUMMA/mpi/summa_mpi_bb.exe --s=${SIZE} --b=${BLOCKSIZE}
```

HPX+MPI version:

```bash
srun -N ${PROCS} -n ${PROCS} ./SUMMA/hpx-mpi/summa_hpx_mpi_bb.exe --s=${SIZE} --b=${BLOCKSIZE} --hpx:run-hpx-main --hpx:ignore-batch-env --hpx:threads=${THREADS_NUM}
```

Remainder:

1. The execution of SUMMA prototype requires one MPI proc per node; 

2. The current implementation of SUMMA requires that `${SIZE}` should be divisible by `${BLOCKSIZE}`.



