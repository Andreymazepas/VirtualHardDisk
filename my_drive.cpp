#include <bits/stdc++.h>
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

typedef struct block { unsigned char bytes_s[512]; } block;
typedef struct sector_array { block sector[60]; } sector_array;
typedef struct track_array { sector_array track[5]; } track_array;
typedef struct fatlist_s {
char file_name[100];
unsigned int first_sector;
} fatlist;

typedef struct fatent_s {
unsigned int used;
unsigned int eof;
unsigned int next;
} fatent;


int write(int n){
	printf("%d\n",n);
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

int menu(){
	int choice;
	printf("1 - Escrever Arquivo\n2 - Ler Arquivo\n3 - Apagar Arquivo\n4 - Mostrar Tabela FAT\n5 - Sair\n");
	scanf("%d",&choice);
	switch(choice){
		case(1):{
			write(256);
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

int main(){
	while(menu());
	return 0;
}	
