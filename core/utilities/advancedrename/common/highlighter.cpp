/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-19
 * Description : syntax highlighter for AdvancedRename utility
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "highlighter.h"

// Qt includes

#include <QTextDocument>
#include <QRegularExpression>

// Local includes

#include "parser.h"

namespace Digikam
{

class Q_DECL_HIDDEN Highlighter::Private
{
public:

    enum PatternType
    {
        OptionPattern = 0,
        ModifierPattern,
        QuotedTextPattern,
        ParameterPattern
    };

public:

    class Q_DECL_HIDDEN HighlightingRule
    {
    public:

        PatternType           type      = OptionPattern;
        QRegularExpression    pattern;
        QTextCharFormat       format;
    };

public:

    Private() = default;

public:

    QVector<HighlightingRule> highlightingRules;
    HighlightingRule          quotationRule;

    QTextCharFormat           optionFormat;
    QTextCharFormat           parameterFormat;
    QTextCharFormat           modifierFormat;
    QTextCharFormat           quotationFormat;
    QTextCharFormat           errorFormat;

    Parser*                   parser  = nullptr;
};

Highlighter::Highlighter(QTextDocument* const document, Parser* const _parser)
    : QSyntaxHighlighter(document),
      d                 (new Private)
{
    d->parser = _parser;
    setupHighlightingGrammar();
}

Highlighter::~Highlighter()
{
    delete d;
}

void Highlighter::highlightBlock(const QString& text)
{
    for (const Private::HighlightingRule& rule : std::as_const(d->highlightingRules))
    {
        QRegularExpression      expression(rule.pattern);
        QRegularExpressionMatch match;
        int index = text.indexOf(expression, 0, &match);

        while (index >= 0)
        {
            int length = match.capturedLength();
            setFormat(index, length, rule.format);

            switch (rule.type)
            {
                case Private::OptionPattern:
                case Private::ModifierPattern:
                {
                    // highlight parameters in options and modifiers

                    if ((expression.captureCount() > 0) && !match.captured(1).isEmpty())
                    {
                        QString fullmatched  = match.captured(0);
                        QString parameters   = match.captured(1);

                        if (parameters.startsWith(QLatin1Char(':')))
                        {
                            parameters.remove(0, 1);

                            if (!parameters.isEmpty())
                            {
                                int pindex = fullmatched.indexOf(parameters);

                                while (pindex >= 0)
                                {
                                    int plength = parameters.length();
                                    setFormat(index + pindex, plength, d->parameterFormat);
                                    pindex      = fullmatched.indexOf(parameters, pindex + plength);
                                }
                            }
                        }
                    }

                    break;
                }

                default:
                {
                    break;
                }
            }

            index = text.indexOf(expression, index + length, &match);
        }
    }

    // mark invalid modifiers in the parse string

    ParseSettings settings;
    settings.parseString = text;
    ParseResults invalid = d->parser->invalidModifiers(settings);
    const auto keys      = invalid.keys();

    for (const ParseResults::ResultsKey& key : keys)
    {
        setFormat(key.first, key.second, d->errorFormat);
    }

    // highlight quoted text in options and modifiers

    {
        QRegularExpression      expression(d->quotationRule.pattern);
        QRegularExpressionMatch match;
        int index = text.indexOf(expression, 0, &match);

        while (index >= 0)
        {
            QString fullmatched = match.captured(0);
            Q_UNUSED(fullmatched);
            int qlength         = match.capturedLength();
            setFormat(index, qlength, d->quotationFormat);
            index               = text.indexOf(expression, index + qlength, &match);
        }
    }
}

void Highlighter::setupHighlightingGrammar()
{
    if (!d->parser)
    {
        return;
    }

    Private::HighlightingRule rule;

    // --------------------------------------------------------

    d->optionFormat.setForeground(Qt::red);
    const auto list = d->parser->options();

    for (const Rule* const option : list)
    {
        QRegularExpression r    = option->regExp();
        rule.type               = Private::OptionPattern;
        rule.pattern            = r;
        rule.format             = d->optionFormat;
        d->highlightingRules.append(rule);
    }

    // --------------------------------------------------------

    d->modifierFormat.setForeground(Qt::darkGreen);
    const auto list2 = d->parser->modifiers();

    for (const Rule* const modifier : list2)
    {
        QRegularExpression r    = modifier->regExp();
        rule.type               = Private::ModifierPattern;
        rule.pattern            = r;
        rule.format             = d->modifierFormat;
        d->highlightingRules.append(rule);
    }

    // --------------------------------------------------------

    d->quotationFormat.setForeground(QColor(0x50, 0x50, 0xff)); // light blue
    d->quotationFormat.setFontItalic(true);
    d->quotationRule.pattern = QRegularExpression(QLatin1String("\".*\""));
    d->quotationRule.pattern.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    d->quotationRule.format  = d->quotationFormat;
    d->quotationRule.type    = Private::QuotedTextPattern;

    // --------------------------------------------------------

    d->parameterFormat.setForeground(Qt::darkYellow);
    d->parameterFormat.setFontItalic(true);

    // --------------------------------------------------------

    d->errorFormat.setForeground(Qt::white);
    d->errorFormat.setBackground(Qt::red);
}

} // namespace Digikam

#include "moc_highlighter.cpp"
