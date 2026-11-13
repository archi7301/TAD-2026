#include "../Common/stdadx.h"
#include "../Common/Error.h"
#include "../Common/Parm.h"
#include "../Common/In.h"
#include "../Common/Log.h"
#include "../Common/Out.h"

static void PrintError(const Error::Entry& e)
{
	printf("%s\n", Error::ToString(e).c_str());
}

int wmain(int argc, wchar_t* argv[])
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	Log::LOG log;       // создаётся закрытым; все функции записи при этом ничего не делают
	int result = 0;

	try
	{
		Parm::PARM parm = Parm::GetParm(argc, argv);   // ошибки 1, 5, 6: только в консоль

		log = Log::Open(parm.log);                     // ошибка 3: только в консоль
		Log::WriteHeader(log, parm);

		In::IN in = In::GetIn(parm.in);                // ошибка 2: в консоль и в протокол
		Log::WriteSource(log, in);

		if (!in.errors.empty())
		{
			Log::WriteSection(log, LOG_SECT_ERRORS);
			for (const Error::Entry& e : in.errors)
			{
				PrintError(e);
				Log::WriteError(log, e);
			}
			Log::Flush(log);
			result = 1;
		}
	}

	catch (const Error::Entry& e)
	{
		PrintError(e);
		result = 1;

		// При ошибке самого протокола повторно писать в него не нужно.
		if (e.id != 3)
		{
			try
			{
				Log::WriteSection(log, LOG_SECT_ERRORS);
				Log::WriteError(log, e);
				Log::Flush(log);
			}
			catch (const Error::Entry& log_error)
			{
				PrintError(log_error);
			}
		}
	}

	Log::Close(log);
	return result;
}