#include "stdadx.h"
#include "In.h"
#include "Error.h"

namespace In
{
	enum Cls { F, T, S, R, I };
	enum State { ST_NORMAL, ST_STRING, ST_COMMENT };

	static Cls table[256];

	static void FillTable()
	{
		for (size_t i = 0; i < 256; i++)
			table[i] = F;

		for (char ch = 'a'; ch <= 'z'; ch++) table[(unsigned char)ch] = T;
		for (char ch = '0'; ch <= '9'; ch++) table[(unsigned char)ch] = T;

		const char special_T[] = "+-*<>=!~():,;'_ ";
		for (size_t i = 0; special_T[i] != '\0'; i++)
			table[(unsigned char)special_T[i]] = T;

		table[0x09] = T;  // tab
		table[0x0A] = T;  // LF
		table[0x0D] = I;  // CR

		for (size_t i = 0x20; i <= 0x7E; i++)
			if (table[i] == F)
				table[i] = S;

		table[0xA8] = R;
		table[0xB8] = R;
		for (size_t i = 0xC0; i <= 0xFF; i++)
			table[i] = R;
	}

	static std::string ReadFile(const wchar_t* path)
	{
		FILE* file = nullptr;
		if (_wfopen_s(&file, path, L"rb") != 0 || file == nullptr)
			throw ERROR_THROW(2);

		std::string text;
		const size_t buff_size = 4096;
		char buf[buff_size];
		size_t bytes_read = 0;

		while ((bytes_read = fread(buf, 1, buff_size, file)) > 0)
			text.append(buf, bytes_read);

		fclose(file);
		return text;
	}

	static void AddError(IN& in, int line, int col)
	{
		if (in.errors.size() < IN_MAX_ERRORS)
			in.errors.push_back(ERROR_THROW_IN(100, line, col));
	}

	IN GetIn(const wchar_t* path)
	{
		FillTable();
		std::string raw = ReadFile(path);

		IN in;
		in.lines = 1;
		State state = ST_NORMAL;
		int line = 1;
		int col = 0;

		for (size_t i = 0; i < raw.size(); i++)
		{
			unsigned char ch = (unsigned char)raw[i];

			if (ch == 0x0D)             // CR
				continue;

			if (ch == 0x0A)             // LF
			{
				in.text += '\n';
				line++;
				col = 0;
				state = ST_NORMAL;      // строка и комментарий не переходят на новую строку
				continue;
			}

			col++;

			switch (state)
			{
			case ST_NORMAL:
				if (ch == '-' && i + 1 < raw.size() && raw[i + 1] == '-')
					state = ST_COMMENT;
				else if (table[ch] == T)
				{
					in.text += (char)ch;
					if (ch == '\'')
						state = ST_STRING;
				}
				else
					AddError(in, line, col);
				break;

			case ST_STRING:
				if (ch >= 0x20 && ch <= 0x7E)
				{
					in.text += (char)ch;
					if (ch == '\'')
						state = ST_NORMAL;
				}
				else
					AddError(in, line, col);
				break;

			case ST_COMMENT:
				if (table[ch] == F)
					AddError(in, line, col);
				break;
			}
		}

		in.lines = line;
		return in;
	}
}