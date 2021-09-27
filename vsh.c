/*vaccine shell*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_COMANDOS 6
#define MAX_STRING 100
#define MAX_SENT 5
//SIGUSER1 e SIGUSER2 São fatais
/*
& Além disso, durante a execução do tratador desses sinais, os seguintes sinais que podem ser gerados
& via comando especial “Ctrl-...” ( isto é, Ctrl-C (SIGINT), Ctrl-\ (SIGQUIT), Ctrl-Z (SIGTSTP))
& devem estar bloqueados, para evitar que o tratamento dos sinais SIGUSR1 e SIGUSR2 seja
& interrompido no meio.
*/
/*
- liberamoita:
faz com que o shell libere todos os seus descendentes (diretos e indiretos,
isto é, filhos, netos, bisnetos, etc.) que estejam no estado “Zombie” antes de
exibir um novo prompt.
*/
/*
- armagedon:
termina a operação do shell, mas antes disso, ele deve matar todos os seus
descendentes (diretos e indiretos, isto é, filhos, netos, bisnetos, etc.) que
ainda estejam rodando.
*/

//- LEMBRAR DE TRATAR CTRL-C
//- CONCERTAR MENSAGEM DE INFECÇÃO

//* Leitura funcionando com liberação de memória OK
//* tratadores de sinal OK
//* Rodar mais de um comando da RUIM [CORRIGIDO]
//* Rodando múltiplos comandos (Mas sem redirecionamento da saída)
//?  |-> Tá bugando a saída pois não tem o redirecionamento com PIPE
//* Processos em BG terminam quando um dos "irmãos" recebe SIGUSR OK
//& Fazer para vários filhos (PIPE!)
//& Implementar armagedon e libera moita
//& Testar sinais no programa


//^ O comando virus1 infecta o vsh com SIGUSR1
//^ O comando virus2 infecta o vsh com SIGUSR2

//Tratadores de sinais
void trataSIGINT(){
    printf("Nao funciona fds\n");
}

void trataSIGQUIT(){
    printf("Nao funciona fds\n");
}

void trataSIGTSTP(){
    printf("Nao funciona fds\n");
}

void fiqueiDoente(){
    printf("I feel so sick, goodbye world...\n");
    raise(SIGTERM);
}

void trataSIGUSER1(){
    FILE* f = fopen("shellImune.txt", "r");
    if(f == NULL){
        printf("I feel sick but, I'm immune\n");
        return;
    }
    char c;
    while (!feof(f)){
        fscanf(f, "%c", &c);
        printf("%c", c);
    }
    fclose(f);
}

void trataSIGUSER2(){
    FILE* f = fopen("shellImune.txt", "r");
    if(f == NULL){
        printf("I feel sick but, I'm immune\n");
        return;
    }
    char c;
    while (!feof(f)){
        fscanf(f, "%c", &c);
        printf("%c", c);
    }
    fclose(f);
}

char** linhaDecomando(int* indice){

    char linha[MAX_STRING];
    char* result;
    char** comandos = (char**)malloc(sizeof(char*)*MAX_COMANDOS);
    char* token = "A";

    result = fgets(linha, MAX_STRING, stdin);

    //Faz a leitura de uma linha inteira de qualquer tamanho
    if (result == NULL){
        printf("Ocorreu um erro na leitura\n");
        return NULL;
    }
    if(result[0] == 10){
        return NULL;
    }

    //Separa os comandos passados via shell em strings separadas (máx 5 comandos)
    token = strtok(linha, "|\n");

    while (token != NULL){
        comandos[*indice] = strdup(token);
        token = strtok(NULL, "|\n");
        (*indice)++;
    }

    comandos[*indice] = NULL;//Para usar no formato do execv..
    return comandos;
}

void liberaComandos(char** vString, int indice){
    for(int i = 0; i < indice; i++){
        free(vString[i]);
    }
    free(vString);
}

