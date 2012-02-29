
#include <stdio.h>
#include <lib3270.h>

int main(int numpar, char *param[])
{
	H3270 *h;

	lib3270_initialize();

	h = lib3270_session_new("");
	printf("3270 session %p created\n",h);




	printf("Ending 3270 session %p\n",h);
	lib3270_session_free(h);

	return 0;
}
