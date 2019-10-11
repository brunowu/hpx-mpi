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
module load CUDA/10.1.105
```

#### Configuration

```bash
cmake ../.. -DCMAKE_MODULE_PATH=/p/project/cslai/wu/source/hpx_1.3.0 -DCMAKE_C_COMPILER=/usr/local/software/jureca/Stages/2019a/software/GCCcore/8.3.0/bin/gcc -DCMAKE_CXX_COMPILER=/usr/local/software/jureca/Stages/2019a/software/GCCcore/8.3.0/bin/g++ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/p/project/cslai/wu/source/hpx_1.3.0/build/Release-mpi -DHWLOC_ROOT=/usr/local/software/jureca/Stages/2019a/software/hwloc/2.0.3-GCCcore-8.3.0 -DHPX_WITH_MALLOC=jemalloc -DHPX_WITH_TESTS=OFF -DHPX_WITH_EXAMPLES=OFF -DHPX_WITH_THREAD_IDLE_RATES=ON -DHPX_WITH_PARCELPORT_MPI=ON 
```

### Compile and run 

#### Compile
```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/p/project/cslai/wu/source/hpx_1.3.0/build/Release-mpi -DCMAKE_BUILD_TYPE=Release
```
#### Run
```bash
srun -n 4 ./hello-world/hello.exe
```

#### Output
```bash
Rank (0) : hello, I'm 0 out of 4 procs !
```

