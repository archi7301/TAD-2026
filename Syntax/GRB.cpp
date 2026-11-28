#include "../Common/stdadx.h"
#include "GRB.h"
#include "../Common/LT.h"

#include <algorithm>
#include <array>
#include <map>
#include <utility>
#include <vector>

namespace GRB
{
    namespace
    {
        using Symbol = GRBALPHABET;
        using Production = std::vector<Symbol>;
        using Alternatives = std::vector<Production>;

        constexpr Symbol T(char code) { return code; }
        constexpr Symbol N(char code) { return -code; }

        enum Nonterminal : Symbol
        {
            Program = N('S'),
            Block = N('B'),
            FunctionHeader = N('F'),
            ProcHeader = N('R'),
            Params = N('P'),
            ParamTail = N('p'),
            ProcBody = N('I'),
            ProcTail = N('i'),
            Binding = N('V'),
            AfterInstructionName = N('A'),
            Output = N('O'),
            LoopBody = N('L'),
            LoopTail = N('l'),
            Args = N('G')
        };

        enum SyntaxError
        {
            InvalidProgram = 200,
            InvalidFunc = 201,
            InvalidProc = 202,
            InvalidParams = 203,
            InvalidExpression = 204,
            InvalidInstruction = 205,
            InvalidLoop = 206,
            InvalidConditional = 207,
            InvalidArgs = 208
        };

        struct RuleData
        {
            int error;
            Alternatives alternatives;
        };

        using Grammar = std::map<Symbol, RuleData>;

        enum class End
        {
            Func, Instruction, Loop, Then, Else, Group, Arg, Count
        };

        enum class State
        {
            Expression,
            Operand,
            AfterName,
            AfterOperand,
            ComparedOperand,
            ComparedName,
            AfterComparison,
            Receiver,
            ReceiverName,
            AfterReceiver,
            Count
        };

        constexpr int contextRuleBase = 128;
        constexpr int stateCount = static_cast<int>(State::Count);
        constexpr int tokenAlphabetSize = 256;

        // У каждого окончания выражения свой набор нетерминалов.
        Symbol exprSymbol(End end, State state = State::Expression)
        {
            return static_cast<Symbol>(-(contextRuleBase + stateCount * static_cast<int>(end)
                                        + static_cast<int>(state)));
        }

        void append(Alternatives& dest, const Alternatives& src)
        {
            dest.insert(dest.end(), src.begin(), src.end());
        }

        Alternatives followedBy(Alternatives src, Symbol next)
        {
            for (auto& production : src)
                production.push_back(next);

            return src;
        }

        void defineProgram(Grammar& grammar)
        {
            constexpr Symbol id = T(LEX_ID);
            constexpr Symbol left = T(LEX_LEFTHESIS);
            constexpr Symbol right = T(LEX_RIGHTHESIS);
            constexpr Symbol semi = T(LEX_SEMICOLON);
            constexpr Symbol colon = T(LEX_COLON);
            constexpr Symbol assign = T(LEX_LEFT_ARROW);

            constexpr Symbol type = T(LEX_OCTET);
            const Alternatives blocks{
                {T(LEX_SPELL), FunctionHeader, exprSymbol(End::Func)},
                {T(LEX_RITUAL), ProcHeader, ProcBody}
            };
            const Alternatives instructions{
                {T(LEX_RUNE), Binding},
                {T(LEX_FLUX), Binding},
                {id, AfterInstructionName},
                {T(LEX_CHANT), Output},
                {T(LEX_CYCLE), LoopBody}
            };

            Alternatives program{{T(LEX_SUMMON), T(LEX_LORE), semi, Block}};
            append(program, blocks);

            auto proc = followedBy(instructions, ProcTail);
            auto procTail = proc;
            procTail.push_back({T(LEX_CLOSE)});
            procTail.push_back({T(LEX_CLOSE), Block});

            auto loop = followedBy(instructions, LoopTail);
            auto loopTail = loop;
            loopTail.push_back({T(LEX_UNTIL), exprSymbol(End::Loop)});

            grammar = {
                {Program, {InvalidProgram, std::move(program)}},
                {Block, {InvalidProgram, blocks}},
                {FunctionHeader, {InvalidFunc, {
                    {id, left, Params, T(LEX_RIGHT_ARROW), type, T(LEX_EQUALS)}
                }}},
                {ProcHeader, {InvalidProc, {
                    {id, left, Params, T(LEX_EQUALS), T(LEX_OPEN)}
                }}},
                {Params, {InvalidParams, {{right}, {id, colon, type, ParamTail}}}},
                {ParamTail, {InvalidParams, {{right}, {T(LEX_COMMA), Params}}}},
                {ProcBody, {InvalidInstruction, std::move(proc)}},
                {ProcTail, {InvalidInstruction, std::move(procTail)}},
                {Binding, {InvalidInstruction, {{id, colon, type, assign, exprSymbol(End::Instruction)}}}},
                {AfterInstructionName, {InvalidInstruction, {
                    {assign, exprSymbol(End::Instruction)},
                    {left, Args, semi}
                }}},
                {LoopBody, {InvalidLoop, std::move(loop)}},
                {LoopTail, {InvalidLoop, std::move(loopTail)}}
            };
        }

