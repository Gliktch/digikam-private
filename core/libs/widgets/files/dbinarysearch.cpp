/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-05
 * Description : a widget to find missing binaries.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbinarysearch.h"

// Qt includes

#include <QLabel>
#include <QHeaderView>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DBinarySearch::Private
{
public:

    Private() = default;

public:

    QVector<DBinaryIface*>    binaryIfaces;
    QVector<QTreeWidgetItem*> items;
    QLabel*                   downloadLabel = nullptr;
};

DBinarySearch::DBinarySearch(QWidget* const parent)
    : QTreeWidget(parent),
      d          (new Private)
{
    setIconSize(QSize(16, 16));
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::NoSelection);
    setSortingEnabled(false);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(5);
    setHeaderLabels(QStringList() << QLatin1String("")
                                  << i18nc("@title: binary properties", "Binary")
                                  << i18nc("@title: binary properties", "Version")
                                  << QLatin1String("")
                                  << QLatin1String(""));

    header()->setSectionResizeMode(Status,    QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(Binary,    QHeaderView::Stretch);
    header()->setSectionResizeMode(Version,   QHeaderView::Stretch);
    header()->setSectionResizeMode(Button,    QHeaderView::Stretch);
    header()->setSectionResizeMode(Link,      QHeaderView::Stretch);

    d->downloadLabel = new QLabel(parentWidget());

    QGridLayout* const layout = qobject_cast<QGridLayout*>(parentWidget()->layout());

    if (layout)
    {
        layout->addWidget(this, 0, 0);
    }
}

DBinarySearch::~DBinarySearch()
{
    delete d;
}

void DBinarySearch::addBinary(DBinaryIface& binary)
{
    delete d->downloadLabel;

    (void)binary.recheckDirectories();

    d->binaryIfaces.append(&binary);
    d->items.append(new QTreeWidgetItem());
    QTreeWidgetItem* const item   = d->items[d->items.size() - 1];
    item->setIcon(Status,       QIcon::fromTheme(QLatin1String("dialog-cancel")).pixmap(16, 16));
    item->setText(Binary,       binary.baseName());
    item->setText(Version,      binary.version());
    item->setToolTip(Binary,    binary.description());
    item->setToolTip(Status,    i18n("Binary not found."));
    item->setToolTip(Version,   i18n("Minimal version number required for this binary is %1", binary.minimalVersion()));
    insertTopLevelItem(d->binaryIfaces.size() - 1, item);
    QPushButton* const findButton = new QPushButton(i18n("Find"));
    setItemWidget(item, Button, findButton);
    QLabel* const downloadLabel   = new QLabel(i18n(" or <a href=\"%1\">download</a>", binary.url().url()));
    downloadLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    downloadLabel->setOpenExternalLinks(true);
    setItemWidget(item, Link, downloadLabel);

    // Starts a dialog to find the binary

    connect(findButton, SIGNAL(clicked(bool)),
            &binary, SLOT(slotNavigateAndCheck()));

    // Rechecks full validity when a binary is found and valid

    connect(&binary, SIGNAL(signalBinaryValid()),
            this, SLOT(slotAreBinariesFound()));

    // Scans (if no binary were found) a new directory where a binary was found

    connect(&binary, SIGNAL(signalSearchDirectoryAdded(QString)),
            this, SIGNAL(signalAddPossibleDirectory(QString)));

    connect(this, SIGNAL(signalAddPossibleDirectory(QString)),
            &binary, SLOT(slotAddPossibleSearchDirectory(QString)));

    // Force scan of a new directory

    connect(this, SIGNAL(signalAddDirectory(QString)),
            &binary, SLOT(slotAddSearchDirectory(QString)));

    d->downloadLabel = new QLabel(i18n(
        "<qt><p><font color=\"red\"><b>Warning:</b> Some necessary binaries have not been found on "
        "your system. If you have these binaries installed, please click the 'Find' button to locate them on your "
        "system, otherwise please download and install them to proceed.</font></p></qt>"), parentWidget());

    QGridLayout* const layout = qobject_cast<QGridLayout*>(parentWidget()->layout());

    if (layout)
    {
        layout->addWidget(d->downloadLabel, layout->rowCount(), 0);
    }

    d->downloadLabel->setContentsMargins(20, 20, 20, 20);
    d->downloadLabel->setWordWrap(true);
    d->downloadLabel->hide();
}

void DBinarySearch::addDirectory(const QString& dir)
{
    Q_EMIT signalAddPossibleDirectory(dir);
}

bool DBinarySearch::allBinariesFound()
{
    bool ret = true;

    for (DBinaryIface* const binary : std::as_const(d->binaryIfaces))
    {
        int index = d->binaryIfaces.indexOf(binary);

        if      (binary->isValid())
        {
            if (!binary->developmentVersion())
            {
                d->items[index]->setIcon(Status, QIcon::fromTheme(QLatin1String("dialog-ok-apply")).pixmap(16, 16));
                d->items[index]->setToolTip(Status, QString());
            }
            else
            {
                d->items[index]->setIcon(Status, QIcon::fromTheme(QLatin1String("dialog-warning")).pixmap(16, 16));
                d->items[index]->setToolTip(Status, i18n("A development version has been detect. "
                                                         "There is no guarantee on the behavior of this binary."));
                d->downloadLabel->show();
            }

            d->items[index]->setText(Version, binary->version());
            QPushButton* const btn = qobject_cast<QPushButton*>(itemWidget(d->items[index], Button));

            if (btn)
            {
                btn->setText(i18n("Change"));
            }
        }
        else if (binary->hasError())
        {
            d->items[index]->setIcon(Status, QIcon::fromTheme(QLatin1String("data-error")).pixmap(16, 16));
            d->items[index]->setToolTip(Status, i18n("The binary file found does not appear to be "
                                                     "working as intended."));

            ret = false;
        }
        else
        {
            ret = false;
        }
    }

    if (ret)
    {
        d->downloadLabel->hide();
    }

    return ret;
}

void DBinarySearch::slotAreBinariesFound()
{
    bool allFound = allBinariesFound();

    Q_EMIT signalBinariesFound(allFound);

    qCDebug(DIGIKAM_GENERAL_LOG) << "All Binaries Found : " << allFound;
}

} // namespace Digikam

#include "moc_dbinarysearch.cpp"
