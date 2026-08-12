/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : Plasma Address Book contacts interface
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "akonadiiface.h"

// Qt includes

#include <QApplication>
#include <QString>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

#if defined(Q_OS_DARWIN) && defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wundef"
#endif

#include <kjob.h>
#include <akonadi_version.h>

#if AKONADI_VERSION >= QT_VERSION_CHECK(5, 18, 41)
#include <Akonadi/Item>
#else
#include <AkonadiCore/Item>
#endif

#if AKONADI_VERSION >= QT_VERSION_CHECK(5, 19, 80)
#include <Akonadi/ContactSearchJob>
#else
#include <Akonadi/Contact/ContactSearchJob>
#endif
#include <KContacts/Addressee>

#if defined(Q_OS_DARWIN) && defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

// See techbase.kde.org/Development/AkonadiPorting/AddressBook

AkonadiIface::AkonadiIface(QMenu* const parent)
    : QObject (parent),
      m_parent(parent)
{
    m_ABCmenu                   = new QMenu(m_parent);
    QAction* const abcAction    = m_ABCmenu->menuAction();
    abcAction->setIcon(QIcon::fromTheme(QLatin1String("address-book-new")));
    abcAction->setText(i18n("Create Tag From Address Book"));
    m_parent->addMenu(m_ABCmenu);

    QAction* const nothingFound = m_ABCmenu->addAction(i18n("No address book entries found"));
    nothingFound->setEnabled(false);

    Akonadi::ContactSearchJob* const job = new Akonadi::ContactSearchJob();

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotABCSearchResult(KJob*)));
}

AkonadiIface::~AkonadiIface()
{
    delete m_ABCmenu;
}

void AkonadiIface::slotABCSearchResult(KJob* job)
{
    if (job->error())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Address book search was not successful";
        return;
    }

    Akonadi::ContactSearchJob* const searchJob = qobject_cast<Akonadi::ContactSearchJob*>(job);
    const KContacts::Addressee::List contacts  = searchJob->contacts();

    if (contacts.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No contacts in address book";
        return;
    }

    QStringList names;

    for (const KContacts::Addressee& addr : std::as_const(contacts))
    {
        if (!addr.realName().isNull())
        {
            names.append(addr.realName());
        }
    }

    names.removeDuplicates();
    names.sort();

    if (names.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No names in the address book";
        return;
    }

    m_ABCmenu->clear();

    for (const QString& name : std::as_const(names))
    {
        m_ABCmenu->addAction(QIcon::fromTheme(QLatin1String("im-user")), name);
    }

    connect(m_ABCmenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotABCMenuTriggered(QAction*)));
}

void AkonadiIface::slotABCMenuTriggered(QAction* action)
{
    QString name = action->iconText();

    Q_EMIT signalContactTriggered(name);
}

} // namespace Digikam

#include "moc_akonadiiface.cpp"
