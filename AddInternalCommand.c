#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct command {

    const char* command_name;
    void (*funcc)(int , const char* []);
};

// Declaracion de comandos internos
void changeDirectory(int count, const char* args[]);

// Lista de comandos internos
static struct command commands[] = {
    {"cd", &changeDirectory}
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


}


