
#include <lib3270.h>

int main(int numpar, char *param[])
{
	H3270 *h;

	h = lib3270_session_new("");




	lib3270_session_free(h);

	return 0;
}
