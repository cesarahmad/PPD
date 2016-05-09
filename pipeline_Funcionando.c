//----------------------------Cabeçalho------------------------------------------
//	Nome: pipeline.c
// Nome: Fabiano Nauter; Pedro Filippi
// Resumo: trata da implementação do algoritimo Insertion Sort no modelo pipeline. Foram atribuído os estagios do pipe como rank, ou seja,
// estágio 0 - rank 0, estagio 1 - rank 1 e assim sucessivamente. Estágio ordena elementos até TAM/size (até encher o vetor do estágio)
// após isso ele envia maior valor ao pŕoximo estágio
//
// Para execução desse trabalho: ladrun -np <numero_processos> pipeline <nomedo arquivo> <numero_elementos>
//---------------------------Includes----------------------------------------
#include <mpi.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
//---------------------------Defines-----------------------------------------
//---------------------------Funções-----------------------------------------

// Função: Lê Arquivo.
void le_arquivo(int *vetor_lido,char arquivo_entrada[100], int TAM)
{
    int i;
    FILE *fp;
    fp = fopen(arquivo_entrada, "r");

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
void imprime_estagio(int rank, int *vetor_ordenado, int tam)
{
    int i;
    FILE *arq;

    //Imprime novo arquivo
    arq = fopen ("saida.txt", "a+");
    fprintf(arq, "Estágio: %d\n", rank);
    for (i = rank*tam; i < (rank*tam + tam); i++)
    {
        fprintf(arq, "%d\n", vetor_ordenado[i]);
    }
    fflush(arq);
    fclose(arq);
}

// Função que compara, armazena e envia (se for o caso) valores no vetor
void compara_envia(int id, int valor_recebido, int *vetor_ordenado, int posicao, int tam)
{
    int j;															// Utilizado nos loops
    int tag = 0;													// Utilizado na comunicação MPI
    int maior_valor;												// Valor enviado caso o valor recebido seja menor
    int pos_final;													// Posição final a ser lida
    int aux;														// Variavel auxiliar da ordenacao
    int inseriu;													// Verifica se foi inserido no vetor


    maior_valor = valor_recebido;									// Inicializa o maior valor como o valor recebido

    // Se o estágio não estiver cheio, não é necessário ler todas as posições do estágio
    if (posicao < (id*tam + tam))
        pos_final = posicao;
    else
        pos_final = (id*tam + tam);

    //Insertion Sort atribuindo somente ao vetor do estagio
    for (j = id*tam; j < pos_final; j++)
    {
        if (maior_valor < vetor_ordenado[j])
        {
            aux = vetor_ordenado[j];
            vetor_ordenado[j] = maior_valor;
            maior_valor = aux;
        }
    }

    if (posicao < (id*tam + tam))									// Caso nao tenha preenchido todas as posições do vetor estagio
    {
        vetor_ordenado[posicao] = maior_valor;						// Caso vetor do estagio nao tenha sido preenchido ainda adiciona maior valor no final do vetor do estagio
    }
    else 															// Caso já tenha preenchido todas as posições do vetor
    {
        MPI_Send(&maior_valor,1,MPI_INT,id+1,tag,MPI_COMM_WORLD);	// Manda a variável ultimo para o próximo, que contém a última posição
    }
}

//----------------------------------Main-------------------------------------
int main(int argc, char **argv)
{
    int TAM = atoi(argv[2]); // TAM será o tamanho de elementos a ordenar

    //Validacao do numero de elementos a ordenar
    if (TAM < 1)
    {
        printf("Erro: Numero de elementos tem que ser maior que 0. \n");
        return (1);
    }

    int size, rank, tag=0;											// Variáveis do MPI
    struct timeval ti, tf; 											// Para imprimir tempo
    int vetor_lido[TAM]; 											// Vetor que vai ser lido
    int vetor_ordenado[TAM]; 										// Vetor que vai ser ordenado
    int valor_recebido; 											// Valor recebido no primeiro estagio
    int terminou_estagio = -1; 										// Determina que etapa leu todos os valores
    int i; 													// Utilizado nos loops
    int tamanho_estagio;											// Tamanho do estagio
    int terminou_estagio_anterior;										// Indica que estágio anterior terminou
    int imprime = 1; 											// Pode imprimir (sem valor lógico, apenas demonstrativo)

    // Limpar lixo de memória do vetor ordenado
    for (i=0; i<TAM; i++)
    {
        vetor_ordenado[i] = 0;
    }

    // Inicialização MPI
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 									// Numero do processo.
    MPI_Comm_size(MPI_COMM_WORLD, &size); 									// Numero total de processos(Processadores).

    // Validacao para que numero de elementos e numero de processos sejam divisiveis
    if (TAM%size != 0) 											//Caso não dê número inteiro, termina o programa
    {
        printf("Razao entre numero de elementos e numero de processos deve ser divisivel \n");
        return 1;
    }

    tamanho_estagio = (TAM / size); 									// Tamanho de cada estagio

     le_arquivo(vetor_lido,argv[1],TAM);                                                                     // Recebe os $
        remove("saida.txt");                                                                    // Remove arquivo antigo.

        gettimeofday(&ti, NULL);                                                                        // Tempo inicial

    //Implementacao do pipeline
    if(rank == 0)
    {

        for (i = 0; i < TAM; i++) 									// Percorre todos os valores do vetor lido
        {
            valor_recebido = vetor_lido[i]; 							// Lê o valor da posição

            compara_envia(rank, valor_recebido, vetor_ordenado, i, tamanho_estagio);		// Atribui o valor recebido no estagio ou manda para o proximo
        }

        if (size > 1)											// Valida se tem mais de um processo
            MPI_Send(&terminou_estagio, 1, MPI_INT, rank+1, tag, MPI_COMM_WORLD); 			// Envia para o próximo que pode terminar
    }
    else
    {
        for (i=rank*tamanho_estagio; i<=TAM; i++)							// Somente vai ler o que interessa ao estagio
        {
            MPI_Recv(&valor_recebido,1,MPI_INT,rank-1,tag,MPI_COMM_WORLD,&status); 			// Recebe do anterior o valor da função Compara

            if (valor_recebido == -1) 								// O valor_recebido -1 eh uma ordem para finalizar
            {
                terminou_estagio_anterior = 1;
            }
            else
                compara_envia(rank, valor_recebido, vetor_ordenado, i, tamanho_estagio);	// Compara valor recebido pelo estagio anterior
        }

        if (terminou_estagio_anterior == 1)
        {
            if (rank+1 < size) 								// Se não é o último processador
                MPI_Send(&terminou_estagio, 1, MPI_INT, rank+1, tag, MPI_COMM_WORLD); 	// Envia a mensagem de fim para o próximo
            else
                MPI_Send(&terminou_estagio, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);	// Envia a mensagem de fim para o primeiro estagio
        }
    }

    //Impressao do tempo de execucao
    if(rank == 0) 												// Primeiro estagio
    {
        if (size > 1)											// Valida se tem mais de um processo
        {
            MPI_Recv(&valor_recebido,1,MPI_INT,size-1,tag,MPI_COMM_WORLD,&status);  		// Espera o ultimo estagio terminar
            if (valor_recebido == -1)								// O valor_recebido -1 eh uma ordem para finalizar
            {
                gettimeofday(&tf, NULL); 							// Pega o tempo final
                imprime_tempo(ti, tf); 								// Exibe o tempo total de execução e imprime o resultado final
            }
        }
        else
        {
            gettimeofday(&tf, NULL); 							// Pega o tempo final
            imprime_tempo(ti, tf); 								// Exibe o tempo total de execução e imprime o resultado final
        }
    }



    MPI_Barrier(MPI_COMM_WORLD); 										// Espera todos os processadores chegar até aqui

    //Impressao dos estagios
    if (rank > 0) 												//Se nao for o primeiro estagio
        MPI_Recv(&imprime,1,MPI_INT,rank-1,tag,MPI_COMM_WORLD,&status); 				// Recebe do anterior que pode imprimir seus valores

    imprime_estagio(rank, vetor_ordenado, tamanho_estagio);							// Imprime o estagio atual sem a faixa

    if (rank+1 < size) 											// Senão for o último processador
        MPI_Send(&imprime, 1, MPI_INT, rank+1, tag, MPI_COMM_WORLD); 					// Manda para o próximo que pode imprimir

    MPI_Barrier(MPI_COMM_WORLD); 										// Espera todos chegarem até aqui

    MPI_Finalize();
    return 0;
}





