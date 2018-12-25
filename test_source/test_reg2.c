#include <stdio.h>
#include <stdint.h>

int main(int argc, char** argv)
{
	register int ai = 10;
	int* pI = &ai;

	printf("%s \n %d %d %p\n", __FILE__, ai, *pI, pI);

	return 0;
}

