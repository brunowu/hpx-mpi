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

#### Bcast example

```bash
srun -n 8 ./Bcast/Bcast_mpi.exe --size=10 --r=500

srun -n 8  ./Bcast/Bcast_hpx_mpi.exe --size=10 --r=500 --hpx:run-hpx-main

srun -n 8 ./Bcast/Bcast_hpx_mpi_fut.exe --size=10 --r=500 --hpx:run-hpx-main

srun -n 8 ./Bcast/Bcast_hpx_mpi_pool.exe --size=10 --r=500 --hpx:run-hpx-main --hpx:threads=2 --hpx:ignore-batch-env
```


For the mixture of HPX and MPI, where HPX runs only in node-level, it manages the computation and communication using different HPX threads.

By default HPX runs hpx-main on rank 0 only. The other ranks wait for actions to be spawned from other ranks. Thus it is necessary to tell HPX to run main from all the ranks with the option `--hpx:run-hpx-main`.
