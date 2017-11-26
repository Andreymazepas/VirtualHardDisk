/* Trabalho final da disciplina de Organização de Arquivos
	UNB 2017
	Andrey Emmanuel Matrosov Mazépas - 16/0112362
	Cristiane Naves Cardos           - 15/0008023

	pelo que eu entendi a representação geral do hd seria assim
	o conteudo no caso sao setores.

	cilindro 1: (153600 bytes, 153,6Kb)
		trilha 1: 0..59 (30720 bytes, 30,72Kb)
		trilha 2: 60..119
		trilha 3: 120..179
		trilha 4: 180..239
		trilha 5: 240..299

	cilindro 2:
		trilha 1: 300..359
		trilha 2: 360..419
		trilha 3: 420..479
		trilha 4: 480..539
		trilha 5: 540..599

	...

	no codigo estaria assim

	track_array = {track[0]
					track[1]
				   ...}
	track[0] = { sector[0]
				sector[1]
				...}

	sector[0] = { char bytes_s[512] } com cada posição sendo um char

*/


//#include <bits/stdc++.h>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>	
#include <string.h>

#define T_SEEK_MEDIO 4
#define T_SEEK_MIN 1
#define T_LATENCIA_MEDIA 6
#define T_TRANSF_TRILHA 12
#define SETORES_TRILHA 60
#define TRILHAS_SUPERF 10
#define TRILHAS_CILINDRO 5
#define SETOR_BYTES 512
#define CLUSTER_SETORES 4
#define CLUSTER_BYTES 2048
#define TOTAL_SETORES (TRILHAS_SUPERF * TRILHAS_CILINDRO * SETORES_TRILHA)	
#define MAX_ARQUIVOS 100

typedef struct block { char bytes_s[512]; } block; //menor unidade de armazenamento
typedef struct sector_array { block sector[60]; } sector_array; //uma trilha, 60 setores com 1 bloco cada (?)
typedef struct track_array { sector_array track[5]; } track_array; //um cilindro

typedef struct fatlist_s {
	char file_name[100] = { '\0' }; //inicializando como string vazia
	unsigned int first_sector;
} fatlist; //conteudo da tabela FAT contendo nome e primeiro setor do arquivo

typedef struct fatent_s {
	unsigned int used;
	unsigned int eof;
	unsigned int next;
} fatent; //flags que identificam cada setor

//a FAT indica se o setor esta ocupado ou nao e a posicao do primeiro setor de cada arquivo
typedef struct Fat {
	int total_arquivos;
	fatlist lista_arquivos[MAX_ARQUIVOS];
	fatent setores[TOTAL_SETORES];
} Fat;

Fat fat;
track_array cilindros[TRILHAS_SUPERF];
/*Prototipos das funcoes */
int buscar_setor_disponivel();
void calcularPos(int *p_cilindro, int *p_trilha, int *p_setor, int pos_bytes);
int write();
void ler_arquivo(int pos_arquivo);
int read();
void apagar_arquivo(int i);
int erase();
int calcula_tamanho(int indice);
int show_FAT();
int menu();


/*Busca o primeiro setor livre da tabela FAT, o que corresponde ao primeiro setor
de um cluster*/ 
int buscar_setor_disponivel(){
	int setor = 0;
	for(int i = 0; i < TOTAL_SETORES; i++){
		if(fat.setores[i].used == 0){
			setor = i;
			break;
		}
	}
	return setor;
}

/*ver em qual cilindro, trilha e setor esta a posicao em bytes do 
pos_setor(posicao do setor inicial do arquivo na FAT) multiplicado
por 512(indicando o total de bytes)*/
void calcularPos(int *p_cilindro, int *p_trilha, int *p_setor, int pos_bytes){
	int resto;
	*p_cilindro = (int)(pos_bytes/(TRILHAS_CILINDRO * SETORES_TRILHA * SETOR_BYTES));
	resto = pos_bytes % (TRILHAS_CILINDRO * SETORES_TRILHA * SETOR_BYTES);
	*p_trilha = (int)(resto/(SETORES_TRILHA * SETOR_BYTES));
	resto = resto % (SETORES_TRILHA * SETOR_BYTES);
	*p_setor = (int)(resto/SETOR_BYTES);

}

/*A funcao de escrita deve pedir um nome de arquivo .TXT na pasta onde
 esta sendo executado o programa e escrever no hd virtual(estrutura cilindro)*/
