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

#include "rule.h"

// Qt includes

#include <QAction>
#include <QMenu>
#include <QPushButton>
#include <QRegularExpression>
#include <QString>
#include <QIcon>
#include <QApplication>
#include <QStyle>

namespace Digikam
{

class Q_DECL_HIDDEN Rule::Private
{
public:

    Private() = default;

public:

    bool                    useTokenMenu = false;

    QString                 description;
    QString                 iconName;
    QRegularExpression      regExp;

    TokenList               tokens;

    QWidget*                widget       = nullptr;
};

Rule::Rule(const QString& name)
    : QObject(nullptr),
      d      (new Private)
{
    setObjectName(name);
}

Rule::Rule(const QString& name, const QString& icon)
    : QObject(nullptr),
      d      (new Private)
{
    setObjectName(name);
    setIcon(icon);
}

Rule::~Rule()
{
    qDeleteAll(d->tokens);
    d->tokens.clear();

    delete d;
}

void Rule::setIcon(const QString& iconName)
{
    d->iconName = iconName;
}

QPixmap Rule::icon(Rule::IconType type) const
{
    QPixmap icon;

    switch (type)
    {
        case Dialog:
        {
            icon = QIcon::fromTheme(d->iconName).pixmap(48);
            break;
        }

        default:
        {
            icon = QIcon::fromTheme(d->iconName).pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize));
            break;
        }
    }

    return icon;
}

void Rule::setDescription(const QString& desc)
{
    d->description = desc;
}

QString Rule::description() const
{
    return d->description;
}

QRegularExpression& Rule::regExp() const
{
    return d->regExp;
}

void Rule::setRegExp(const QRegularExpression& regExp)
{
    d->regExp = regExp;
}

QPushButton* Rule::createButton(const QString& name, const QIcon& icon)
{
    const int maxHeight       = 28;
    QPushButton* const button = new QPushButton;
    button->setText(name);
    button->setIcon(icon);
    button->setMinimumHeight(maxHeight);
    button->setMaximumHeight(maxHeight);

    return button;
}

QPushButton* Rule::registerButton(QWidget* parent)
{
    QPushButton* const button = createButton(objectName(), icon());

    QList<QAction*> actions;

    if      (d->tokens.count() > 1 && d->useTokenMenu)
    {
        QMenu* const menu = new QMenu(button);

        for (const Token* const token : std::as_const(d->tokens))
        {
            actions << token->action();
        }

        menu->addActions(actions);
        button->setMenu(menu);
    }
    else if (!d->tokens.isEmpty())
    {
        Token* const token = d->tokens.first();

        connect(button, SIGNAL(clicked()),
                token, SLOT(slotTriggered()));
    }

    button->setParent(parent);

    return button;
}

QAction* Rule::registerMenu(QMenu* parent)
{
    QAction* action = nullptr;

    if      (d->tokens.count() > 1 && d->useTokenMenu)
    {
        QMenu* const menu = new QMenu(parent);
        QList<QAction*> actions;

        for (const Token* const token : std::as_const(d->tokens))
        {
            actions << token->action();
        }

        menu->addActions(actions);
        action = parent->addMenu(menu);
    }
    else if (!d->tokens.isEmpty())
    {
        action = d->tokens.first()->action();
        parent->insertAction(nullptr, action);
    }

    if (action)
    {
        action->setText(objectName());
        action->setIcon(icon());
    }

    return action;
}

bool Rule::addToken(const QString& id, const QString& description, const QString& actionName)
{
    if (id.isEmpty() || description.isEmpty())
    {
        return false;
    }

    Token* const token = new Token(id, description);

    if (!actionName.isEmpty())
    {
        token->action()->setText(actionName);
    }

    connect(token, SIGNAL(signalTokenTriggered(QString)),
            this, SLOT(slotTokenTriggered(QString)));

    d->tokens << token;

    return true;
}

void Rule::setUseTokenMenu(bool value)
{
    d->useTokenMenu = value;
}

bool Rule::useTokenMenu() const
{
    return d->useTokenMenu;
}

TokenList& Rule::tokens() const
{
    return d->tokens;
}

void Rule::setParentWidget(QWidget* const widget)
{
    d->widget = widget;
}

QWidget* Rule::getParentWidget() const
{
    return d->widget;
}

void Rule::slotTokenTriggered(const QString& token)
{
    Q_EMIT signalTokenTriggered(token);
}

bool Rule::isValid() const
{
    return (
            !d->tokens.isEmpty()           &&
            !d->regExp.pattern().isEmpty() &&
            d->regExp.isValid()
           );
}

void Rule::reset()
{
}

QString Rule::escapeToken(const QString& token)
{
    QString escaped = token;

    // replace special characters for renaming options

    escaped.replace(QLatin1Char('['), QLatin1String("\\["));
    escaped.replace(QLatin1Char(']'), QLatin1String("\\]"));

    // replace special characters for modifiers

    escaped.replace(QLatin1Char('{'), QLatin1String("\\{"));
    escaped.replace(QLatin1Char('}'), QLatin1String("\\}"));

    return escaped;
}

ParseResults Rule::parse(ParseSettings& settings)
{
    ParseResults parsedResults;
    const QRegularExpression& reg = regExp();
    const QString& parseString    = settings.parseString;
    QRegularExpressionMatch         match;

    int pos = 0;

    while (pos > -1)
    {
        pos = parseString.indexOf(reg, pos, &match);

        if (pos > -1)
        {
            QString result = parseOperation(settings, match);

            ParseResults::ResultsKey   k(pos, match.captured(0).size());
            ParseResults::ResultsValue v(match.captured(0), result);
            parsedResults.addEntry(k, v);
            pos           += match.capturedLength();
        }
    }

    return parsedResults;
}

} // namespace Digikam

#include "moc_rule.cpp"
