/*
 * testa_scatter.c
 * Programa para testar as minhas implementações do MPI_Scatter e MPI_Scatterv
 * 
 * Teste 1: meu_MPI_Scatter - distribui blocos iguais
 * Teste 2: meu_MPI_Scatterv - distribui blocos diferentes
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Incluo as minhas implementações
// (na prática, poderia usar header files, mas vou colocar o código aqui para simplificar)

// ===== MINHA IMPLEMENTAÇÃO DO SCATTER =====
void meu_MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                     void *recvbuf, int recvcount, MPI_Datatype recvtype,
                     int root, MPI_Comm comm) {
    
    int id, p;
    MPI_Comm_rank(comm, &id);
    MPI_Comm_size(comm, &p);
    
    if(id == root) {
        for(int i = 0; i < p; i++) {
            int deslocamento = i * sendcount;
            char *ptr_envio = (char*)sendbuf + deslocamento * sizeof(int);
            
            if(i == root) {
                for(int j = 0; j < sendcount; j++) {
                    ((int*)recvbuf)[j] = ((int*)sendbuf)[deslocamento + j];
                }
            } else {
                MPI_Send(ptr_envio, sendcount, sendtype, i, 0, comm);
            }
        }
    } else {
        MPI_Recv(recvbuf, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
    }
}

// ===== MINHA IMPLEMENTAÇÃO DO SCATTERV =====
void meu_MPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                      MPI_Datatype sendtype, void *recvbuf, int recvcount,
                      MPI_Datatype recvtype, int root, MPI_Comm comm) {
    
    int id, p;
    MPI_Comm_rank(comm, &id);
    MPI_Comm_size(comm, &p);
    
    if(id == root) {
        for(int i = 0; i < p; i++) {
            if(sendcounts[i] > 0) {
                int deslocamento = displs[i];
                char *ptr_envio = (char*)sendbuf + deslocamento * sizeof(int);
                
                if(i == root) {
                    for(int j = 0; j < recvcount; j++) {
                        ((int*)recvbuf)[j] = ((int*)sendbuf)[deslocamento + j];
                    }
                } else {
                    MPI_Send(ptr_envio, sendcounts[i], sendtype, i, 0, comm);
                }
            }
        }
    } else {
        MPI_Recv(recvbuf, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
    }
}

// ===== PROGRAMA PRINCIPAL =====
int main(int argc, char *argv[]) {
    
    MPI_Init(&argc, &argv);
    
    int id, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    
    // Verifico se tenho 4 processos (necessário para os testes)
    if(p != 4) {
        if(id == 0) {
            printf("Este programa precisa de 4 processos.\n");
            printf("Execute: mpirun -np 4 --oversubscribe ./testa_scatter\n");
        }
        MPI_Finalize();
        return 0;
    }
    
    // ============================================
    // TESTE 1: meu_MPI_Scatter (blocos iguais)
    // ============================================
    
    int dados_originais[12];
    int dados_recebidos[3];  // cada processo recebe 3 elementos
    
    if(id == 0) {
        printf("\n========== TESTE 1: meu_MPI_Scatter ==========\n");
        printf("Dados originais (apenas o processo 0 tem): ");
        for(int i = 0; i < 12; i++) {
            dados_originais[i] = i * 10;  // 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110
            printf("%d ", dados_originais[i]);
        }
        printf("\n\n");
    }
    
    // Sincronizo todos os processos antes de começar o teste
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Chamo a MINHA implementação do scatter
    // Cada processo vai receber 3 elementos (12 elementos / 4 processos)
    meu_MPI_Scatter(dados_originais, 3, MPI_INT,
                    dados_recebidos, 3, MPI_INT,
                    0, MPI_COMM_WORLD);
    
    // Cada processo mostra o que recebeu
    printf("Processo %d recebeu: [%d, %d, %d]\n", 
           id, dados_recebidos[0], dados_recebidos[1], dados_recebidos[2]);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Pequena pausa para organizar a saída
    if(id == 0) printf("\n");
    
    // ============================================
    // TESTE 2: meu_MPI_Scatterv (blocos diferentes)
    // ============================================
    
    int novos_dados[10];
    int meu_buffer[10];
    
    // Arrays para configurar o Scatterv (apenas o processo root precisa)
    int sendcounts[4];
    int displs[4];
    
    if(id == 0) {
        printf("========== TESTE 2: meu_MPI_Scatterv ==========\n");
        
        // Preparo os dados originais
        for(int i = 0; i < 10; i++) {
            novos_dados[i] = i * 5;  // 0, 5, 10, 15, 20, 25, 30, 35, 40, 45
        }
        
        printf("Dados originais: ");
        for(int i = 0; i < 10; i++) {
            printf("%d ", novos_dados[i]);
        }
        printf("\n\n");
        
        // Defino quantos elementos cada processo vai receber
        sendcounts[0] = 2;  // processo 0 recebe 2 elementos
        sendcounts[1] = 3;  // processo 1 recebe 3 elementos
        sendcounts[2] = 3;  // processo 2 recebe 3 elementos
        sendcounts[3] = 2;  // processo 3 recebe 2 elementos
        
        // Calculo os deslocamentos (onde cada parte começa no buffer original)
        displs[0] = 0;                    // começa no indice 0
        displs[1] = sendcounts[0];        // começa no indice 2
        displs[2] = displs[1] + sendcounts[1];  // começa no indice 5
        displs[3] = displs[2] + sendcounts[2];  // começa no indice 8
    }
    
    // Cada processo precisa saber quantos elementos vai receber
    int meu_recvcount;
    if(id == 0) meu_recvcount = 2;
    else if(id == 1) meu_recvcount = 3;
    else if(id == 2) meu_recvcount = 3;
    else meu_recvcount = 2;
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Chamo a MINHA implementação do scatterv
    meu_MPI_Scatterv(novos_dados, sendcounts, displs, MPI_INT,
                     meu_buffer, meu_recvcount, MPI_INT,
                     0, MPI_COMM_WORLD);
    
    // Cada processo mostra o que recebeu
    printf("Processo %d recebeu %d elementos: [", id, meu_recvcount);
    for(int i = 0; i < meu_recvcount; i++) {
        printf("%d ", meu_buffer[i]);
    }
    printf("]\n");
    
    MPI_Finalize();
    return 0;
}