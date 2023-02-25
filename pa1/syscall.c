#include <sys/syscall.h>
#include <syscall.h>
#include <stdio.h>
#include <unistd.h>

// long int x;
int main() {
	char longerThan32[] = "This string is going to be longer than 32 characters lalalalalala";
	printf("string before syscall: %s\n", longerThan32);
	long ret1 = syscall(548, longerThan32);
	printf("return value of syscall: %ld\n", ret1);
	printf("string after syscall: %s\n", longerThan32);

	char shorterThan32[] = "shorter than 32";
	printf("string before syscall: %s\n", shorterThan32);
	long ret2 = syscall(548, shorterThan32);
	printf("return value of syscall: %ld\n", ret2);
	printf("string after syscall: %s\n", shorterThan32);
}
