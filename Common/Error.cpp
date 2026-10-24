#include "Error.h"

namespace Error
{
    ERROR errors[ERROR_MAX_ENTRY] =
    {
        ERROR_ENTRY(0, "Недопустимый код ошибки!"),
        ERROR_ENTRY(1, "Не указан обязательный параметр -in!"),
        ERROR_ENTRY(2, "Невозможно создать входной файл!"),
        ERROR_ENTRY(3, "Невозможно создать файл протокола!"), 
        ERROR_ENTRY(4, "Невозможно создать выходной файл!"), 
        ERROR_ENTRY(5, "Неизвестный параметр командной строки!"),
        ERROR_ENTRY_NODEF(6), ERROR_ENTRY_NODEF(7), ERROR_ENTRY_NODEF(8), ERROR_ENTRY_NODEF(9), ERROR_ENTRY_NODEF10(10), 
        ERROR_ENTRY(100, "Недопустимый символ в исходном тексте!"),
        ERROR_ENTRY(101, "Не закрыт строковый литерал!"), 
        ERROR_ENTRY(102, "Лексема не распознана!"),
        ERROR_ENTRY(103, "Превышена длина идентификатора!"),
        ERROR_ENTRY(104, "Превышена длина строкового литерала!"),
        ERROR_ENTRY_NODEF(105), ERROR_ENTRY_NODEF(106), ERROR_ENTRY_NODEF(107), ERROR_ENTRY_NODEF(108), ERROR_ENTRY_NODEF(109), 
        ERROR_ENTRY_NODEF(110), ERROR_ENTRY_NODEF(111), ERROR_ENTRY_NODEF(112), ERROR_ENTRY_NODEF(113), ERROR_ENTRY_NODEF(114),
        ERROR_ENTRY(200, "Неверная структура программы!"),
        ERROR_ENTRY(201, "Ошибка в заголовке функции!"),
        ERROR_ENTRY(202, "Ошибка в заголовке процедуры!"),
        ERROR_ENTRY(203, "Ошибка в списке параметров!"),
        ERROR_ENTRY(204, "Ошибка в выражении!"),
        ERROR_ENTRY(205, "Ошибка в инструкции!"),
        ERROR_ENTRY(206, "Ошибка в конструкции cycle ... until!"),
        ERROR_ENTRY(207, "Ошибка в конструкции when ... then ... else!"),
        ERROR_ENTRY(208, "Ошибка в списке аргументов!"),  ERROR_ENTRY_NODEF(209), ERROR_ENTRY_NODEF10(210), ERROR_ENTRY_NODEF10(220), 
        ERROR_ENTRY_NODEF10(230), ERROR_ENTRY_NODEF10(240),  ERROR_ENTRY_NODEF10(250), ERROR_ENTRY_NODEF10(260), 
        ERROR_ENTRY_NODEF10(270), ERROR_ENTRY_NODEF100(280),  ERROR_ENTRY_NODEF100(290), 
        ERROR_ENTRY(300, "Отсутсвует точка входа main!"),
        ERROR_ENTRY(301, "Обнаружено более одной точки входа main!"),
        ERROR_ENTRY(302, "Повторное объявление идентификатора!"),
        ERROR_ENTRY(303, "Необъявленный идентификатор!"),
        ERROR_ENTRY(304, "Блок вызывается до своего объявления!"),
        ERROR_ENTRY(305, "Функция библиотеки используется без директивы summon lore!"),
        ERROR_ENTRY(306, "Переопределение имени функции библиотеки!"),
        ERROR_ENTRY(307, "Типы операндов не соответствуют операции!"),
        ERROR_ENTRY(308, "Тип выражения не соответствует типу данных!"),
        ERROR_ENTRY(309, "Неверное число или типы аргументов!"),
        ERROR_ENTRY(310, "Тип результата не соответствует объявленному!"),
        ERROR_ENTRY(311, "Присваивание неизменяемому значению!"),
        ERROR_ENTRY(312, "Условие должно иметь тип truth!"),
        ERROR_ENTRY(313, "Ветви when имеют разные типы!"),
        ERROR_ENTRY(314, "Вызов процедуры в выражении!"),
        ERROR_ENTRY(315, "Значение литерала превышает 255!"),
        ERROR_ENTRY(316, "Ошибка в конвейере: функция не подходит!"),
        ERROR_ENTRY(317, "Вызов процедуры main недопустим!"),
        ERROR_ENTRY_NODEF(318), ERROR_ENTRY_NODEF(319), ERROR_ENTRY_NODEF(320), ERROR_ENTRY_NODEF10(330), ERROR_ENTRY_NODEF10(340),
        ERROR_ENTRY_NODEF10(350), ERROR_ENTRY_NODEF10(360), ERROR_ENTRY_NODEF10(370), ERROR_ENTRY_NODEF10(380), ERROR_ENTRY_NODEF10(390),
        ERROR_ENTRY_NODEF100(400), ERROR_ENTRY_NODEF100(500), ERROR_ENTRY_NODEF100(600), ERROR_ENTRY_NODEF100(700), 
        ERROR_ENTRY_NODEF100(800)
    };


    ERROR geterror(int id)
    {
        if (id > 0 && id < ERROR_MAX_ENTRY)
            return errors[id];
        return errors[0];
    }

    ERROR geterrorin(int id, int line, int col)
    {
        ERROR err = geterror(id);
        err.inext.line = static_cast<short>(line);
        err.inext.col = static_cast<short>(col);
        return err;
    }
}