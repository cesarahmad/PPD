//----------------------------Cabeçalho------------------------------------------
//	Nome: mestre_escravo.c
//	Autor: Clóvis Rogério Paes Bitencourt Júnior, Miguel Cardoso Abreu.
//	E-mail: clovis.bitencourt@rstech.com.br, miguel.abreu@acad.pucrs.br 
//	Modificado em: 12.03.2015 - 18:10
//	Versão: final
//
// Resumo: trata da implementação do algoritimo Rank Sort no modelo mestre e escravo. Foi atribuído o mestre como se fosse rank 0 e escravos demais os
// processos, onde o mestre somente separa os pedaços e os junta exibindo o resultado final enquanto o escravo somente ordena um pedaço e devolve ao mestre.
// O programa executará o seguinte número de tarefas <numero_elementos>/4*<numero_escravos>, onde o número de escravos é o <numero_processos> - 1
//
// Para execução desse trabalho: ladrun -np <numero_processos> mestre_escravo <numero_elementos>  
//---------------------------Includes----------------------------------------
#include <mpi.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
//---------------------------Defines-----------------------------------------

//---------------------------Funções-----------------------------------------
// Função: Merge (Prof. Avelino)
void merge(int array[], int begin, int mid, int end) 
{
	int ib = begin;
	int im = mid;
	int j;
	int size = end-begin;
	int b[size];

	// Enquanto existirem elementos na lista da esquerda ou direita
	for (j = 0; j < (size); j++)
	{
		if (ib < mid && (im >= end || array[ib] <= array[im]))
		{
			b[j] = array[ib];
			ib = ib + 1;
        	}
        	else
		{
			b[j] = array[im];
			im = im + 1;
		}
	}
	for (j=0, ib=begin; ib<end; j++, ib++) array[ib] = b[j];
}

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


