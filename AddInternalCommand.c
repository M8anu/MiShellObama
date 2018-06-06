#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "job_control.h"

extern job* jobb;

struct command {

    const char* command_name;
    //El primer argumento es el numero de elementos del array
    //El segundo, el array, es el array de argumentos 
    void (*funcc)(int , const char* []);
};

// Declaracion de comandos internos
void changeDirectory(int count, const char* args[]);
void printJobList(int count, const char* args[]);
void backGround(int count, const char* args[]);
void foreGround(int count, const char* args[]);

// Lista de comandos internos
static struct command commands[] = {
    {"cd", &changeDirectory}, 
    {"jobs", &printJobList}, 
    {"bg", &backGround}, 
    {"fg", &foreGround}
};

int searchInternal(const char* args[]){

    int pos=0;
    int count=0;

    while(args[count] != NULL){

        count++;
    }

    for(; pos<sizeof(commands)/sizeof(struct command); pos++){ //Como todos los elementos del array pesan lo mismo, usamos esta triquiñuela jeje

        if(strcmp(commands[pos].command_name, args[0]) == 0){

            commands[pos].funcc(count, args);
            return 1; //ya sé que es un return en un if... Aunque usamos el comodín de que en C es usado, así evitamos que itere el for completo cuando no se requiere

        }

    } 
    return 0;
}

void changeDirectory(int count, const char* args[]){
    //cd a secas, nos lleva a home
    if(count == 1){
        chdir(getenv("HOME"));
    } else { //en caso de haber dos o más argumentos, imprime un error si no ha podido ejecutar el comando correctamente
        if(chdir(args[1]) == -1){
            perror(args[1]);
        }
    }
}

void printJobList(int count, const char* args[]){
    block_SIGCHLD();
    print_job_list(jobb);
    unblock_SIGCHLD();

}

void backGround(int count, const char* args[]) {

    job* jobbaux;
    int pos;
    if(count == 1){
        pos = 1;

    }else{
        pos = atoi(args[1]);

    }

    if(pos == 0){ //atoi ha fallado(pe: "pato")

        printf("A number has not been introduced: %s\n", args[1]);

    }else{

        block_SIGCHLD();
        jobbaux = get_item_bypos(jobb, pos); //bg 2 pe: indica que el trabajo que ocupa la posicion 2(args[1]) debe ir a bg
        
        if(jobbaux == NULL){    

            printf("\033[31mJob not found:\033[0m %d\n", pos); //No existe tal trabajo a mandar a background

        }else{

            jobbaux->state = BACKGROUND;
            killpg(jobbaux->pgid, SIGCONT);
            printf("Process: %s has been sent to background.\n", jobbaux->command);           
        }

        unblock_SIGCHLD();

    }

}

void foreGround(int count, const char* args[]) {

    job* jobbaux;
    int pos, status, pid_wait, status_res, info;
    if(count == 1){
        pos = 1;

    }else{
        pos = atoi(args[1]);

    }

    if(pos == 0){ //atoi ha fallado(pe: "pato")

        printf("A number has not been introduced: %s\n", args[1]);

    }else{

        block_SIGCHLD();
        jobbaux = get_item_bypos(jobb, pos); //fg 2 pe: indica que el trabajo que ocupa la posicion 2(args[1]) debe ir a fg

        if(jobbaux == NULL){    

            printf("\033[31mJob not found:\033[0m %d\n", pos); //No existe tal trabajo a mandar a foreground

        }else{

        set_terminal(jobbaux->pgid);
        jobbaux->state = FOREGROUND;
        killpg(jobbaux->pgid, SIGCONT);
        pid_wait = waitpid(jobbaux->pgid, &status, WUNTRACED);
        status_res = analyze_status(status, &info);

        if(pid_wait == jobbaux->pgid){

            if(status_res == SUSPENDED){

                jobbaux->state = STOPPED;
                printf("Process %s with pid %d, is suspended\n", jobbaux->command, jobbaux->pgid);
            
            }else{

            printf("Process %s has finished.\n", jobbaux->command);
            pos = delete_job(jobb, jobbaux);
            }
        }

        set_terminal(getpid());
    }
    unblock_SIGCHLD();
}
}
