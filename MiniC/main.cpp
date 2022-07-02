#include "CLI.h"

//#define _Debug
int main(int argc, char *argv[])
{
#ifdef _Debug
	if (argc < 2)
	{
		char *tmp[] = {"mcc.exe", "-d", "D:\\MyProjects\\vscode\\cpp\\MiniC\\bin\\test.mc.ir", "-z"};
		argc = 4;
		argv = tmp;
	}
#endif
	CLI cmd;
	cmd.Parse(argc, argv);
	return 0;
}