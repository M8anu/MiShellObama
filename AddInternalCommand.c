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

// Lista de comandos internos
static struct command commands[] = {
    {"cd", &changeDirectory}, 
    {"jobs", &printJobList} 
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

    print_job_list(jobb);
}

