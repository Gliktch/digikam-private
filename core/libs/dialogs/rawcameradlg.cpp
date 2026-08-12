/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-04-07
 * Description : Raw camera list dialog
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rawcameradlg.h"

// Qt includes

#include <QStringList>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QTreeWidget>
#include <QHeaderView>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "drawdecoder.h"

namespace Digikam
{

class Q_DECL_HIDDEN RawCameraDlg::Private
{
public:

    Private() = default;

    QLabel*        header    = nullptr;
    SearchTextBar* searchBar = nullptr;
};

RawCameraDlg::RawCameraDlg(QWidget* const parent)
    : InfoDlg(parent),
      d      (new Private)
{
    setWindowTitle(i18nc("@title:window", "List of Supported RAW Cameras"));

    QStringList list = DRawDecoder::supportedCamera();

    // --------------------------------------------------------

    d->header    = new QLabel(this);
    d->searchBar = new SearchTextBar(this, QLatin1String("RawCameraDlgSearchBar"));
    updateHeader();

    listView()->setColumnCount(1);
    listView()->setHeaderLabels(QStringList() << QLatin1String("Camera Model")); // Header is hidden. No i18n here.
    listView()->header()->hide();

    for (QStringList::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        new QTreeWidgetItem(listView(), QStringList() << *it);
    }

    // --------------------------------------------------------

    QGridLayout* const  grid = dynamic_cast<QGridLayout*>(mainWidget()->layout());
    grid->addWidget(d->header,    1, 0, 1, -1);
    grid->addWidget(d->searchBar, 3, 0, 1, -1);

    // --------------------------------------------------------

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));
}

RawCameraDlg::~RawCameraDlg()
{
    delete d;
}

void RawCameraDlg::slotSearchTextChanged(const SearchTextSettings& settings)
{
    bool query     = false;
    int  results   = 0;
    QString search = settings.text.toLower();

    QTreeWidgetItemIterator it(listView());

    while (*it)
    {
        QTreeWidgetItem* const item  = *it;

        if (item->text(0).toLower().contains(search, settings.caseSensitive))
        {
            ++results;
            query = true;
            item->setHidden(false);
        }
        else
        {
            item->setHidden(true);
        }

        ++it;
    }

    updateHeader(results);
    d->searchBar->slotSearchResult(query);
}

void RawCameraDlg::updateHeader(int results)
{
    QString librawVer = DRawDecoder::librawVersion();
    QStringList list  = DRawDecoder::supportedCamera();

    if (!results)
    {
        d->header->setText(i18np("Using LibRaw version %2<br>"
                                 "1 model on the list",
                                 "Using LibRaw version %2<br>"
                                 "%1 models on the list",
                                 list.count(),
                                 librawVer
                                 ));
    }
    else
    {
        d->header->setText(i18np("Using LibRaw version %2<br>"
                                 "1 model on the list (found: %3)",
                                 "Using LibRaw version %2<br>"
                                 "%1 models on the list (found: %3)",
                                 list.count(),
                                 librawVer,
                                 results));
    }
}

} // namespace Digikam

#include "moc_rawcameradlg.cpp"
