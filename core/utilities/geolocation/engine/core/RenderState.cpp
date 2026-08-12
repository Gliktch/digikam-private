/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "RenderState.h"

// Qt includes

#include <QList>

namespace Marble
{

class Q_DECL_HIDDEN RenderState::Private
{
public:

    QString             m_name;
    QList<RenderState>  m_children;
    RenderStatus        m_status;

public:

    Private(const QString& name = QString(), RenderStatus status = Complete);
    RenderStatus status() const;
    static RenderStatus minimumStatus(RenderStatus a, RenderStatus b);
    QString toString(const RenderState& state, int indent) const;
};

RenderState::RenderState(const QString& name, RenderStatus status) :
    d(new Private(name, status))
{
    // nothing to do
}

RenderState::RenderState(const RenderState& other) :
    d(new Private)
{
    *d = *other.d;
}

RenderState& RenderState::operator=(const RenderState& other)
{
    *d = *other.d;
    return *this;
}

RenderState::~RenderState()
{
    delete d;
}

RenderStatus RenderState::status() const
{
    return d->status();
}

QString RenderState::name() const
{
    return d->m_name;
}

int RenderState::children() const
{
    return d->m_children.size();
}

RenderState RenderState::childAt(int index) const
{
    return d->m_children.at(index);
}

void RenderState::addChild(const RenderState& child)
{
    d->m_children.push_back(child);
}

RenderState::operator QString() const
{
    return d->toString(*this, 0);
}

RenderState::Private::Private(const QString& name, RenderStatus status) :
    m_name(name),
    m_status(status)
{
    // nothing to do
}

RenderStatus RenderState::Private::status() const
{
    RenderStatus status = Complete;

    for (const RenderState& child : m_children)
    {
        status = minimumStatus(status, child.status());
    }

    return minimumStatus(status, m_status);
}

RenderStatus RenderState::Private::minimumStatus(RenderStatus a, RenderStatus b)
{
    if (a == Incomplete || b == Incomplete)
    {
        return Incomplete;
    }

    if (a == WaitingForData || b == WaitingForData)
    {
        return WaitingForData;
    }

    if (a == WaitingForUpdate || b == WaitingForUpdate)
    {
        return WaitingForUpdate;
    }

    Q_ASSERT(a == Complete || b == Complete);
    return Complete;
}

QString RenderState::Private::toString(const RenderState& state, int level) const
{
    QString const prefix = level > 0 ? QString::fromUtf8("\n") : QString::fromUtf8("");
    QString const indent(level * 2, QLatin1Char(' '));
    QString status;

    switch (state.status())
    {
        case Marble::Complete:
            status = QString::fromUtf8("Complete");
            break;

        case Marble::WaitingForUpdate:
            status = QString::fromUtf8("Waiting for update");
            break;

        case Marble::WaitingForData:
            status = QString::fromUtf8("Waiting for data");
            break;

        case Marble::Incomplete:
            status = QString::fromUtf8("Incomplete");
            break;
    }

    QString const name = (state.name().isEmpty() ? QString::fromUtf8("Anonymous renderer") : state.name());
    QString result = QString::fromUtf8("%1%2%3: %4").arg(prefix, indent, name, status);

    for (const RenderState& child : std::as_const(state.d->m_children))
    {
        result += toString(child, level + 1);
    }

    return result;
}

} // namespace Marble
