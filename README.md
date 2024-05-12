# Trabalho 01 cliente-dns
Sistema que funciona como um cliente dns de dominio autoritativo

## Membros
Alex Gabriel Alves Faustino - 200056603  
Levi de Oliveira Queiroz - 170108341  
Lucas de Lima Spinosa dos Santos - 180022512  
Nicolas Roberto de Queiroz - 200042360  

## Sistema operacional utilizado no desenvolvimento

3 membros usaram Linux - Ubuntu 22.04 lts
Subsistema Windows para Linux 

## Ambiente de desenvolvimento foi usado

Visual studio code  
Codeblocks  
Vim  

# Como construir a  aplicação

Antes de executar os comandos abaixo, certifique-se de ter o GCC, Make e CMake instalados no seu sistema.

```console
sudo apt install gcc 
```
```console
sudo apt install make
```
```console
sudo apt install cmake
```
entre na pasta /cliente e execute o seguinte comando

```console
make all  
```
# Como executar

após compilar execute 

Para rodar o projeto com as variaveis padrão execute apenas  

```console
make run 
```
Dominio padrão: unb.br  
Servidor DNS padrão: 8.8.8.8  

Para consulta personalizada execute o seguinte comando
```console
make run dominio=<nome_do_dominio> servidorDns=<ip_servidor_dns>
```

lembre-se de substituir <nome_do_dominio> e <ip_servidor_dns> pelos valores que deseja consultar, por exemplo: 
```console
make run dominio=amazon.com servidorDns=1.1.1.1
```

## quais são as telas 
## limitações conhecidas 
