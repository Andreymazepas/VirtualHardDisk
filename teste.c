#include <stdio.h>

int main(){
	char char1[20];
	char char2[20];
	char char3[20];
	FILE *arq;

	arq = fopen("texto.txt", "rt");
    if(arq == NULL)
    	return 0;

    fgets(char1, 10, arq);
    fgets(char2, 10, arq);
    fgets(char3, 10, arq);
    printf("%s\n",char1);
    printf("%s\n",char2);
    printf("%s\n",char3);
    fclose(arq);
}