        Alternatives operandStarts(Symbol operand, Symbol afterName, Symbol afterOperand)
        {
            return {
                {T(LEX_INT_LITERAL), afterOperand},
                {T(LEX_ID), afterName},
                {T(LEX_FLIP), operand},
                {T(LEX_LEFTHESIS), exprSymbol(End::Group), afterOperand}
            };
        }

        Alternatives binaryOperations(Symbol operand)
        {
            return {
                {T(LEX_STAR), operand},
                {T(LEX_PLUS), operand},
                {T(LEX_MINUS), operand},
                {T(LEX_MASK), operand},
                {T(LEX_FUSE), operand}
            };
        }

        void defineExpressions(Grammar& grammar)
        {
            constexpr Symbol id = T(LEX_ID);
            constexpr Symbol left = T(LEX_LEFTHESIS);
            constexpr Symbol right = T(LEX_RIGHTHESIS);
            constexpr Symbol semi = T(LEX_SEMICOLON);
            constexpr Symbol pipe = T(LEX_TILDE_ARROW);

            const std::array<RuleData, static_cast<std::size_t>(End::Count)> endings{{
                {InvalidFunc, {{semi}, {semi, Block}}},
                {InvalidInstruction, {{semi}}},
                {InvalidLoop, {{semi}}},
                {InvalidConditional, {{T(LEX_THEN)}}},
                {InvalidConditional, {{T(LEX_ELSE)}}},
                {InvalidExpression, {{right}}},
                {InvalidArgs, {{right}, {T(LEX_COMMA), Args}}}
            }};

            for (std::size_t index = 0; index < endings.size(); ++index)
            {
                const auto end = static_cast<End>(index);
                const auto& ending = endings[index];

                const Symbol expression = exprSymbol(end);
                const Symbol operand = exprSymbol(end, State::Operand);
                const Symbol afterOperand = exprSymbol(end, State::AfterOperand);
                const Symbol afterName = exprSymbol(end, State::AfterName);
                const Symbol comparedOperand = exprSymbol(end, State::ComparedOperand);
                const Symbol afterComparison = exprSymbol(end, State::AfterComparison);
                const Symbol comparedName = exprSymbol(end, State::ComparedName);
                const Symbol receiver = exprSymbol(end, State::Receiver);
                const Symbol receiverName = exprSymbol(end, State::ReceiverName);
                const Symbol afterReceiver = exprSymbol(end, State::AfterReceiver);

                const auto firstOperand = operandStarts(operand, afterName, afterOperand);
                const auto comparedStarts = operandStarts(comparedOperand, comparedName, afterComparison);
                auto expressionStarts = firstOperand;
                expressionStarts.push_back({
                    T(LEX_WHEN), exprSymbol(End::Then), exprSymbol(End::Else), expression
                });

                auto beforeComparison = binaryOperations(operand);
                beforeComparison.push_back({T(LEX_LEFT_ANGLE_BRACKET), comparedOperand});
                beforeComparison.push_back({T(LEX_RIGHT_ANGLE_BRACKET), comparedOperand});
                beforeComparison.push_back({T(LEX_EQUAL), comparedOperand});
                beforeComparison.push_back({T(LEX_NOT_EQUAL), comparedOperand});
                beforeComparison.push_back({pipe, receiver});
                append(beforeComparison, ending.alternatives);

                auto afterComparisonRules = binaryOperations(comparedOperand);
                afterComparisonRules.push_back({pipe, receiver});
                append(afterComparisonRules, ending.alternatives);

                Alternatives pipeline{{pipe, receiver}};
                append(pipeline, ending.alternatives);

                auto withCall = beforeComparison;
                withCall.push_back({left, Args, afterOperand});
                auto comparedWithCall = afterComparisonRules;
                comparedWithCall.push_back({left, Args, afterComparison});
                auto receiverWithCall = pipeline;
                receiverWithCall.push_back({left, Args, afterReceiver});

                grammar.emplace(expression, RuleData{InvalidExpression, std::move(expressionStarts)});
                grammar.emplace(operand, RuleData{InvalidExpression, firstOperand});
                grammar.emplace(afterOperand, RuleData{ending.error, std::move(beforeComparison)});
                grammar.emplace(afterName, RuleData{ending.error, std::move(withCall)});
                grammar.emplace(comparedOperand, RuleData{InvalidExpression, comparedStarts});
                grammar.emplace(afterComparison, RuleData{ending.error, std::move(afterComparisonRules)});
                grammar.emplace(comparedName, RuleData{ending.error, std::move(comparedWithCall)});
                grammar.emplace(receiver, RuleData{InvalidExpression, {{id, receiverName}}});
                grammar.emplace(receiverName, RuleData{ending.error, std::move(receiverWithCall)});
                grammar.emplace(afterReceiver, RuleData{ending.error, std::move(pipeline)});
            }

            Alternatives output{{T(LEX_STR_LITERAL), semi}};
            append(output, grammar.at(exprSymbol(End::Instruction)).alternatives);
            grammar.emplace(Output, RuleData{InvalidInstruction, std::move(output)});

            Alternatives arguments{{right}};
            append(arguments, grammar.at(exprSymbol(End::Arg)).alternatives);
            grammar.emplace(Args, RuleData{InvalidArgs, std::move(arguments)});
        }

