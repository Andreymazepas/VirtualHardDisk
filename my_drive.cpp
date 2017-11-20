/* Trabalho final da disciplina de Organização de Arquivos
	UNB 2017
	Andrey Emmanuel Matrosov Mazépas - 16/0112362
	Cristiane  seu sobrenome - sua matricula

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


#include <bits/stdc++.h>
#include <fstream>
#include <unistd.h>

//usleep utiliza microsegundos
#define T_SEEK_MEDIO 4000
#define T_SEEK_MIN 1000
#define T_LATENCIA_MEDIA 6000
#define T_TRANSF_TRILHA 12000
#define SETORES_TRILHA 60
#define TRILHAS_SUPERF 10
#define TRILHAS_CILINDRO 5
#define SETOR_BYTES 512
#define CLUSTER_SETORES 4
#define CLUSTER_BYTES 2048

typedef struct block { unsigned char bytes_s[512]; } block; //menor unidade de armazenamento
typedef struct sector_array { block sector[60]; } sector_array; //uma trilha, 60 setores com 1 bloco cada (?)
typedef struct track_array { sector_array track[5]; } track_array; //um cilindro
typedef struct fatlist_s {
char file_name[100];
unsigned int first_sector;
} fatlist; //conteudo da tabela FAT contendo nome e primeiro setor do arquivo

typedef struct fatent_s {
unsigned int used;
unsigned int eof;
unsigned int next;
} fatent; //flags que identificam cada setor

//sao declarados um ponteiro que vai apontar pro cilindro sendo utilizado e todos os 5 cilindros
track_array *cylinder, cylinder1,cylinder2,cylinder3,cylinder4,cylinder5;

//A funcao de escrita deve pedir um nome de arquivo .TXT na pasta onde esta sendo executado o programa e escrever no hd virtual
int write(){
	std::string NOME_ARQUIVO;
	std::ifstream ARQUIVO;
	printf("Digite o nome do arquivo terminado em .txt da pasta raiz do programa:\n");
	std::cin >> NOME_ARQUIVO;
	std::cin.ignore(); //limpa o buffer de entrada
	
	std::cout << "Abrindo arquivo: " << NOME_ARQUIVO << std::endl;
	ARQUIVO.open(NOME_ARQUIVO.c_str(), std::fstream::in);

	if (!ARQUIVO){
		std::cout << "Nao foi possivel abrir o arquivo " << NOME_ARQUIVO << ", verifique o nome ou se o arquivo esta aberto em outro programa." << std::endl;
		std::cout << "Pressione Enter para voltar ao menu principal" << std::endl;
		std::cin.get();
		return 0; //volta pro menu principal
	}

	std::cout << "Arquivo aberto com sucesso." << std::endl;

	char ch;
	while( ARQUIVO >> std::noskipws >> ch) //noskipws: no skip white space, nao ignora os espacos em branco do arquivo
		std::cout << ch;
	std::cout << std::endl;
	
	cylinder = &cylinder1; 

	ARQUIVO.close();
	std::cout << "Pressione Enter para voltar ao menu principal" << std::endl;
	std::cin.get();
	return 0;
}

int read(){
	usleep(1000000);
	printf("lido\n");
	return 0;
}

int erase(){
	printf("apagado\n");
}

int show_FAT(){
	printf("NOME:\tTAMANHO EM DISCO:\tLOCALIZACAO\n");
	printf("ARQUIVO1.TXT\t2048 Bytes\t0,1,2,3\n");
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
