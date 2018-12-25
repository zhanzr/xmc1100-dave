#include <stdio.h>
#include <stdint.h>

void modi_ptr(int* pD)
{
	(*pD) ++;
}

void modi_ptr2(int pD[])
{
	pD[0]++;
}

int main(int argc, char** argv)
{
	register int ai = 10;
	int bi = 100;
	int* pI = &bi;

	printf("%s \n %d %d %p\n", __FILE__, ai, *pI, pI);
	modi_ptr(pI);
	printf("%s \n %d %d %p\n", __FILE__, ai, *pI, pI);

	register int arrA[] = {1, 2, 3};
	int arrB[] = {10, 20, 30};
	modi_ptr(arrB);
	printf("%d %d %d\n", arrB[0], arrB[1], arrB[2]);
	modi_ptr(arrA);
	printf("%d %d %d\n", arrA[0], arrA[1], arrA[2]);

	modi_ptr2(arrB);
	printf("%d %d %d\n", arrB[0], arrB[1], arrB[2]);
	modi_ptr2(arrA);
	printf("%d %d %d\n", arrA[0], arrA[1], arrA[2]);

	return 0;
}


