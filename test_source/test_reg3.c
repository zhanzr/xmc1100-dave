#include <stdio.h>
#include <stdint.h>

int main(int argc, char** argv)
{
	static register int ai = 10;
	int bi = 100;
	int* pI = &bi;

	printf("%s \n %d %d %p\n", __FILE__, ai, *pI, pI);

	return 0;
}

