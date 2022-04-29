# twitter-notification

## Compilação

Após baixar o código, é necessário compilar o servidor e o cliente:

```bash
cmake -B build .
cmake --build build
```

Após isto, na pasta `build` vai existir uma `bin` com os executaveis do cliente e do servidor.

## Execução

Para executar um servido como primário:

```bash
./server -p <port>
```

Para executar um servidor como backup:

```bash
./server -b <port> <primary_host> <primary_port>
```

Para executar um cliente:

```bash
./client <profile> <server_host> <server_port>
```