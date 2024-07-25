# MiShell Obama
Tarea de evaluación para la parte práctica de la asignatura Sistemas Operativos, que consiste en la creación de una shell basada en bourne again shell(BA-SH, nice acrónimo ajaj), preparada para leer comandos y  ejecutar programas,  básicos, y avanzados(ampliaciones).

## Agradecimientos
Melchor(melchor9000): https://github.com/melchor629 Asesoramiento y gurú things.

Zu: parte creativa, da name, apoyo rocoso.

## Contenido

Este repositorio contiene los archivos necesarios para compilar y ejecutar una shell en cualquier sistema linux, con un toque de personalización jeje.

Compilar y ejecutamos usando los ficheros AddInternalCommand.c , Shell_project.c y job_control.c de la siguiente forma:

```
gcc -o MiShell Shell_project.c AddInternalCommand.c job_control.c 
./MiShell
```

La shell incorpora el tratamiento de comandos en primer y segundo plano, su suspensión, el tratamiento de las señales SIGCHLD, la incorporación de los comandos internos `fg`, `bg`, `cd` y `jobs`.

En expansión...
