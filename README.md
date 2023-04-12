# PROYECTO CHAT SISTEMAS OPERATIVOS

### Integrantes
- Diego Alonzo, 20172.
- Roberto Rios, 20979.

###  Compilacion del programa

Ejecute el `make` estando en el root:

El make ejecutara los siguientes comandos:
- `g++ -std=c++17 serverex.cpp project.pb.cc -o server `pkg-config --cflags --libs protobuf`
- `g++ -std=c++17 clientex.cpp project.pb.cc -o client `pkg-config --cflags --libs protobuf``


Luego de eso, en el root del directorio, ingrese los siguientes comandos:

- Para abrir el servidor:
`./server 1404`

- Para abrir el cliente y conectarse a AWS:
`./client <nombre de usuario> 35.169.93.164 1404`

