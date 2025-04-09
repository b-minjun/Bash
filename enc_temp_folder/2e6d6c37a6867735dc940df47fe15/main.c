#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

char username[100];
char password[100];
char hostname[100];
char prompt[100];

int main(void) {
	printf("username : ");
	scanf("%s", username);
	printf("password : ");
	scanf("%s", password);
	printf("hostname : ");
	scanf("%s", hostname);
	while (1){
		printf("%s@%s:$ ",username,hostname);
		scanf("%s", prompt);
		if (prompt == "exit") {
			break;
		}
	}
}