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

#pragma once

// Qt includes

#include <QList>
#include <QMap>
#include <QString>

// Local includes

#include "modifier.h"
#include "option.h"

namespace Digikam
{

class Modifier;
class Option;

class Parser
{

public:

    explicit Parser(QWidget* const widget);
    virtual ~Parser();

    void          reset();

    QString       parse(ParseSettings& settings);

    RulesList     options()   const;
    RulesList     modifiers() const;

    bool          tokenAtPosition(ParseSettings& settings, int pos);
    bool          tokenAtPosition(ParseSettings& settings, int pos, int& start, int& length);

    ParseResults  invalidModifiers(ParseSettings& settings);

    /**
     * check if the given parse string is valid
     * @param str the parse string
     * @return true if valid / can be parsed
     */
    static bool parseStringIsValid(const QString& str);

protected:

    void registerOption(Rule* option);
    void unregisterOption(const Rule* option);

    void registerModifier(Rule* modifier);
    void unregisterModifier(const Rule* modifier);

private:

    ParseResults results(ParseSettings& settings);

    /**
     * Applies modifiers to the given ParseResults.
     * @param   settings        the ParseSettings with parseString to analyze
     * @param   results         the ParseResults object the modifiers should be applied to
     * @return  a ParseResults object with invalid modifiers (modifiers that have a wrong position in the parse string)
     */
    ParseResults applyModifiers(const ParseSettings& _settings, ParseResults& results);

private:

    /// @note disabled
    Parser(const Parser&)            = delete;
    Parser& operator=(const Parser&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
