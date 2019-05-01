#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../GLOBAL_SOURCE/global.h"

#define LINE_STOCK 9

#define TAM_PEDIDO 25

#define TAM_RESPOSTA 43

#define BASE_PATH "../PipeVendas/pipePrintCliente"

//----------------------------------------------------

void printStockPreco(int codigoProduto, int clientID) {

	//----------------------------------------------------
	//         MOSTRAR O STOCK
	//----------------------------------------------------
	
	//Abrir o ficheiro de Stocks
	int fd_stock = open(PATH_STOCK, O_RDONLY, 0666);

	//----------------------------------------------------

	//Caracteres lidos pelo readln
	int n = 0, codigoAtual = 1;
	//Ler para o buffer
	char buffer[MAX_LINE];

	do 
	{
		n = readln(fd_stock, buffer, MAX_LINE);

		codigoAtual++;

	} while(codigoAtual <= codigoProduto && n > 0);

	//Caso nao exista o codigo
	if (codigoAtual == codigoProduto) 
		return;

	int quantidadeStock = atoi(buffer);

	//Fechar o ficheiro de stocks
	close(fd_stock);

	//----------------------------------------------------
	//         MOSTRAR O PRECO
	//----------------------------------------------------

	//Abrir o ficheiro de artigos
	int fd_artigo = open(PATH_ARTIGOS, O_RDONLY, 0666);
	//----------------------------------------------------

	int pos_leitura = LINE_ARTIGOS * (codigoProduto - 1);

	off_t offset = lseek(fd_artigo, pos_leitura, SEEK_SET);

	n = readln(fd_artigo, buffer, LINE_ARTIGOS);

	char pathCliente[200];
	sprintf(pathCliente, "%s%d", BASE_PATH, (int) clientID);

	//Caso o codigo exceda o tamanho do ficheiro
	if (buffer == NULL || strlen(buffer) == 0) {

		char error_overflow[MAX_LINE];

		int fd_pipe_escrita = open(pathCliente, O_WRONLY);
	
		sprintf(error_overflow, "3 %07d %08d %08d %012.2lf", 0, 0, 0, 0.0);

		if(write(fd_pipe_escrita, error_overflow, TAM_RESPOSTA)!=-1);

		close(fd_pipe_escrita);

		return;
	}
	
	char **campos = tokenizeArtigo(campos, buffer);

	double precoLido = atof(campos[1]);

	char bufferEscrita[MAX_LINE];
	sprintf(bufferEscrita, "0 %07d %08d %08d %012.2lf", clientID, 
														codigoProduto, 
														quantidadeStock, 
														precoLido);
	

	int fd_pipe_escrita = open(pathCliente, O_WRONLY);

	if(write(fd_pipe_escrita, 
		bufferEscrita, TAM_RESPOSTA)!=-1);

	close(fd_pipe_escrita);



	//Já nao é necessario o ficheiro artigos
	close(fd_artigo);
}

//----------------------------------------------------

void updateQuantidadeStock (int codigo, int novaQuantidade, int clientID) {

	int fd_stock = open(PATH_STOCK, O_RDWR, 0666);

	int pos_leitura = (codigo-1) * LINE_STOCK;
	off_t offset = lseek(fd_stock, pos_leitura, SEEK_SET);

	char stockAntigo[MAX_LINE];
	int n = readln(fd_stock, stockAntigo, MAX_LINE);

	offset = lseek(fd_stock, pos_leitura, SEEK_SET);

	int finalQuantidade = atoi(stockAntigo) + novaQuantidade;
	
	char newStock[10];

	sprintf(newStock, "%08d\n", finalQuantidade);	

	if (write(fd_stock, newStock, 9) != -1);

	close(fd_stock);

	char bufferEscrita[MAX_LINE];
	sprintf(bufferEscrita, "1 %07d %08d %08d %012.2lf", clientID, 
														0, 
														finalQuantidade, 
														0.0);

	char pathCliente[200];
	sprintf(pathCliente, "%s%d", BASE_PATH, (int) clientID);

	int fd_pipe_escrita = open(pathCliente, O_WRONLY);
	
	if(write(fd_pipe_escrita, 
		bufferEscrita, TAM_RESPOSTA)!=-1);

	close(fd_pipe_escrita);
}

//----------------------------------------------------

void updateVenda (int codigo, int quantidade) 
{
	//Obter o preço de uma venda---------------------

	//Abrir o ficheiro de artigos
	int fd_artigo = open(PATH_ARTIGOS, O_RDONLY, 0666);

	//Zerar o contador
	int codigoAtual = 0;
	int n = 0;
	char buffer[MAX_LINE];

	do 
	{
		n = readln(fd_artigo, buffer, MAX_LINE);		

		codigoAtual++;

	} while(codigoAtual < codigo && n > 0);

	//No buffer tenho a linha certa
	//Para guardar a divisao do buffer
	char **campos = tokenizeArtigo(campos, buffer);

	//Preco do artigo para calcular o montante
	double precoLido = atof(campos[1]);

	//Falta apenas acrescentar ao ficheiro de vendas o final
	int fd_vendas = open(PATH_VENDAS, O_APPEND | O_WRONLY, 0666);

	//Juntar a venda toda num buffer

	char bufferEscrita[MAX_LINE];
	sprintf(bufferEscrita, "%d %d %.2lf\n", codigo, quantidade, quantidade*precoLido);	

	if(write(fd_vendas, bufferEscrita, strlen(bufferEscrita)) != -1);

	close(fd_vendas);
}

int main() 
{
	//Numero de chars lidos pelo read
	int n = 1, i = 0;

	//Array de strings para guardar o comando lido
	char** campos;

	//Para guardar o comando lido
	char buffer[MAX_LINE];
	
	int fd_pedidos = open("../PipeVendas/pipeClienteVendas", O_RDONLY);
	
	while(n > 0) {

		n = read(fd_pedidos, buffer, TAM_PEDIDO);

		if (n <= 0) break;

		campos = tokenizePedidodServidor(buffer);

		//Os comandos inseridos podem ser dois:
		//1: <codigo> -> mostra o stock
		//2: <codigo> <quantidade> -> atualiza o stock e mostra novo stock
		//Se tiver um espaço então é o 2º comando
		if(atoi(campos[2]) != 0) {

			int codigo = atoi(campos[1]), quantidade = atoi(campos[2]);

			if (quantidade > 0) 
				updateQuantidadeStock(codigo, quantidade, atoi(campos[0]));
			else	
			{	
				updateQuantidadeStock(codigo, quantidade, atoi(campos[0]));
				updateVenda(codigo, abs(quantidade));
			}
		} 
		else {

			//Passo-lhe o codigo do produto em questao
			printStockPreco(atoi(campos[1]), atoi(campos[0]));
		}
	}
	
	close(fd_pedidos);
	
	main();

	return 0;
}