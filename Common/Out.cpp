#include "stdadx.h"
#include "Out.h"
#include "Error.h"

namespace Out
{
	static void append_uint32_le(std::vector<unsigned char>& buff, unsigned int value)
	{
		buff.push_back((unsigned char)(value & 0xFF));
		buff.push_back((unsigned char)((value >> 8) & 0xFF));
		buff.push_back((unsigned char)((value >> 16) & 0xFF));
		buff.push_back((unsigned char)((value >> 24) & 0xFF));
	}

	unsigned int AddString(IMAGE& image, const std::string& str)
	{
		unsigned int offset = static_cast<unsigned int>(image.consts.size());
		image.consts.push_back(static_cast<unsigned char>(str.size()));
		image.consts.insert(image.consts.end(), str.begin(), str.end());
		return offset;
	}

	void Write(const wchar_t* path, const IMAGE& image)
	{
		std::vector<unsigned char> buff;
		for (int i = 0; i < TBC_SIGNATURE_SIZE; i++)
			buff.push_back(static_cast<unsigned char>(TBC_SIGNATURE[i]));

		append_uint32_le(buff, image.entry);
		append_uint32_le(buff, static_cast<unsigned int>(image.consts.size()));
		append_uint32_le(buff, static_cast<unsigned int>(image.code.size()));

		FILE* file = nullptr;
		bool ok = true;
		if (_wfopen_s(&file, path, L"wb") != 0 || file == nullptr)
			throw ERROR_THROW(4);

		if (fwrite(buff.data(), 1, buff.size(), file) != buff.size())
			ok = false;

		if (fwrite(image.consts.data(), 1, image.consts.size(), file) != image.consts.size())
			ok = false;

		if (fwrite(image.code.data(), 1, image.code.size(), file) != image.code.size())
			ok = false;

		if (fclose(file) != 0)
			ok = false;

		if (!ok)
		{
			_wremove(path);
			throw ERROR_THROW(4);
		}
	}
}