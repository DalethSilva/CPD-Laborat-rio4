/*
 * meu_scatterv.c
 * Implementação da minha versão do MPI_Scatterv usando MPI_Send e MPI_Recv
 * 
 * Esta função distribui blocos DIFERENTES para cada processo.
 * Cada processo pode receber um número diferente de elementos.
 */

#include <mpi.h>

void meu_MPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                      MPI_Datatype sendtype, void *recvbuf, int recvcount,
                      MPI_Datatype recvtype, int root, MPI_Comm comm) {
    
    int id, p;
    MPI_Comm_rank(comm, &id);  // descobre qual é o meu ID
    MPI_Comm_size(comm, &p);   // descobre quantos processos existem
    
    // ===== SOU O PROCESSO ROOT =====
    if(id == root) {
        // O root envia para cada processo a sua parte específica
        for(int i = 0; i < p; i++) {
            
            // So envio se o processo i tiver elementos para receber
            if(sendcounts[i] > 0) {
                
                // Calculo onde começa a parte do processo i no buffer original
                // Uso o displacement que foi fornecido pelo utilizador
                int deslocamento = displs[i];
                char *ptr_envio = (char*)sendbuf + deslocamento * sizeof(int);
                
                if(i == root) {
                    // Se for o proprio root, copio localmente
                    for(int j = 0; j < recvcount; j++) {
                        ((int*)recvbuf)[j] = ((int*)sendbuf)[deslocamento + j];
                    }
                } else {
                    // Envio para o processo i a quantidade especifica de elementos
                    MPI_Send(ptr_envio, sendcounts[i], sendtype, i, 0, comm);
                }
            }
        }
    } 
    // ===== NAO SOU O PROCESSO ROOT =====
    else {
        // Recebo a minha parte do processo root
        MPI_Recv(recvbuf, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
    }
}