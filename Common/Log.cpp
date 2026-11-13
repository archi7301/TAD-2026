#include "stdadx.h"
#include "Log.h"
#include "Error.h"

namespace Log
{
	// Переводит широкую строку в UTF-8. Нужна только для вывода путей в текст
	// протокола; при ошибке возвращает пустую строку.
	static std::string wstr_to_utf8(const wchar_t* str)
	{
		if (str == nullptr || str[0] == L'\0')
			return std::string();

		int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
		if (size <= 1)
			return std::string();

		std::string result(size - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, str, -1, &result[0], size, nullptr, nullptr);
		return result;
	}

	static std::string source_to_utf8(const std::string& text)
	{
		if (text.empty())
			return {};

		const int source_size = static_cast<int>(text.size());

		const int wide_size = MultiByteToWideChar(
			1251,
			0,
			text.data(),
			source_size,
			nullptr,
			0
		);

		if (wide_size == 0)
			throw ERROR_THROW(3);

		std::wstring wide(wide_size, L'\0');

		if (MultiByteToWideChar(
			1251,
			0,
			text.data(),
			source_size,
			wide.data(),
			wide_size) == 0)
		{
			throw ERROR_THROW(3);
		}

		const int utf8_size = WideCharToMultiByte(
			CP_UTF8,
			0,
			wide.data(),
			wide_size,
			nullptr,
			0,
			nullptr,
			nullptr
		);

		if (utf8_size == 0)
			throw ERROR_THROW(3);

		std::string result(utf8_size, '\0');

		if (WideCharToMultiByte(
			CP_UTF8,
			0,
			wide.data(),
			wide_size,
			result.data(),
			utf8_size,
			nullptr,
			nullptr) == 0)
		{
			throw ERROR_THROW(3);
		}

		return result;
	}

	// Текущее локальное время в виде «дд.мм.гггг чч:мм:сс»
	static std::string CurrentTime()
	{
		std::time_t now = std::time(nullptr);
		std::tm info;
		if (localtime_s(&info, &now) != 0)
			return "неизвестно";

		char buff[32];
		if (std::strftime(buff, sizeof(buff), "%d.%m.%Y %H:%M:%S", &info) == 0)
			return "неизвестно";
		return buff;
	}

	// Линия-разделитель. При закрытом потоке ничего не делает.
	static void WriteSeparator(std::ofstream& file)
	{
		if (!file.is_open())
			return;
		file << std::string(LOG_LINE_WIDTH, '-') << "\n";
	}

	LOG Open(const wchar_t* path)
	{
		LOG log;
		log.file.open(path, std::ios::out | std::ios::trunc);
		if (!log.file.is_open())
			throw ERROR_THROW(3);
		return log;
	}

	void Close(LOG& log)
	{
		if (log.file.is_open())
			log.file.close();
	}

	void WriteLine(LOG& log, const char* text)
	{
		if (!log.file.is_open())
			return;
		log.file << text << "\n";
	}

	void Flush(LOG& log)
	{
		if (!log.file.is_open())
			return;

		log.file.flush();

		if (!log.file)
			throw ERROR_THROW(3);
	}

	void WriteSection(LOG& log, const char* title)
	{
		WriteLine(log, title);
		WriteSeparator(log.file);
		WriteLine(log, "");
	}

	void WriteHeader(LOG& log, const Parm::PARM& parm)
	{
		if (!log.file.is_open())
			return;

		WriteLine(log, LOG_TITLE);
		WriteLine(log, ("Дата и время запуска: " + CurrentTime()).c_str());
		WriteLine(log, "");
		WriteLine(log, "Параметры:");
		WriteLine(log, ("  Входной файл:      " + wstr_to_utf8(parm.in)).c_str());
		WriteLine(log, ("  Выходной файл:     " + wstr_to_utf8(parm.out)).c_str());
		WriteLine(log, ("  Файл протокола:    " + wstr_to_utf8(parm.log)).c_str());
		WriteLine(log, parm.run ? "  Выполнение (-run): да" : "  Выполнение (-run): нет");
		WriteSeparator(log.file);
		WriteLine(log, "");
		Flush(log);
	}

	void WriteSource(LOG& log, const In::IN& in)
	{
		if (!log.file.is_open())
			return;

		WriteSection(log, LOG_SECT_SOURCE);

		const std::string source = source_to_utf8(in.original_text);

		const int width = static_cast<int>(std::to_string(in.lines).size());

		size_t start = 0;
		int number = 1;

		while (true)
		{
			const size_t end = source.find('\n', start);
			const bool last = end == std::string::npos;

			std::string piece = source.substr(
				start,
				last ? std::string::npos : end - start
			);

			// CRLF выводим как обычный перевод строки протокола.
			if (!piece.empty() && piece.back() == '\r')
				piece.pop_back();

			if (last && piece.empty() && number > 1)
				break;

			log.file << std::setw(width)
				<< number
				<< " | "
				<< piece
				<< '\n';

			if (last)
				break;

			start = end + 1;
			++number;
		}

		WriteLine(log, "");
		Flush(log);
	}

	void WriteError(LOG& log, const Error::Entry& e)
	{
		WriteLine(log, Error::ToString(e).c_str());
	}
}