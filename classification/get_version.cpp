#include "tengine_c_api.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	printf("Tengine Version : %s\n",get_tengine_version());
	return 0;
}

