###  Example Invocation

```
CWD=`pwd`
srun --mpi=pmi2 singularity exec -B $I_MPI_ROOT:/intel-mpi -B /projects:/projects ~/.containers/scool-examples.simg bash -c "export TAU_PROFILE=1; export TAU_TRACE=1; export PROFILEDIR=\"$CWD\"; tau_exec /SCoOL/examples/bin/qap_mpi /SCoOL/examples/data/qap/Nug6.dat"
singularity exec -B $I_MPI_ROOT:/intel-mpi -B /projects:/projects ~/.containers/scool-examples.simg bash -c "tau_treemerge.pl; tau2slog2 tau.trc tau.edf -o tau.slog2"
```