int write(){
	int p_cilindro, p_trilha, p_setor, p_cilindro_inicial;       //variaveis que indicam a posicao no cilindro
	int pos_setor = 0, pos_setor_aux = 0;   //index do setor na FAT
	int eof = 0;
	int tempo=0;
	int index_arquivo;
    char b[20];                           //buffer para ver se esta no fim do arquivo
    char nome_arquivo[100];
    FILE *arq;

    printf("Qual o nome do arquivo (.txt):\n");
    scanf("%s", nome_arquivo);

    arq = fopen(nome_arquivo, "rt");
    if(arq == NULL){
    	printf("Arquivo invalido\n");
    	return 0;
    }

	pos_setor = buscar_setor_disponivel();
	tempo += T_SEEK_MEDIO - T_SEEK_MIN; //desconta um seek minimo devido a soma extra dentro do while
	
	/*marco os 4 setores seguidos do pos_setor com used igual a 1
	e tambem indico o next(para onde cada setor aponta) de cada um deles*/
	for(int i = 0; i < CLUSTER_SETORES; i++){
		fat.setores[(pos_setor) + i].used = 1;
		if(i < CLUSTER_SETORES - 1)
			fat.setores[(pos_setor) + i].next = (pos_setor + i) + 1;
	}
	
	/*coloco na FAT o nome do arquivo e o primeiro setor dele*/
	//procura primeira posicao livre da lista de arquivos
	for(int i=0; i<MAX_ARQUIVOS; i++){
		if(fat.lista_arquivos[i].file_name[0] == '\0'){
			strcpy(fat.lista_arquivos[i].file_name, nome_arquivo);
			fat.lista_arquivos[i].first_sector = pos_setor;
			fat.total_arquivos++;
			index_arquivo = i;
			break;
		}
	}
	
   
	do{
		/*ver em qual cilindro, trilha e setor esta a posicao em bytes do pos_setor * 512*/
		calcularPos(&p_cilindro, &p_trilha, &p_setor, pos_setor * 512);
		if(p_cilindro != p_cilindro_inicial){ //se mudou de cilindro, adicionar T_SEEK_MIN
			p_cilindro_inicial = p_cilindro;
			tempo+=T_SEEK_MIN;
		}
		tempo += T_LATENCIA_MEDIA; //espero o setor ficar embaixo da cabeca de leitura
		/*escrevo 512 * CLUSTER_SETORES bytes do arquivo passado na estrutura cilindro*/
		for(int i = 0; i < CLUSTER_SETORES; i++){
			//obs.: e necessario ler um numero a menos do tamanho do 
			//setor, para guardar o delimitador, no caso 511
			fread(cilindros[p_cilindro].track[p_trilha].sector[p_setor + i].bytes_s, 511, 1, arq);
		}
		/*serve para ver se o arquivo ja esta no fim*/
		//eof = fscanf(arq, "%s", b);
		
		/*se nao estiver no final do arquivo eh necessario buscar um novo cluster*/
		if(!feof(arq)){
			pos_setor_aux = pos_setor + 3;    //recebe o ultimo setor do cluster atual
			/*se o cluster seguinte ao atual estiver sendo utilizado, procuro um outro
			atraves da funcao buscar_setor_disponivel*/
			if(fat.setores[pos_setor + 4].used == 1){
				pos_setor = buscar_setor_disponivel();
			}
			/*se o proximo cluster nao estiver sendo utilizado, utilizo ele*/
			else{
				pos_setor = (pos_setor + 4);
			}
					
			
			/*Verifica se ainda existe espaco livre no HD atraves da FAT*/
			if(pos_setor >= TOTAL_SETORES){
				printf("Espaço em disco insuficiente\n");
				fat.setores[pos_setor_aux].eof = 1;
				apagar_arquivo(index_arquivo); //faz mais sentido apagar o arquivo se ele nao cabe inteiro
				return 0;
			}

			/*faco o next do cluster anterior apontar para o inicio do atual*/
			fat.setores[pos_setor_aux].next = pos_setor;

			/*marco os 4 setores seguidos do pos_setor com used igual a 1 e tambem 
			indico o next(para onde cada setor aponta) de cada um deles*/
			for(int i = 0; i < CLUSTER_SETORES; i++){
				fat.setores[(pos_setor) + i].used = 1;
				if(i < CLUSTER_SETORES - 1)
					fat.setores[(pos_setor) + i].next = (pos_setor + i) + 1;
			}
		}
	} while(!feof(arq));
	/*indico que a escrita do arquivo acabou*/
	fat.setores[pos_setor + 3].eof = 1;

	/*calcular o tempo gasto*/
	
    printf("Tempo final: %dms\n", tempo);

	fclose(arq);	
	return 1;
}

