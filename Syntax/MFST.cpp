#include "../Common/stdadx.h"
#include "MFST.h"

#include <ostream>
#include <string_view>
#include <utility>

namespace MFST
{
    namespace
    {
        constexpr int invalidProgram = 200;
        constexpr std::size_t traceWidth = 25;

        struct SavedState
        {
            std::size_t position;
            std::vector<GRBALPHABET> stack;
            short nextChain;
            std::size_t ruleCount;
        };

        struct Diagnosis
        {
            std::size_t position;
            int code;
        };

        struct State
        {
            const std::vector<LT::Entry>& tokens;
            const GRB::Greibach& grammar;
            std::ostream* trace;
            std::vector<GRBALPHABET> stack;
            std::vector<SavedState> saved;
            std::vector<AppliedRule> rules;
            std::array<Diagnosis, 3> errors{};
            std::size_t errorCount = 0;
            std::size_t position = 0;
            std::size_t traceStep = 0;
            short nextChain = 0;
        };

        enum Step { Continue, Accepted, Rejected };

        GRBALPHABET tokenAt(const State& state, std::size_t position)
        {
            return position < state.tokens.size() ? state.tokens[position].lexema : state.grammar.end;
        }

        void writeColumn(std::ostream& output, std::string_view text, std::size_t width)
        {
            output << text;
            // Начальные байты UTF-8 дают ширину русских заголовков в символах.
            std::size_t length = 0;
            for (unsigned char byte : text)
            {
                if ((byte & 0xc0) != 0x80)
                    ++length;
            }
            for (; length < width; ++length)
                output.put(' ');
            output << " : ";
        }

        void writeRow(std::ostream& output, std::string_view step, std::string_view rule,
                      std::string_view tape, std::string_view stack)
        {
            writeColumn(output, step, 6);
            writeColumn(output, rule, 40);
            writeColumn(output, tape, 30);
            output << stack << '\n';
        }

        void writeTrace(const State& state, std::string_view rule = {})
        {
            if (!state.trace)
                return;

            char tape[traceWidth];
            char stack[traceWidth];
            std::size_t tapeSize = 0;
            std::size_t stackSize = 0;

            while (tapeSize < traceWidth && state.position + tapeSize <= state.tokens.size())
            {
                tape[tapeSize] = GRB::symbolChar(tokenAt(state, state.position + tapeSize));
                ++tapeSize;
            }

            while (stackSize < traceWidth && stackSize < state.stack.size())
            {
                stack[stackSize] = GRB::symbolChar(state.stack[state.stack.size() - 1 - stackSize]);
                ++stackSize;
            }

            writeRow(*state.trace, std::to_string(state.traceStep), rule,
                     {tape, tapeSize}, {stack, stackSize});
        }

        void saveError(State& state, int code)
        {
            std::size_t i = 0;
            while (i < state.errorCount && state.errors[i].position > state.position)
                ++i;

            if (i == state.errors.size() ||
                (i < state.errorCount && state.errors[i].position == state.position))
                return;

            if (state.errorCount < state.errors.size())
                ++state.errorCount;

            for (std::size_t j = state.errorCount - 1; j > i; --j)
                state.errors[j] = state.errors[j - 1];

            state.errors[i] = {state.position, code};
        }

        Step restore(State& state)
        {
            if (state.saved.empty())
                return Rejected;

            auto& saved = state.saved.back();
            state.position = saved.position;
            state.stack = std::move(saved.stack);
            state.nextChain = saved.nextChain;
            // Сбрасываем правила, применённые в отвергнутой ветке.
            state.rules.resize(saved.ruleCount);
            state.saved.pop_back();
            writeTrace(state, "RESSTATE");
            return Continue;
        }

        Step step(State& state)
        {
            const auto inputSize = state.tokens.size() + 1;
            if (state.position == inputSize && state.stack.empty())
                return Accepted;

            if (state.position == inputSize || state.stack.empty())
            {
                writeTrace(state, "Несогласованный конец разбора");
                saveError(state, invalidProgram);
                return restore(state);
            }

            const auto token = tokenAt(state, state.position);
            const auto top = state.stack.back();
            if (top > 0)
            {
                if (top == token)
                {
                    writeTrace(state);
                    state.stack.pop_back();
                    ++state.position;
                    state.nextChain = 0;
                    return Continue;
                }

                writeTrace(state, "TS_NOK/NS_NORULECHAIN");
                const int code = top == state.grammar.end || state.rules.empty()
                    ? invalidProgram
                    : state.grammar.rules[state.rules.back().rule].error;
                saveError(state, code);
                return restore(state);
            }

            const short ruleIndex = GRB::findRule(state.grammar, top);
            if (ruleIndex < 0)
            {
                writeTrace(state, "NS_ERROR");
                saveError(state, invalidProgram);
                return Rejected;
            }

            const auto& rule = state.grammar.rules[ruleIndex];
            const short chainIndex = GRB::findChain(rule, token, state.nextChain);
            if (chainIndex < 0)
            {
                writeTrace(state, "NS_NORULECHAIN");
                saveError(state, rule.error);
                return restore(state);
            }

            if (state.trace)
                writeTrace(state, GRB::ruleText(rule, chainIndex));

            const short next = GRB::findChain(rule, token, static_cast<short>(chainIndex + 1));
            if (next >= 0)
            {
                state.saved.push_back({state.position, state.stack, next, state.rules.size()});
                if (state.trace)
                    writeTrace(state, "SAVESTATE: " + std::to_string(state.saved.size()));
            }

            state.rules.push_back({state.position, ruleIndex, chainIndex});
            state.stack.pop_back();
            const auto& chain = rule.chains[chainIndex];
            // Кладём цепочку справа налево, чтобы первым сверить её начальный символ.
            for (short i = chain.size; i > 0; --i)
                state.stack.push_back(chain.symbols[i - 1]);

            writeTrace(state);
            state.nextChain = 0;
            return Continue;
        }
    }

    Result parse(const LT::LexTable& lexTable, const GRB::Greibach& grammar, std::ostream* trace)
    {
        State state{lexTable.table, grammar, trace, {grammar.end, grammar.start}, {}, {}};
        state.rules.reserve(state.tokens.size());

        if (trace)
            writeRow(*trace, "Шаг", "Правило", "Входная лента", "Стек");

        auto status = step(state);
        while (status == Continue)
        {
            ++state.traceStep;
            status = step(state);
        }

        Result result;
        result.accepted = status == Accepted;

        writeTrace(state, result.accepted ? "LENTA_END" : "Разбор прерван");
        if (trace)
        {
            *trace << std::string(100, '-') << '\n';
            if (result.accepted)
                *trace << "Синтаксический анализ выполнен без ошибок. Лексем: "
                       << state.tokens.size() << '\n';
            else
                *trace << "Синтаксический анализ завершён с ошибкой.\n";
        }

        if (result.accepted)
        {
            result.rules = std::move(state.rules);
            return result;
        }

        for (std::size_t i = 0; i < state.errorCount; ++i)
        {
            const auto& error = state.errors[i];
            int line = 1;
            int column = 1;
            if (!state.tokens.empty())
            {
                const auto& token = error.position < state.tokens.size()
                    ? state.tokens[error.position] : state.tokens.back();
                line = token.sn;
                column = token.pos;
            }
            result.errors[i] = Error::geterrorin(error.code, line, column);
        }

        return result;
    }

    void writeRules(const Result& result, const GRB::Greibach& grammar, std::ostream& output)
    {
        for (const auto& entry : result.rules)
            output << entry.position << ": " << GRB::ruleText(grammar.rules[entry.rule], entry.chain) << '\n';
    }
}
