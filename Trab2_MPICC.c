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
/*
	É NECESSARIO UTILIZAR IF, ELSE POIS O RANK 0 SERÁ INICIADO COM A MATRIZ COMPLETA
*/


	// MESTRE
/*
	O RANK 0 TERA A MATRIZ COMPLETA E PASSARÁ EM DIANTE O QUE NÃO FOR UTILIZAR
*/
	if(rank == 0)
	{

	} 
	// ESCRAVO
/*
	RECEBERA TO RANK-1 ENTERIOR E PASSARÁ PARA O RANK+1 POSTERIOR, A MENOS QUE SEJA O ULTIMO
*/
	else
	{
		e
	}
	MPI_Finalize();
	return 0;
}

// START PROGRAMMING FUNCTIONS 

// ...

// END PROGRAMMING FUNCTIONS
