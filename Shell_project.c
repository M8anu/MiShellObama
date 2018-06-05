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

#include "job_control.h"   // remember to compile with module job_control.c 

int searchInternal(const char* args[]);
void print_shell(void);
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

    ignore_terminal_signals();
    /* the  (BASIC) steps are:
             (1) fork a child process using fork()
             (2) the child process will invoke execvp()
             (3) if background == 0, the parent will wait, otherwise continue 
             (4) Shell shows a status message for processed command 
             (5) loop returns to get_commnad() function
        */
    print_shell();

    while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
    {           
        printf("\033[94mGive me a command m8:\033[0m ");
        fflush(stdout);
        get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */
        
        if(args[0]==NULL) continue;   // if empty command

        if(searchInternal(args) == 1) { //lifehack, si reconoce un comando interno, continue quiere decir que ejecuta de nuevo desde el comienzo del bucle, para así
                                        // ejecutar el comando interno encontrado
            continue;
        }

        pid_fork = fork(); //Se crea un nuevo proceso(hijo/copia)
        
        if(pid_fork == 0){ //ESTE CODIGO es del hijo

            restore_terminal_signals(); //recupera las señales ignoradas previamente
            new_process_group(getpid()); //este es el orden correcto, para que no nos suspenda los procesos 
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
                    printf("Foreground pid: %d , command: %s , %s , info: %d \n", pid_fork, args[0], status_strings[status_res], info);
                }

            }else{

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

           "\n      Welcome to MiShell Obama\033[0m\n\n");
}
