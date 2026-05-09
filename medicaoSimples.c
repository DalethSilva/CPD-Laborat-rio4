/*
 * Programa para medir latência e largura de banda com diferentes tamanhos
 * Versão simples e comentada para facilitar o estudo
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {

    MPI_Status status;
    int id, p;
    int i, repeticoes;
    double inicio, fim, tempo_total;
    int tamanho_msg;
    char *buffer;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    
    // vou usar apenas 2 processos
    if(p != 2) {
        if(id == 0) {
            printf("Preciso de 2 processos. Execute: mpirun -np 2 ./medicao_simples\n");
        }
        MPI_Finalize();
        exit(1);
    }
    
    // só o processo 0 vai mostrar os resultados
    if(id == 0) {
        printf("\n");
        printf("========================================\n");
        printf("    LATÊNCIA E LARGURA DE BANDA\n");
        printf("========================================\n");
        printf("Tamanho(B)  Repetições  Tempo(s)  Latência(us)  Banda(MB/s)\n");
        printf("========================================\n");
    }
    
    // vou testar estes tamanhos: 1, 8, 64, 512, 4096, 32768, 262144, 1048576
    int tamanhos[] = {1, 8, 64, 512, 4096, 32768, 262144, 1048576};
    
    // para cada tamanho, faço a medição
    for(int t = 0; t < 8; t++) {
        
        tamanho_msg = tamanhos[t];
        
        // escolho quantas repetições baseado no tamanho da mensagem
        if(tamanho_msg <= 512) {
            repeticoes = 5000;   // mensagens pequenas: muitas repetições
        } else {
            repeticoes = 500;    // mensagens grandes: menos repetições
        }
        
        // reservo espaço na memória para a mensagem
        buffer = (char*)malloc(tamanho_msg * sizeof(char));
        
        // coloco dados dentro do buffer (pode ser qualquer coisa)
        for(i = 0; i < tamanho_msg; i++) {
            buffer[i] = 'A';
        }
        
        // espero todos os processos ficarem prontos
        MPI_Barrier(MPI_COMM_WORLD);
        
        // começo a contar o tempo
        inicio = MPI_Wtime();
        
        // aqui é a parte principal: processo 0 e 1 trocam mensagens
        if(id == 0) {
            for(i = 0; i < repeticoes; i++) {
                MPI_Send(buffer, tamanho_msg, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buffer, tamanho_msg, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
            }
        }
        else { // id == 1
            for(i = 0; i < repeticoes; i++) {
                MPI_Recv(buffer, tamanho_msg, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
                MPI_Send(buffer, tamanho_msg, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }
        
        // paro de contar o tempo
        fim = MPI_Wtime();
        tempo_total = fim - inicio;
        
        // calculo a latência (tempo por mensagem, em microsegundos)
        double latencia = (tempo_total / (2 * repeticoes)) * 1000000;
        
        // calculo a largura de banda (bytes por segundo, converto para MB/s)
        double total_bytes = 2.0 * repeticoes * tamanho_msg;
        double banda = (total_bytes / tempo_total) / (1024 * 1024);
        
        // mostro o resultado
        if(id == 0) {
            printf("%-12d %-12d %-10.6f %-13.2f %-10.2f\n", 
                   tamanho_msg, repeticoes, tempo_total, latencia, banda);
        }
        
        // liberto a memória que reservei
        free(buffer);
    }
    
    if(id == 0) {
        printf("========================================\n");
    }
    
    MPI_Finalize();
    return 0;
}