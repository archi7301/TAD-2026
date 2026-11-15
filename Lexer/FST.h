#pragma once
#include "../Common/stdadx.h"

namespace FST
{
    // Служебные обозначения классов символов в переходах
    constexpr char ANY_LETTER = '\x01';
    constexpr char ANY_DIGIT = '\x02';
    constexpr char STRING_CHAR = '\x03';

    // Переход по символу в другое состояние автомата
    struct Relation
    {
        char symbol;    // символ, разрешающий переход
        size_t next;    // индекс состояния назначения
    };

    // Конечный автомат. Начальное состояние имеет индекс 0
    struct FST
    {
        // states[i] содержит все исходящие переходы состояния i
        std::vector<std::vector<Relation>> states;

        // isFinal[i] == true: токен принимается, если его чтение завершилось в состоянии i
        std::vector<bool> isFinal;
    };

    // Создаёт stateCount состояний без переходов
    // Изначально ни одно состояние не является допускающим
    FST Create(size_t stateCount);

    // Добавляет переход из from в to по символу symbol
    // Оба индекса должны обозначать существующие состояния
    void AddRelation(FST& fst, size_t from, char symbol, size_t to);

    // Помечает существующее состояние как допускающее
    void SetFinal(FST& fst, size_t state);

    // Строит автомат, принимающий только заданное слово
    FST MakeWord(const std::string& word);

    // Возвращает true, если автомат принимает весь переданный текст
    bool Execute(const FST& fst, const std::string& text);
}