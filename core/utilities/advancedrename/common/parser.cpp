/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : the main parser object for the AdvancedRename utility
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "parser.h"

// Qt includes

#include <QWidget>
#include <QFileInfo>
#include <QRegularExpression>

// Local includes

#include "cameranameoption.h"
#include "databaseoption.h"
#include "dateoption.h"
#include "directorynameoption.h"
#include "filepropertiesoption.h"
#include "metadataoption.h"
#include "sequencenumberoption.h"
#include "casemodifier.h"
#include "dateformatmodifier.h"
#include "defaultvaluemodifier.h"
#include "rangemodifier.h"
#include "removedoublesmodifier.h"
#include "replacemodifier.h"
#include "trimmedmodifier.h"
#include "uniquemodifier.h"

namespace Digikam
{

class Q_DECL_HIDDEN Parser::Private
{
public:

    Private() = default;

public:

    RulesList options;
    RulesList modifiers;
};

// --------------------------------------------------------

Parser::Parser(QWidget* const widget)
    : d(new Private)
{
    registerOption(new FilePropertiesOption());
    registerOption(new DirectoryNameOption());
    registerOption(new CameraNameOption());
    registerOption(new SequenceNumberOption(widget));
    registerOption(new DateOption(widget));
    registerOption(new DatabaseOption(widget));
    registerOption(new MetadataOption(widget));

    // --------------------------------------------------------

    registerModifier(new CaseModifier());
    registerModifier(new DateFormatModifier());
    registerModifier(new TrimmedModifier());
    registerModifier(new UniqueModifier());
    registerModifier(new RemoveDoublesModifier());
    registerModifier(new DefaultValueModifier(widget));
    registerModifier(new ReplaceModifier(widget));
    registerModifier(new RangeModifier(widget));
}

Parser::~Parser()
{
    qDeleteAll(d->options);
    d->options.clear();

    qDeleteAll(d->modifiers);
    d->modifiers.clear();

    delete d;
}

void Parser::reset()
{
    for (Rule* const option : std::as_const(d->options))
    {
        option->reset();
    }

    for (Rule* const modifier : std::as_const(d->modifiers))
    {
        modifier->reset();
    }
}

bool Parser::parseStringIsValid(const QString& str)
{
    QRegularExpression invalidString(QRegularExpression::anchoredPattern(QLatin1String("^\\s*$")));

    return (!str.isEmpty() && !invalidString.match(str).hasMatch());
}

RulesList Parser::options() const
{
    return d->options;
}

RulesList Parser::modifiers() const
{
    return d->modifiers;
}

void Parser::registerOption(Rule* option)
{
    if (!option || !option->isValid())
    {
        return;
    }

    d->options.append(option);
}

void Parser::unregisterOption(const Rule* option)
{
    if (!option)
    {
        return;
    }

    for (RulesList::iterator it = d->options.begin() ;
         it != d->options.end() ; )
    {
        if (*it == option)
        {
            delete *it;
            it = d->options.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Parser::registerModifier(Rule* modifier)
{
    if (!modifier || !modifier->isValid())
    {
        return;
    }

    d->modifiers.append(modifier);
}

void Parser::unregisterModifier(const Rule* modifier)
{
    if (!modifier)
    {
        return;
    }

    for (RulesList::iterator it = d->modifiers.begin() ;
         it != d->modifiers.end() ; )
    {
        if (*it == modifier)
        {
            delete *it;
            it = d->modifiers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

ParseResults Parser::results(ParseSettings& settings)
{
    ParseResults results;

    for (Rule* const option : std::as_const(d->options))
    {
        ParseResults r = option->parse(settings);
        results.append(r);
    }

    for (Rule* const modifier : std::as_const(d->modifiers))
    {
        ParseResults r = modifier->parse(settings);
        results.append(r);
    }

    return results;
}

ParseResults Parser::invalidModifiers(ParseSettings& settings)
{
    parse(settings);

    return settings.invalidModifiers;
}

QString Parser::parse(ParseSettings& settings)
{
    QFileInfo fi(settings.fileUrl.toLocalFile());

    if (!parseStringIsValid(settings.parseString))
    {
        return fi.fileName();
    }

    ParseResults res;

    for (Rule* const option : std::as_const(d->options))
    {
        ParseResults r = option->parse(settings);
        res.append(r);
    }

    settings.invalidModifiers = applyModifiers(settings, res);
    QString newName           = res.replaceTokens(settings.parseString);
    settings.results          = res;

    // remove invalid modifiers from the new name

    for (const Rule* const mod : std::as_const(d->modifiers))
    {
        newName.remove(mod->regExp());
    }

    if (newName.isEmpty())
    {
        return fi.fileName();
    }

    if (settings.useOriginalFileExtension)
    {
        newName.append(QLatin1Char('.')).append(fi.suffix());
    }

    return newName;
}

bool Parser::tokenAtPosition(ParseSettings& settings, int pos)
{
    int start;
    int length;

    return tokenAtPosition(settings, pos, start, length);
}

bool Parser::tokenAtPosition(ParseSettings& settings, int pos, int& start, int& length)
{
    bool found                   = false;
    ParseResults r               = results(settings);
    ParseResults::ResultsKey key = r.keyAtApproximatePosition(pos);
    start                        = key.first;
    length                       = key.second;

    if ((pos >= start) && (pos <= start + length))
    {
        found = true;
    }

    return found;
}

ParseResults Parser::applyModifiers(const ParseSettings& _settings, ParseResults& results)
{
    if (results.isEmpty() || d->modifiers.isEmpty())
    {
        return ParseResults();
    }

    ParseSettings settings = _settings;
    settings.results       = results;

    // appliedModifiers holds all the modified parse results

    ParseResults appliedModifiers = results;

    // modifierResults holds all the modifiers found in the parse string

    ParseResults modifierResults;

    // modifierMap maps the actual modifier objects to the entries in the modifierResults structure

    QMap<ParseResults::ResultsKey, Rule*> modifierMap;

    for (Rule* const modifier : std::as_const(d->modifiers))              // cppcheck-suppress constVariablePointer
    {
        QRegularExpression regExp = modifier->regExp();
        int pos                   = 0;
        QRegularExpressionMatch match;

        while (pos > -1)
        {
            pos = _settings.parseString.indexOf(regExp, pos, &match);

            if (pos > -1)
            {
                ParseResults::ResultsKey   k(pos, match.capturedLength());
                ParseResults::ResultsValue v(match.captured(0), QString());

                modifierResults.addEntry(k, v);
                modifierMap.insert(k, modifier);

                pos += match.capturedLength();
            }
        }
    }

    // Check for valid modifiers (they must appear directly after an option) and apply the modification to the option
    // parse result.
    // We need to create a second ParseResults object with modified keys, otherwise the final parsing step will not
    // remove the modifier tokens from the result.

    const auto keys = results.keys();

    for (const ParseResults::ResultsKey& key : keys)
    {
        int off  = results.offset(key);
        int diff = 0;

        for (int pos = off ; pos < _settings.parseString.size() ; )
        {
            if (modifierResults.hasKeyAtPosition(pos))
            {
                ParseResults::ResultsKey mkey = modifierResults.keyAtPosition(pos);
                Rule* const mod               = modifierMap[mkey];
                QString modToken              = modifierResults.token(mkey);

                QString token                 = results.token(key);
                QString str2Modify            = results.result(key);

                QString modResult;

                if (mod)
                {
                    settings.parseString       = modToken;
                    settings.currentResultsKey = key;
                    settings.str2Modify        = str2Modify;
                    ParseResults modResults    = mod->parse(settings);

                    if (!modResults.isEmpty() && (modResults.values().length() == 1))
                    {
                        modResult = modResults.result(modResults.keys().constFirst());
                    }
                }

                // update result

                ParseResults::ResultsKey   resultKey = key;
                ParseResults::ResultsValue resultValue(token, modResult);
                results.addEntry(resultKey, resultValue);

                // update modifier map

                ParseResults::ResultsKey modifierKey = key;
                modifierKey.second                  += diff;
                ParseResults::ResultsValue modifierValue(modToken, modResult);

                appliedModifiers.deleteEntry(modifierKey);
                modifierKey.second += modToken.size();
                appliedModifiers.addEntry(modifierKey, modifierValue);

                // set position to the next possible token

                pos  += mkey.second;
                diff += mkey.second;

                // remove assigned modifier from modifierResults

                modifierResults.deleteEntry(mkey);
            }
            else
            {
                break;
            }
        }
    }

    results = appliedModifiers;

    return modifierResults;
}

} // namespace Digikam
