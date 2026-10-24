#include "Parm.h"
#include "stdadx.h"
#include "Error.h"

namespace Parm
{
	PARM* GetParm(int argc, wchar_t* argv[])
	{
		PARM parm;
		parm.in[0] = L'\0';
		parm.log[0] = L'\0';
		parm.out[0] = L'\0';

		for (int i = 1; i < argc; i++)
		{
			if (wcslen(argv[i]) >= PARM_MAX_SIZE)
				throw ERROR_THROW(2);   // невозможно создать входной файл

			if (wcsstr(argv[i], PARM_IN) == argv[i])
				wcscpy(parm.in, argv[i] + wcslen(PARM_IN));
			if (wcsstr(argv[i], PARM_LOG) == argv[i])
				wcscpy(parm.in, argv[i] + wcslen(PARM_LOG));
			if (wcsstr(argv[i], PARM_OUT) == argv[i])
				wcscpy(parm.in, argv[i] + wcslen(PARM_OUT));
		}

		if (wcslen(parm.in) == 0)
			throw ERROR_THROW(1)   // не указан обязательный параметр -in

		if (wcslen(parm.log) == 0)
		{
			wcscpy(parm.log, parm.in);
			wcsncat(parm.log, PARM_LOG_DEFAULT_EXT, _TRUNCATE);
		}

		if (wcslen(parm.out) == 0)
		{
			wcscpy(parm.out, parm.in);
			wcsncat(parm.out, PARM_LOG_DEFAULT_EXT, _TRUNCATE);
		}
	}
}