        Grammar buildGrammar()
        {
            Grammar grammar;
            defineProgram(grammar);
            defineExpressions(grammar);

            // В форме Грейбах терминалы после первого символа заменяются правилами вида [t] -> t.
            for (auto& entry : grammar)
            {
                auto& def = entry.second;
                for (auto& production : def.alternatives)
                {
                    for (std::size_t index = 1; index < production.size(); ++index)
                    {
                        const Symbol terminal = production[index];
                        if (terminal > 0)
                        {
                            const int code = (def.error - InvalidProgram + 1) * tokenAlphabetSize;
                            const Symbol alias = static_cast<Symbol>(-(code + terminal));
                            grammar.emplace(alias, RuleData{def.error, {{terminal}}});
                            production[index] = alias;
                        }
                    }
                }
            }
            return grammar;
        }

        struct GrammarData
        {
            std::vector<Symbol> symbols;
            std::vector<Chain> chains;
            std::vector<Rule> rules;
        };

        GrammarData compileGrammar(const Grammar& grammar)
        {
            std::size_t chainCount = 0;
            std::size_t symbolCount = 0;
            for (const auto& entry : grammar)
            {
                chainCount += entry.second.alternatives.size();
                for (const auto& production : entry.second.alternatives)
                    symbolCount += production.size();
            }

            GrammarData data;
            // Резервируем место заранее, чтобы указатели на цепочки и символы не смещались.
            data.symbols.reserve(symbolCount);
            data.chains.reserve(chainCount);
            data.rules.reserve(grammar.size());

            for (const auto& entry : grammar)
            {
                const auto& rule = entry.second;
                const auto firstChain = data.chains.size();
                for (const auto& production : rule.alternatives)
                {
                    const auto firstSymbol = data.symbols.size();
                    data.symbols.insert(data.symbols.end(), production.begin(), production.end());
                    data.chains.push_back({static_cast<short>(production.size()),
                                           data.symbols.data() + firstSymbol});
                }

                data.rules.push_back({entry.first, rule.error,
                                      static_cast<short>(rule.alternatives.size()),
                                      data.chains.data() + firstChain});
            }
            return data;
        }

        constexpr char stateLabels[] = "XUANTntHRJ";

        void appendSymbol(std::string& text, Symbol symbol)
        {
            const char label = symbolChar(symbol);
            if (symbol > 0)
            {
                text += label;
                return;
            }

            const int code = -symbol;
            if (code >= tokenAlphabetSize)
            {
                text += '[';
                text += label;
                text += ']';
                return;
            }

            text += '<';
            text += label;
            if (code >= contextRuleBase)
                text += static_cast<char>('0' + (code - contextRuleBase) / stateCount);
            text += '>';
        }
    }

    Greibach getGreibach()
    {
        static const GrammarData data = compileGrammar(buildGrammar());
        return {static_cast<short>(data.rules.size()), Program, T('$'), data.rules.data()};
    }

    short findRule(const Greibach& grammar, GRBALPHABET symbol)
    {
        const auto* end = grammar.rules + grammar.size;
        const auto* rule = std::lower_bound(grammar.rules, end, symbol,
            [](const Rule& item, GRBALPHABET key) { return item.left < key; });

        if (rule == end || rule->left != symbol)
            return -1;

        return static_cast<short>(rule - grammar.rules);
    }

    short findChain(const Rule& rule, GRBALPHABET token, short from)
    {
        for (short i = from; i < rule.size; ++i)
        {
            if (rule.chains[i].symbols[0] == token)
                return i;
        }
        return -1;
    }

    char symbolChar(GRBALPHABET symbol)
    {
        if (symbol > 0)
            return static_cast<char>(symbol);

        const int code = -symbol;
        if (code >= contextRuleBase && code < tokenAlphabetSize)
            return stateLabels[(code - contextRuleBase) % stateCount];

        return static_cast<char>(code % tokenAlphabetSize);
    }

    std::string ruleText(const Rule& rule, short chainIndex)
    {
        const auto& chain = rule.chains[chainIndex];
        std::string text;
        text.reserve(4 * (1 + chain.size) + 2);
        appendSymbol(text, rule.left);
        text += "->";
        for (short i = 0; i < chain.size; ++i)
            appendSymbol(text, chain.symbols[i]);

        return text;
    }
}
