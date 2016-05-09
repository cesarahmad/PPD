#include <stdio.h>
#include <mpi.h>

// START NAME FUNCTIONS 

// ...

// END NAME FUNCTIONS

int main (int argc, char **argv)
{
	int size, rank;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
		// .......
	printf("hello world, process %d of %d\n", rank, size);

	// MESTRE
	if(rank == 0)
	{

	} 
	// ESCRAVO
	else
	{

	}
	MPI_Finalize();
	return 0;
}

// START PROGRAMMING FUNCTIONS 

// ...

// END PROGRAMMING FUNCTIONS
