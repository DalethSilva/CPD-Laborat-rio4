/*
 * meu_scatter.c
 * Implementação da minha versão do MPI_Scatter usando MPI_Send e MPI_Recv
 * 
 * Esta função distribui blocos IGUAIS para todos os processos.
 * Exemplo: com 4 processos e 12 elementos, cada um recebe 3 elementos.
 */

#include <mpi.h>

void meu_MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                     void *recvbuf, int recvcount, MPI_Datatype recvtype,
                     int root, MPI_Comm comm) {
    
    int id, p;
    MPI_Comm_rank(comm, &id);  // descobre qual é o meu ID (0,1,2,3...)
    MPI_Comm_size(comm, &p);   // descobre quantos processos existem no total
    
    // ===== SOU O PROCESSO ROOT =====
    if(id == root) {
        // O root envia uma parte para cada processo (incluindo para ele mesmo)
        for(int i = 0; i < p; i++) {
            
            // Calculo onde começa a parte do processo i no buffer original
            // Exemplo: se cada processo recebe 3 elementos, o processo 0 começa na posicao 0,
            // o processo 1 na posicao 3, o processo 2 na posicao 6, etc.
            int deslocamento = i * sendcount;
            
            // Ponteiro para o inicio da parte do processo i
            char *ptr_envio = (char*)sendbuf + deslocamento * sizeof(int);
            
            if(i == root) {
                // Se for o proprio root, nao preciso enviar pela rede
                // copio diretamente do buffer de envio para o buffer de recepcao
                for(int j = 0; j < sendcount; j++) {
                    ((int*)recvbuf)[j] = ((int*)sendbuf)[deslocamento + j];
                }
            } else {
                // Envio a parte do processo i pela rede
                MPI_Send(ptr_envio, sendcount, sendtype, i, 0, comm);
            }
        }
    } 
    // ===== NAO SOU O PROCESSO ROOT =====
    else {
        // Recebo a minha parte do processo root
        MPI_Recv(recvbuf, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
    }
}