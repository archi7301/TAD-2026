#pragma once

#define PARM_IN L"-in"
#define PARM_LOG L"-log"
#define PARM_OUT L"-out"
#define PARM_MAX_SIZE 100
#define PARM_LOG_DEFAULT_EXT L".log"
#define PARM_OUT_DEFAULT_EXT L".js"

namespace Parm
{
	struct PARM
	{
		wchar_t in[PARM_MAX_SIZE];
		wchar_t log[PARM_MAX_SIZE];
		wchar_t out[PARM_MAX_SIZE];
	};

	PARM* GetParm(int argc, wchar_t* argv[]);
}