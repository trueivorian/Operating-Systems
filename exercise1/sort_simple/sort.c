#include <stdio.h>
#include <string.h>

int main(int argc, char ** argv) {
	int i=0;
	int j=0;
	int count=0;
	char strList[20][20];
	char temp[20];

	for(i=0; i<(sizeof(strList[0])/sizeof(char)); i++){
		if(fgets(strList[i], sizeof(strList[i]), stdin)){
			count++;
		}

	}

	for(j=0; j<count-1; j++){
		for(i=j+1; i<count; i++){
			if(strcmp(strList[j], strList[i])>0){
				strcpy(temp, strList[j]); 
                strcpy(strList[j], strList[i]); 
                strcpy(strList[i], temp); 	
			}
		}
	}

	for(i=0; i<(sizeof(strList[0])/sizeof(char)); i++){
		printf("%s \n", strList[i]);
	}

	return 0;
}
	
	
