#include "../Common/stdadx.h"
#include "../Common/Error.h"
#include "../Common/Parm.h"
#include "../Common/In.h"

static void PrintError(const Error::Entry& e)
{
	printf("Ошибка %d: %s", e.id, e.message);
	if (e.inext.line != -1 && e.inext.col != -1)
		printf(" (строка %d, позиция %d)", e.inext.line, e.inext.col);
	printf("\n");
}

int wmain(int argc, wchar_t* argv[])
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	try
	{
		Parm::PARM parm = Parm::GetParm(argc, argv);
		In::IN in = In::GetIn(parm.in);

		if (!in.errors.empty())
		{
			for (const Error::Entry& e : in.errors)
				PrintError(e);
			return 1;
		}

		printf("%s", in.text.c_str());
	}

	catch (const Error::Entry& e)
	{
		PrintError(e);
		return 1;
	}

	return 0;
}