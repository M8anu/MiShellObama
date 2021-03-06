/**
UNIX Shell Project

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell          
    (then type ^D to exit program)

Author: Manuel Ariza López
**/

#include <signal.h> //primero las cabeceras del sistema
#include "job_control.h"   // remember to compile with module job_control.c 

int searchInternal(const char* args[]);
void handler(int);
void print_shell(void);

job* jobb; //Creamos una lista de trabajos

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

int main(void)
{
    char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
    int background;             /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
    // probably useful variables:
    int pid_fork, pid_wait; /* pid for created and waited process */
    int status;             /* status returned by wait */
    enum status status_res; /* status processed by analyze_status() */
    int info;               /* info processed by analyze_status() */

    ignore_terminal_signals(); //Hace que el shell no crashee en caso de que los hijos hagan sus cosas
    /* the  (BASIC) steps are:
             (1) fork a child process using fork()
             (2) the child process will invoke execvp()
             (3) if background == 0, the parent will wait, otherwise continue 
             (4) Shell shows a status message for processed command 
             (5) loop returns to get_commnad() function
        */
    print_shell(); //Imprime una bonita introducción a la shell
    signal(SIGCHLD, handler); //Cuando llegue una señal SIGCHLD, se ejecuta el manejador
    jobb = new_list("Jobb");

    while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
    {           
        printf("\033[94mGive me a command m8:\033[0m ");
        fflush(stdout);
        get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */
        
        if(args[0]==NULL) continue;   // if empty command

        if(searchInternal((const char**) args) == 1) {
                                        //lifehack, si reconoce un comando interno, continue quiere decir que ejecuta de nuevo desde el comienzo del bucle, para así
                                        // ejecutar el comando interno encontrado
                                        //doble puntero, array de arrays
            continue;
        }

        pid_fork = fork(); //Se crea un nuevo proceso(hijo/copia)
        
        if(pid_fork == 0){ //ESTE CODIGO es del hijo

            restore_terminal_signals(); //recupera las señales ignoradas previamente ya que aqui no van a causar conflicto, ademas, pueden ser utiles para el hijo
            new_process_group(getpid()); //este es el orden correcto, para que no nos suspenda los procesos (no reason given)
            execvp(args[0], args); //ejecuta el comando, metaformosis de Kafka, el hijo, shell originalmente, se transforma en el programa a ejecutar
            //si no ha podido ejecutar correctamente, arroja este error
            printf("\033[31mCommand not found m8:\033[0m %s\n", args[0]);
            exit(127); //usamos 127 porque es el info arrojado por el hijo al morir, lo matamos para que no se ejecute infinamente, haciendonos llorar

         } else { //ESTE CODIGO es del padre

            if(background == 0) { //indica la ejecución en fg del comando

                set_terminal(pid_fork); //le damos el terminal al grupo de procesos donde se encuentra el hijo
                pid_wait = waitpid(pid_fork, &status, WUNTRACED); //el padre espera a que termine el hijo
                status_res = analyze_status(status, &info); //analiza el estado de finalizacion del proceso
                set_terminal(getpid()); //le devuelve la terminal a MiShell

                if(status_res != EXITED || info != 127) { // comprueba que el comando introducido efectivamente es valido
                    if(status_res == SUSPENDED) { //ATENTO AQUI recuerda SUSPENDED

                        block_SIGCHLD();
                        job* Antonio = new_job(pid_fork, args[0], STOPPED);
                        add_job(jobb, Antonio);
                        unblock_SIGCHLD();
                        printf("\rForeground pid: %d , command: %s , %s , info: %d \n", pid_wait, args[0], status_strings[status_res], info);
                    } else {
                        //El proceso ha terminado correctamente
                        printf("Foreground pid: %d , command: %s , %s , info: %d \n", pid_wait, args[0], status_strings[status_res], info);
                    }
                }

            }else{

                block_SIGCHLD();
                job* Antonio = new_job(pid_fork, args[0], BACKGROUND); //Creamos un nuevo trabajo segun los hearders del archivo job_control.h
                add_job(jobb, Antonio); //Anyadimos un trabajo, pasando la lista de trabajos creada y el trabajo creado
                unblock_SIGCHLD();
                printf("Background job running... pid: %d , command: %s\n", pid_fork, args[0]);
            }
        }



    } // end while

}

void print_shell(){
    printf("\n\033[92m"
           "───█───▄▀█▀▀█▀▄▄───▐█──────▄▀█▀▀█▀▄▄\n"
           "──█───▀─▐▌──▐▌─▀▀──▐█─────▀─▐▌──▐▌─█▀\n"
           "─▐▌──────▀▄▄▀──────▐█▄▄──────▀▄▄▀──▐▌\n"
           "─█────────────────────▀█────────────█\n"
           "▐█─────────────────────█▌───────────█\n"
           "▐█─────────────────────█▌───────────█\n"
           "─█───────────────█▄───▄█────────────█\n"
           "─▐▌───────────────▀███▀────────────▐▌\n"
           "──█──────────▀▄───────────▄▀───────█\n"
           "───█───────────▀▄▄▄▄▄▄▄▄▄▀────────█\n"
 
           "\n      Welcome to MiShell Obama\n\n"
           "-Name courtesy of Zu\n"
           "-Constant help and support by @melchor9000(Melchor)\033[0m\n\n"
           );

}

void handler(int m){ //A esta funcion se la llama cuando un proceso hijo ejecutado en segundo plano sufre algun evento(muere, suspendido, reanudado)

    pid_t pid_wait;
    int status, info;
    enum status status_res;
    int auxDied;
    job* aux = jobb->next, *auxaux;
    //Aux != NULL lo usamos debido a que en una LinkedList siempre va a haber un elemento siguiente, si es NULL, sabemos que hay un NULL, claro, pero jeje
    while(aux != NULL){

        auxDied = 0;
        pid_wait = waitpid(aux->pgid, &status, WUNTRACED | WNOHANG | WCONTINUED); //Usamos OR bit a bit, es decir, 0 -> 1 -> 11 -> 111
        //Aux->pgid indica como accedemos al id del proceso encerrado en la casilla actual (aux) del array de trabajos (jobb)
        //Con wait esperamos la respuesta del proceso, y de no haberla WNOHANG hace que devuelva 0 en lugar de esperar a que ocurra algo

        if(pid_wait == aux->pgid){ //Comprobamos si le ha pasado algo al proceso actual(le ha pasado)

            status_res = analyze_status(status, &info); //analiza el estado del proceso actual

            if(status_res == CONTINUED){

                aux->state = BACKGROUND; //Anotamos la reanimacion de un proceso a background, no puede ser a foreground, es solo cosa de Shell Padre
                printf("Process %s with pid %d, has been resumed again\n", aux->command, aux->pgid);


            }else if(status_res != SUSPENDED){

                printf("Process %s with pid %d, has finished because of %s\n",  aux->command, aux->pgid, status_strings[status_res]);
                auxaux = aux; //Guardamos el proceso actual porque en la siguiente linea la vamos a perder
                aux = aux->next; //Avanzamos al siguiente porque aux va a ser eliminado y no lo podemos usar mas
                delete_job(jobb, auxaux); //Borramos el proceso actual

                auxDied=1; //Ponemos esto para que si muere, no avance al if de abajo, crashea, porque pega dos saltos, perdiendo el link
                
            }else{
                aux->state = STOPPED;
                printf("Process %s with pid %d, is suspended\n", aux->command, aux->pgid);

            }
        }

        if(auxDied == 0){
            aux = aux->next; //Avanza a la siguiente tarea y esa nueva tarea es el nuevo indice, ricos punteros de C
        }
    }
}