/*recebe como parametro o first_sector do arquivo a ser escrito no SAIDA.txt
Essa funcao e responsavel por gravar o conteudo do arquivo no arquivo de texto
SAIDA.txt*/
void ler_arquivo(int pos_arquivo){
	int eof = 0;
	int tempo;
	int p_cilindro, p_trilha, p_setor;   //variaveis que indicam a posicao no cilindro
	FILE *arq;
	tempo += T_SEEK_MEDIO - T_SEEK_MIN; //desconta um seek minimo pro calculo dentro do while bater
	arq = fopen("SAIDA.TXT", "w");
	int p_cilindro_inicial;
	do{
		calcularPos(&p_cilindro, &p_trilha, &p_setor, pos_arquivo * 512);
		if(p_cilindro != p_cilindro_inicial){ //se mudou de cilindro, adicionar T_SEEK_MIN
			p_cilindro_inicial = p_cilindro;
			tempo+=T_SEEK_MIN;
		}
		tempo+=T_LATENCIA_MEDIA;
		/*Escreve um cluster no arquivo de saida*/
		tempo += T_TRANSF_TRILHA; //tempo de leitura da trilha
		for(int i = 0; i < CLUSTER_SETORES; i++){
			fputs(cilindros[p_cilindro].track[p_trilha].sector[p_setor + i].bytes_s, arq);
		}
		
		/*enquanto não acabar o arquivo, procuro o proximo cluster*/
		if(fat.setores[pos_arquivo + 3].eof != 1){
			pos_arquivo = fat.setores[pos_arquivo + 3].next;
		}
		else
			eof = 1;
			
	} while(eof == 0);
	/*calcular o tempo gasto*/
    printf("Tempo final: %dms\n", tempo);
	fclose(arq);
}

/*A funcao e responsavel por pedir o nome do arquivo para leitura,
ver se esse arquivo existe e se existir chamar a funcao ler_arquivo*/
int read(){
	char nome_arquivo[100];

	printf("Qual arquivo ler:\n");
	scanf("%s", nome_arquivo);

	/*Procura na lista de arquivos se o arquivo pedido existe, se
	sim retorna 1, caso contrario retorna 0*/
	for(int i = 0; i < fat.total_arquivos; i++){
		if(strcmp(fat.lista_arquivos[i].file_name, nome_arquivo) == 0){
			ler_arquivo(i);
			return 1;
		}
	}
	printf("Arquivo invalido\n");
	return 0;
}

void apagar_arquivo(int i){
	int eof = 0;
	int setor_atual = fat.lista_arquivos[i].first_sector;
	fat.lista_arquivos[i].file_name[0] = '\0';
	do{
		fat.setores[setor_atual].used = 0;
		if(fat.setores[setor_atual].eof != 1)
			setor_atual = fat.setores[setor_atual].next;
		else{
			eof = 1;
			fat.setores[setor_atual].eof = 0;
		}
	}while(eof == 0);
}

int erase(){
	char nome_arquivo[100];

	printf("Qual arquivo apagar? \n");
	scanf("%s", nome_arquivo);

	for(int i = 0; i < MAX_ARQUIVOS; i++){
		if(strcmp(fat.lista_arquivos[i].file_name, nome_arquivo) == 0){ //comparar essas duas palavras -> está dando erro
			apagar_arquivo(i);
			return 1;
		}
	}
	printf("Arquivo invalido\n");
	return 0;
}


int calcula_tamanho(int indice){
	int tamanho = 2; //ja contando o primeiro e o EOF
	int setor_atual = fat.lista_arquivos[indice].first_sector;
	while(fat.setores[setor_atual].eof != 1){
		tamanho++;
		setor_atual = fat.setores[setor_atual].next;
	}
	return tamanho * 512;
}

int show_FAT(){
	printf("NOME:\t\tTAMANHO EM DISCO:\tLOCALIZACAO:\n");
	
	//itera sobre todos as posicoes, mas para quando ja encontrou todos os arquivos
	for(int i=0, j=0; i< MAX_ARQUIVOS && j<fat.total_arquivos; i++){
		if(fat.lista_arquivos[i].file_name[0]!='\0'){
			j++; //encontrou um arquivo
			printf("%s\t",fat.lista_arquivos[i].file_name);
			printf("%d bytes\t\t", calcula_tamanho(i));

			int pos_leitura = fat.lista_arquivos[i].first_sector;
			while(fat.setores[pos_leitura].eof != 1){
				printf("%d ", pos_leitura);
				pos_leitura = fat.setores[pos_leitura].next;
			}
			printf("%d \n", pos_leitura); //o ultimo setor
		}
	}
	return 0;
}


/* Função contendo apenas o menu de operações, retorna 0 apenas para sair */
int menu(){
	int choice;
	printf("1 - Escrever Arquivo\n2 - Ler Arquivo\n3 - Apagar Arquivo\n4 - Mostrar Tabela FAT\n5 - Sair\n");
	scanf("%d",&choice);
	switch(choice){
		case(1):{
			write();
			break;
		}
		case(2):{
			read();
			break;
		}
		case(3):{
			erase();
			break;
		}
		case(4):{
			show_FAT();
			break;
		}
		case(5):{
			return 0;
		}
		default:{
			printf("opcao invalida\n");
			return 1;
		}
	}
	return 1;
}

//Função principal que chama o menu principal ate que a opcao SAIR seja selecionada
int main(){
	while(menu());
	return 0;
}	
