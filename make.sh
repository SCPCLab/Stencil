rm -rf *.o benchmark-optmizied 
sw5cc -host -I/usr/sw-mpp/mpi2/include -Wall -std=c99 -OPT:IEEE_arith=1  -c  benchmark.c  
sw5cc -host -I/usr/sw-mpp/mpi2/include -Wall -std=c99 -OPT:IEEE_arith=1 -c check.c   
sw5cc -host -I/usr/sw-mpp/mpi2/include -Wall -std=c99 -OPT:IEEE_arith=1 -c stencil-optimized.c  
sw5cc -slave -Wall -std=c99 -OPT:IEEE_arith=1 -c stencil-slave-7.c   
mpicc -hybrid stencil-slave-7.o  benchmark.o check.o stencil-optimized.o -o benchmark-optimized 

