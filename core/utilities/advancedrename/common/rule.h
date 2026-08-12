/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-01
 * Description : an abstract rule class
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "parseresults.h"
#include "parsesettings.h"
#include "token.h"

class QAction;
class QMenu;
class QPushButton;
class QString;

namespace Digikam
{

class Rule : public QObject
{
    Q_OBJECT

public:

    enum IconType
    {
        Action = 0,
        Dialog
    };

public:

    explicit Rule(const QString& name);
    Rule(const QString& name, const QString& icon);
    ~Rule() override;

    /**
     * TODO: This is probably not needed anymore. Find out.
     * returns the currently assigned regExp object. Note that it is returned as a const ref, meaning
     * that if you use it in your custom parse operation, the main parse method has already searched for the pattern
     * and filled in the results of this search, so that you can use QRegularExpressionMatch::captured() immediately,
     * you don't have to search on your own.
     *
     * For example when implementing the Option::parseOperation() method, get the regExp object with
     *
     *      const QRegularExpression& reg = regExp();
     *
     * and immediately fetch possible matches with
     *
     *      const QString& param1 = reg.captured(1);
     *
     * @see Option
     * @see Modifier
     *
     * @return a const ref to the assigned regexp object
     */
    QRegularExpression& regExp() const;

    QString description() const;
    QPixmap icon(Rule::IconType type = Rule::Action) const;

    /**
     * @return a list of all registered tokens
     */
    TokenList& tokens() const;

    /**
     * Register a button in the parent object. By calling this method, a new button for the parser
     * object will be created and all necessary connections will be setup.
     *
     * @param  parent the parent object the button will be registered for
     * @return a pointer to the newly created button
     */
    QPushButton* registerButton(QWidget* parent);

    /**
     * Register a menu action in the parent object. By calling this method, a new action for the parser
     * object will be created and all necessary connections will be setup.
     *
     * @param  parent the parent object the action will be registered for
     * @return a pointer to the newly created action
     */
    QAction* registerMenu(QMenu* parent);

    /**
     * Returns true if a token menu is used.
     */
    bool useTokenMenu() const;

    /**
     * Checks the validity of the parse object
     * @return true if valid
     */
    bool isValid() const;

    /**
     * Resets the parser to its initial state
     */
    virtual void reset();

    /**
     * Escape the token characters to make them work in regular expressions
     *
     * @param token the token to be escaped
     * @return A token with escaped characters. This token can then be used in a regular expression
     */
    static QString escapeToken(const QString& token);

    ParseResults parse(ParseSettings& settings);

Q_SIGNALS:

    void signalTokenTriggered(const QString&);

protected:

    /**
     * TODO: describe me
     * @param settings contains settings
     * @param match result of the regular expression match done in Option::parse()
     * @return
     */
    virtual QString parseOperation(ParseSettings& settings,
                                   const QRegularExpressionMatch& match) = 0;

    /**
     * add a token to the parser, every parser should at least assign one token object
     * @param id the token id string (used for parsing)
     * @param description the description of the token (used for example in the tooltip)
     * @param actionName [optional] the name of the token action (only used when the token menu is displayed)
     * @return
     */
    bool addToken(const QString& id, const QString& description, const QString& actionName = QString());

    void setRegExp(const QRegularExpression& regExp);
    void setDescription(const QString& desc);
    void setIcon(const QString& pixmap);

    /**
     * If multiple tokens have been assigned to a rule, a menu will be created.
     * If you want to display a menu for every defined token, set this method to 'true' and
     * re-implement the @see slotTokenTriggered method.
     * @param value boolean parameter to set token menu usage
     */
    void setUseTokenMenu(bool value);

    /**
     * Set and get parent widget for the dialogs
     * @param widget QWidget pointer from parent widget
     */
    void setParentWidget(QWidget* const widget);
    QWidget* getParentWidget() const;

protected Q_SLOTS:

    virtual void slotTokenTriggered(const QString&);

private:

    QPushButton* createButton(const QString& name, const QIcon& icon);

private:

    /// @note disabled
    Rule(QObject*)               = delete;
    Rule(const Rule&)            = delete;
    Rule& operator=(const Rule&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

typedef QList<Rule*> RulesList;

} // namespace Digikam
