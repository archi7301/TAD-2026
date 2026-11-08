#pragma once
#include "stdadx.h"
#include "Bytecode.h"

namespace IT // таблица идентификаторов (ТИ)
{
    constexpr size_t ID_MAXSIZE = 16;       // максимальная длина идентификатора в языке
    constexpr int    TI_NULLIDX = -1;       // нет элемента ТИ

    enum class ID_DATA_TYPE { NONE, OCTET, TRUTH, STR };                            // типы данных
    enum class ID_TYPE { FUNC, PROC, PARAM, RUNE, FLUX, LIB, LITERAL, UNKNOWN };    // виды идентификаторов

    struct Entry // строка ТИ
    {
        std::string  name;
        std::string  scope;
        std::string  str;                                // текст строкового литерала
        int          idx_first_LE = TI_NULLIDX;          // индекс первой строки в ТЛ
        long long    value = 0;                          // значение (octet, yes=1, no=0)
        ID_DATA_TYPE id_data_type = ID_DATA_TYPE::NONE;  // тип данных
        ID_TYPE      id_type = ID_TYPE::UNKNOWN;         // вид идентификатора
    };

    struct IdTable // экземпляр ТИ
    {
        std::vector<Entry> table;
    };

    IdTable Create(size_t reserve_size = 256);                                              // создать ТИ (с резервированием памяти)
    void Add(IdTable& idtable, Entry entry);                                                // добавить строку в ТИ (через std::move)
    const Entry& GetEntry(const IdTable& idtable, int n);                                   // получить строку ТИ

    int IsId(const IdTable& idtable, const std::string& scope, const std::string& name);    // поиск в одной области
    int FindId(const IdTable& idtable, const std::string& scope, const std::string& name);  // поиск вверх по областям (main/1 -> main -> -)
    int FindLiteral(const IdTable& idtable, ID_DATA_TYPE type, long long value);            // поиск числового/логического литерала
    int FindLiteral(const IdTable& idtable, const std::string& str);                        // поиск строкового литерала
}
