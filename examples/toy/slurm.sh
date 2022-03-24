#!/bin/bash

#SBATCH --clusters=faculty
#SBATCH --qos=planex
#SBATCH --partition=planex
#SBATCH --account=cse603
#SBATCH --exclusive
#SBATCH --mem=64000


#SBATCH --output=%j.stdout
#SBATCH --error=%j.stderr

#SBATCH --job-name="OMP_Executor_Test"
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=20
#SBATCH --time=04:00:00

hostname

module load gcc

echo "t1 mildew "

OMP_NUM_THREADS=40 ./bnsl_t1 35 ../data/bnsl/mildew.35x160K.mps
OMP_NUM_THREADS=32 ./bnsl_t1 35 ../data/bnsl/mildew.35x160K.mps
OMP_NUM_THREADS=20 ./bnsl_t1 35 ../data/bnsl/mildew.35x160K.mps
OMP_NUM_THREADS=16 ./bnsl_t1 35 ../data/bnsl/mildew.35x160K.mps


