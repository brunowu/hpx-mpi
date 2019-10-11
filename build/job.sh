#!/bin/bash -x
#SBATCH --account=slai
#SBATCH --nodes=1
#SBATCH --ntasks=4
#SBATCH --output=mpi-out.%j
#SBATCH --error=mpi-err.%j
#SBATCH --time=00:15:00
#SBATCH --partition=devel

srun -n 4 ./hello-world/hello.exe

