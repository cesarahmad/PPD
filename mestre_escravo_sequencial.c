//----------------------------Cabeçalho------------------------------------------
//	Nome: mestre_escravo_sequencial.c
//	Autor: Clóvis Rogério Paes Bitencourt Júnior, Miguel Cardoso Abreu.
//	E-mail: clovis.bitencourt@rstech.com.br, miguel.abreu@acad.pucrs.br 
//	Modificado em: 12.03.2015 - 18:10
//	Versão: final
//
// Resumo: trata da implementação do algoritimo Rank Sort no modelo sequencial para comparativo com o modelo mestre e escravo
//
// Para execução desse trabalho: ladrun -np <numero_execucoes> mestre_escravo_sequencial <numero_elementos>  
//---------------------------Includes----------------------------------------
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
//---------------------------Defines-----------------------------------------

//---------------------------Funções-----------------------------------------
// Função: Lê Arquivo.
void le_arquivo(int *vetor_lido,int TAM)
{
	int i;
	FILE *fp;
 	fp = fopen("elementos.txt", "r");
	
	if(fp == NULL)
 	{
		puts("Erro na abertura de arquivo");
		exit(EXIT_FAILURE);
	}
	for(i=0; i<TAM; i++) 
	{ 
		vetor_lido[i] = 0; 
		fscanf(fp, "%d", &vetor_lido[i]); 
  	}
	fclose(fp);
}

// Função: Imprimi o tempo.
void imprime_tempo(struct timeval ti, struct timeval tf)
{
	int secs, msecs, usecs;
	
	msecs = (int) (tf.tv_usec-ti.tv_usec)/1000;
	usecs = (int) (tf.tv_usec-ti.tv_usec)%1000;
	secs = (int) (tf.tv_sec - ti.tv_sec);
           
	if (msecs < 0)
	{
		secs--;
		msecs = 1000+msecs;
	}
	if (usecs <0)
	{
		msecs--;
		usecs = 1000+usecs;
	}
	printf("Tempo Gasto: %ds%dms%dus\n", secs, msecs, usecs);
}

// Função: Salva vetor ordenado em arquivo.
void salva_txt(int *vetor_ordenado, int TAM)
{
	int i;
	FILE *arq;

	remove("ordenados.txt"); // Remove arquivo antigo.

	//Imprime novo arquivo
	arq = fopen ("ordenados.txt", "a+");
	for (i = 0; i < TAM; i++)
		fprintf(arq, "%d\n", vetor_ordenado[i]);
	fflush(arq);
	fclose(arq);
}


int main(int argc, char **argv) { 
	int TAM = atoi(argv[1]); // TAM será o tamanho de elementos a ordenar

	//Validacao do numero de elementos a ordenar
	if (TAM < 1)
	{
		printf("Erro: Numero de elementos tem que ser maior que 0. \n");
		return (1);
	}
	
	int size, rank, tag=0;					// Variávis do MPI
	struct timeval ti, tf; 					// Para imprimir tempo
	int vetor_lido[TAM]; 					// Vetor que vai ser lido
	int vetor_ordenado[TAM]; 				// Vetor que vai ser ordenado
	int i, x, j;						// Auxiliares para for

	le_arquivo(vetor_lido, TAM); 				// Recebe os valores do arquivo texto e armazena nos vetores

	gettimeofday(&ti, NULL); 				//Tempo Inicial

	// Algoritmo Rank Sort
	for (i = 0; i < TAM; i++) { 
		x = 0;
		for (j = 0; j < TAM; j++) 
			if (vetor_lido[i] > vetor_lido[j]) 
				x++;
			vetor_ordenado[x] = vetor_lido[i]; 
	}

	gettimeofday(&tf, NULL); 				//Tempo Final

	imprime_tempo(ti, tf); 	//Exibe o tempo total de execução e imprime o resultado final em arquivo 
	
	return(0); 
} 