//----------------------------------Main-------------------------------------
int main(int argc, char **argv)
{
	int TAM = atoi(argv[1]); // TAM será o tamanho de elementos a ordenar

	//Validacao do numero de elementos a ordenar
	if (TAM < 1)
	{
		printf("Erro: Numero de elementos tem que ser maior que 0. \n");
		return (1);
	}
	
	int size, rank, tag=0;			// Variávis do MPI
	struct timeval ti, tf; 			// Para imprimir tempo
	int vetor_lido[TAM]; 			// Vetor que vai ser lido
	int vetor_ordenado[TAM]; 		// Vetor que vai ser ordenado
	int numero_escravos;			// Numero de escravos
	int numero_tarefas;			// Numero de tarefas
	int tamanho_tarefa;			// Tamanho a ser ordenado por cada escravo
	int posicao_inicial=0; 			// Inicio do pedaco a ser ordenado
	int posicao_meio=0;			// Posição do meio passada para a função merge
	int posicao_final=0; 			// Fim do pedaco a ser ordenado
	int cont=0;				// Controle do vetor principal
	int escravo;				// rank recebido
	int send=0;				// Quantidade de vezes que foi enviado para escravo
	int receive=0;				// Quantidade de vezes que retornou para o mestre
	int i, j;				// Auxiliares para for

	//Inicialização MPI
	MPI_Status status;	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 			// Numero do processo.
	MPI_Comm_size(MPI_COMM_WORLD, &size); 			// Numero total de processos(Processadores).	

	numero_escravos	= size - 1;
	numero_tarefas = 4*numero_escravos;
	tamanho_tarefa 	= TAM / numero_tarefas; 		// Tamanho a ser ordenado para cada escravo (np-1)

	int vetor_escravo[tamanho_tarefa]; 			// Vetor que vai ser recebido pelo mestre

	if( ((TAM%tamanho_tarefa)!=0) || (TAM<numero_tarefas) )	//Validação se numero de elementos é divisivel com 4*numero_escravos
	{
		printf("Erro: Numero de elementos não é divisivel com 4*numero_escravos. \n");
		return (1);
	}

	le_arquivo(vetor_lido, TAM); 				// Recebe os valores do arquivo texto e armazena nos vetores

	//Mestre
	if (rank == 0)
	{
		gettimeofday(&ti, NULL); 										// Início da contagem do tempo

		// Envia uma vez pra todos os escravos
		for(escravo=1;escravo<=numero_escravos;escravo++)
		{

			posicao_inicial = (tamanho_tarefa * cont);							//Início do pedaco do vetor    

			MPI_Send(&posicao_inicial, 1, MPI_INT, escravo, tag, MPI_COMM_WORLD);				//Envia início do pedaco do vetor

			cont++;
		}	
		
		send=cont; 												// Guardando quantas tarefas foram enviadas

		// Mestre recebe valores ordenados pelo escravo e os armazena no vetor_ordenado
        	while (numero_tarefas > 0)
		{	
			//Receive
			MPI_Recv(vetor_escravo, tamanho_tarefa, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status); // Recebe vetor de um escravo
	        	escravo = status.MPI_SOURCE; 									// Identifica de que escravo ele recebeu	

			posicao_meio=receive*tamanho_tarefa; 								// Primeira posição do vetor recebido do escravo
			receive++;											// Acumula número de tarefas finalizadas

			// Recebe do escravo e coloca no vetor ordenado
			for(i=0;i<tamanho_tarefa;i++) 
			{
				vetor_ordenado[(posicao_meio+i)] = vetor_escravo[i];
			}

			if(receive>1)
				merge(vetor_ordenado, 0, posicao_meio, (posicao_meio + tamanho_tarefa) ); 		//Compara resultado ordenado com o restante do vetor
			
							
			
			//Verifica se já enviou todas as tarefas
			if(send < numero_tarefas)
			{		
				posicao_inicial = (tamanho_tarefa * cont);						//Início do pedaco do vetor 
	       				
		    		MPI_Send(&posicao_inicial, 1, MPI_INT, escravo, tag, MPI_COMM_WORLD);			//Envia início do pedaco do vetor

				cont++;
			}	 

			numero_tarefas--;  										//Se chegar a zero para de receber		
		}


		// Enviando valor -1 para todos os escravos para encerra-los
		posicao_inicial = -1;
	    	for (escravo=1; escravo<=numero_escravos; escravo++) 
		{
	        	MPI_Send(&posicao_inicial, 1, MPI_INT, escravo, tag, MPI_COMM_WORLD);
	    	}

		gettimeofday(&tf, NULL); 		// Final da contagem do tempo

		imprime_tempo(ti, tf); 			// Imprime tempo

		salva_txt(vetor_ordenado, TAM); 	// Salva vetor ordenado em arquivo
	}
	else // Escravo
	{
		int terminou = 0; 	// Indica que programa terminou
		int x=0; 			// Auxiliar para algoritimo Rank Sort
		// Fica trabalhando enquanto o mestre não terminar        
		while (terminou==0) 
		{ 
			
		 	MPI_Recv(&posicao_inicial, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status); 		// Receber do mestre posição inicial do vetor lido à ordenar

			if (posicao_inicial < 0) 								// Mestre terminou (escravo recebeu -1)
				terminou = 1;
		    	else // Ainda tem trabalho
			{ 
		    		posicao_final = posicao_inicial + tamanho_tarefa - 1; 				// Calcula a posição final pelo tamanho da tarefa

				// Ordenação do vetor para cada número do intervalo pelo algoritimo Rank Sort
				for (i = posicao_inicial; i <= posicao_final; i++) 
				{  
					x = 0;
					for (j = posicao_inicial; j <= posicao_final; j++) 			//Conta quantos números são menores que ele
						if (vetor_lido[i] > vetor_lido[j]) 
							x++;
					vetor_escravo[x] = vetor_lido[i]; 					//Copia no lugar correto do pedaço enviado
				}
				
				MPI_Send(vetor_escravo, tamanho_tarefa, MPI_INT, 0, tag, MPI_COMM_WORLD); 	// Envia vetor escravo e seu tamanho
				

			}
		}
	}		

	MPI_Finalize();
	return 0;
}





