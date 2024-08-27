/*
 * quicksort.c
 *
 * Created: 2019-03-14 9:20:27
 *  Author: erikw_000
 */ 

// https://www.goodreads.com/book/show/27862.Algorithms_in_C_Parts_1_4

// "This code is from "Algorithms in C, Third Edition,"
// by Robert Sedgewick, Addison-Wesley, 1998."

typedef uint8_t Item;

#define key(A) (A)
#define less(A, B) (key(A) < key(B))
//#define exch(A, B) {Item t = A; A = B; B = t;} // This function is hijacked to check the stack pointer value

//#define GET_MAX_STACK_POINTER
#ifdef GET_MAX_STACK_POINTER
static int max_sp = 0;
#define exch(A, B) {Item t = A; register int sp asm ("sp"); max_sp = (sp > max_sp)? sp : max_sp; A = B; B = t;}
#else
#define exch(A, B) {Item t = A; A = B; B = t;}
#endif

int partition(Item a[], int l, int r);
void quicksort(Item a[], int l, int r)
{
	int i;
	if (r <= l) return;
	i = partition(a, l, r);
	quicksort(a, l, i-1);
	quicksort(a, i+1, r);
}

int partition(Item a[], int l, int r)
{ 
	int i = l-1, j = r; Item v = a[r];
	for (;;)
	{
		while (less(a[++i], v)) ;
		while (less(v, a[--j])) if (j == l) break;
		if (i >= j) break;
		exch(a[i], a[j]);
	}
	exch(a[i], a[r]);
	return i;
}