//tentativa de funcao que fecha os pipes
void closeAllPipes(int nPipes, int fd[][2]){
    for (int i = 0; i < nPipes; ++i) {
        close(fd[i][0]);
        close(fd[i][1]);
    }
}

int main(){

    char** comandos;
    int indice = 0;

    //Instalando tratadores de sinais
    signal(SIGQUIT, trataSIGQUIT);
    signal(SIGTSTP, trataSIGTSTP);
    signal(SIGUSR1, trataSIGUSER1);

    //printf("vsh> ");

    // 4 pipes, pois sao 5 processos no maximo, 4 ligacoes entre eles
    int fd[4][2];

    //f[x][0] = read
    //f[x][1] = write

    // filho1 ->  filho2
    //f[0][1] -> f[0][0]

    //filho2  -> filho3
    //f[1][1] -> f[1][0]

    //filho3  -> filho4
    //f[2][1] -> f[2][0]

    //filho4  -> filho5
    //f[3][1] -> f[3][0]

    /*
     * filho1: f[0][1]
     * filho2: f[0][0] | f[1][1]
     * filho3: f[1][0] | f[2][1]
     * filho4: f[2][0] | f[3][1]
     * filho5: f[3][0]
     * */


    int cont = 0;
    int pid;//pid do filho em FG
    int* sentinela;//Sentinelas responsáveis pelos processos BG

    int n = 0;//Número de Sentinelas
    int prox = 0;
    int tamSentinelas = MAX_SENT;//Tamnho do vetor de sentinelas

    //& CORPO DO WHILE
    while(cont < 10){
        printf("vsh> ");
        indice = 0;
        comandos = linhaDecomando(&indice);
        if(comandos == NULL){
            cont++;
            continue;
        }


        //-DEBUG PARA TESTAR SINAIS
        if(strcmp(comandos[0], "virus1") == 0){
            raise(SIGUSR1);
        }else if(strcmp(comandos[0], "virus2") == 0){
            raise(SIGUSR2);
        }else if(indice == 1){//foreground

            if((pid = fork()) < 0){
                printf("Infelizmente um erro ocorreu. Falha na criacao de um proceso\n");
                exit(1);
            }

            if(pid == 0){

                char* flags[10];//Armazena o comando e as flags no formato do exec

                char* token = strtok(comandos[0], " ");
                int i = 0;

                while (token != NULL){
                    //	printf("[]%s\n", token);
                    flags[i] = token;
                    token = strtok(NULL, " ");
                    i++;
                }
                flags[i] = NULL;

                execvp(flags[0], flags);
                //Em caso de sucesso o código abaixo não é executado,
                //caso haja falha, o código abaixo exibe uma mensagem de erro no terminal

                //sleep(300);

                printf("Falha no comando: ");
                for(int k = 0; k < i; k++){
                    printf("%s ", flags[k]);
                }
                printf("\n");

                return 0;
            }
            //Aqui o vsh espera pelo término de seu único filho, para simular o foreground
            waitpid(pid, NULL, 0);
        }else{//background

            //Coleta de "zombies" (sentinelas que morreram e não tiveram status reportado ao vsh)
            //? -----------------------
            if(n == MAX_SENT){//Procura por filhos zombies
                prox = -1;
                for(int i = 0; i < tamSentinelas; i++){
                    if(waitpid(sentinela[i], NULL, WNOHANG) > 0){
                        sentinela[i] = 0;
                        prox = i;
                        n--;
                    }
                }

                if(prox == -1){//vetor cheio e nenhum dos filhos terminou
                    int* aux = (int*)malloc(sizeof(int)*(tamSentinelas+3));
                    for(int i = 0; i < tamSentinelas; i++){
                        aux[i] = sentinela[i];
                    }
                    free(sentinela);
                    sentinela = aux;
                }
            }else{//Procura prox posição vazia
                for(int i = 0; i < tamSentinelas; i++){
                    if(!sentinela[i]) prox = i;
                }
            }
            //? -----------------------
            if((sentinela[prox] = fork()) < 0){
                printf("Infelizmente um erro ocorreu. Falha na criacao de um preocesso.\n");
                exit(1);
            }

            if(sentinela[prox] == 0){//Código do sentinela (VSH não executa essa parte)

                //criacao e verificação dos pipes com base no numero de processos
                int pipes = indice -1;
                for (int i = 0; i < pipes; ++i) {
                    if (pipe(fd[i]) == -1){
                        return 1;
                    }
                }

                int c_pid[indice];//Armazenar o pid de todos os filhos
                setsid();//? Fazer mais testes quando o pipe estiver pronto
                for(int p = 0; p < indice; p++){
                    if((c_pid[p] = fork()) < 0){
                        printf("Infelizmente um erro ocorreu. Falha na criacao de um preocesso.\n");
                        exit(1);
                    }else if(c_pid[p] == 0){//Código do processo Filho

                        if (p == 0){//primeiro filho definido o pgid o proprio pid
                            setpgrp();
                        }
                        else{//proximos filhos herdam o pgid do primeiro filho
                            setpgid(getpid(), c_pid[0]);
                        }

                        char* flags[10];

                        char* token = strtok(comandos[p], " ");
                        int i = 0;

                        while (token != NULL){
                            //	printf("[]%s\n", token);
                            flags[i] = token;
                            token = strtok(NULL, " ");
                            i++;
                        }
                        flags[i] = NULL;

                        if (p == 0){
                            //primeiro filho
                            //filho1: f[0][1]
                            //faz a saida do exec ser direcionada ao pipe
                            dup2(fd[p][1], STDOUT_FILENO);
                        }else if (p == 4){
                            //quinto filho
                            //filho5: f[3][0]
                            dup2(fd[p - 1][0], STDIN_FILENO);
                        }
                        else{
                            //segundo, terceiro, quarto filho
                            //filho2: f[0][0] | f[1][1]
                            //filho3: f[1][0] | f[2][1]
                            //filho4: f[2][0] | f[3][1]
                            dup2(fd[p - 1][0], STDIN_FILENO);

                            //verifica se ainda existe um pipe a receber a saida deste processo
                            if (pipes > p){
                                dup2(fd[p][1], STDOUT_FILENO);
                            }
                        }
                        closeAllPipes(pipes, fd);

                        execvp(flags[0], flags);
                        //Em caso de sucesso o código abaixo não é executado,
                        //caso haja falha, o código abaixo exibe uma mensagem de erro no terminal

                        printf("Falha no comando: ");
                        for(int k = 0; k < i; k++){
                            printf("%s ", flags[k]);
                        }
                        printf("\n");
                        return 0;
                    }
                    //Para impedir que a race-condition dê problrmas, os grupos são atualizados também na sentinela
                    setpgid(c_pid[p], c_pid[0]);
                }
                int status;
                int pid1;//dupla declaracao de pid, modifiquei para pid1
                //Espera por todos os filhos
                for(int i = 0; i < indice; i++){
                    pid1 = waitpid(-1*c_pid[0], &status, 0);//Ver se qualquer filho do grupo terminou
                    if(pid1 != -1){
                        if(WIFSIGNALED(status)){
                            if(WTERMSIG(status) == SIGUSR1){//Se algum filho terminoi de SIGUr

                                printf("Filho terminou de sigUsr1, terminar os irmaos\n");
                                killpg(c_pid[0], SIGTERM);//Envia o sinal de terminação para o grupo todo
                                break;
                            }
                        }
                    }
                }
                closeAllPipes(pipes, fd);
                return 0;
            }else{//vsh
                n++;//incrementa número de sentinelas
            }
        }
        /*Processo Principal*/
        liberaComandos(comandos, indice);
        sleep(1);
        cont++;
    }

    free(sentinela);

    //& -----------------

    // //- DEBUG
    // for(int i = 0 ; i < indice; i++){
    // 	printf("[%s]\n", comandos[i]);
    // }
    return 0;
}