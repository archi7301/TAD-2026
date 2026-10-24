#include "../Common/stdadx.h"
#include "../Common/Error.h"
#include "../Common/Parm.h"

int wmain(int argc, wchar_t* argv[])
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	try
	{
		Parm::PARM parm = Parm::GetParm(argc, argv);
	}
	catch (const Error::Entry& e)
	{
		printf("Ошибка %d: %s", e.id, e.message);
		if (e.inext.line != -1 && e.inext.col != -1)
			printf(" (строка %d, позиция %d)", e.inext.line, e.inext.col);
		printf("\n");
		return 1;
	}

	return 0;
}