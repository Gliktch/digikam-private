/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a token class
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "token.h"

// Qt includes

#include <QAction>

namespace Digikam
{

class Q_DECL_HIDDEN Token::Private
{
public:

    Private() = default;

    QString  id;
    QString  description;
    QAction* action       = nullptr;
};

Token::Token(const QString& id, const QString& description)
    : QObject(nullptr),
      d      (new Private)
{
    d->id          = id;
    d->description = description;

    d->action = new QAction(this);
    d->action->setText(id);
    d->action->setToolTip(description);

    connect(d->action, SIGNAL(triggered()),
            this, SLOT(slotTriggered()));
}

Token::~Token()
{
    delete d;
}

void Token::slotTriggered()
{
    Q_EMIT signalTokenTriggered(d->id);
}

QString Token::id() const
{
    return d->id;
}

QString Token::description() const
{
    return d->description;
}

QAction* Token::action() const
{
    return d->action;
}

} // namespace Digikam

#include "moc_token.cpp"
