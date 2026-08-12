/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Import Filter combobox
 *
 * SPDX-FileCopyrightText: 2010-2011 by Petri Damstén <petri dot damsten at iki dot fi>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importfiltercombobox.h"

// Qt includes

#include <QStringList>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "camiteminfo.h"

namespace Digikam
{

// JVC camera (see bug #133185).
const QString ImportFilterComboBox::defaultIgnoreNames(QLatin1String("mgr_data pgr_mgr"));

// HP Photosmart camera (see bug #156338).
// Minolta camera in PTP mode
const QString ImportFilterComboBox::defaultIgnoreExtensions(QLatin1String("dsp dps"));

// ---------------------------------------------------------------------------------

class Q_DECL_HIDDEN ImportFilterComboBox::Private
{
public:

    Private()
    {
        createFilters();
    }

    void createFilters()
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(QLatin1String("Import Filters"));
        QString ignoreNames       = group.readEntry(QLatin1String("IgnoreNames"),
                                                    defaultIgnoreNames).toLower();
        QString ignoreExtensions  = group.readEntry(QLatin1String("IgnoreExtensions"),
                                                    defaultIgnoreExtensions).toLower();

        for (int i = 0 ; true ; ++i)
        {
            QString filter = group.readEntry(QString::fromUtf8("Filter%1").arg(i), QString());

            if (filter.isEmpty())
            {
                break;
            }

            filter.append(QLatin1Char('|') + ignoreNames);
            filter.append(QLatin1Char('|') + ignoreExtensions);

            Filter* const f = new Filter;
            f->fromString(filter);
            filters.append(f);
        }

        ImportFilterComboBox::defaultFilters(&filters);
        currentFilter = group.readEntry(QLatin1String("CurrentFilter"), 0);
    }

    ~Private()
    {
        qDeleteAll(filters);
    }

public:

    int            currentFilter = 0;
    QList<Filter*> filters;
};

// -------------------------------------------------------

ImportFilterComboBox::ImportFilterComboBox(QWidget* const parent)
    : QComboBox(parent),
      d        (new Private)
{
    fillCombo();

    connect(this, SIGNAL(activated(int)),
            this, SLOT(indexChanged(int)));
}

ImportFilterComboBox::~ImportFilterComboBox()
{
    delete d;
}

Filter* ImportFilterComboBox::currentFilter() const
{
    Filter* const filter = d->filters.value(d->currentFilter);

    return filter;
}

void ImportFilterComboBox::defaultFilters(FilterList* const filters)
{
    if (filters->count() == 0)
    {
        Filter* const f1 = new Filter;
        f1->name         = i18nc("@item:inlistbox", "All Files");
        filters->append(f1);

        Filter* const f2 = new Filter;
        f2->name         = i18nc("@item:inlistbox", "Only New Files");
        f2->onlyNew      = true;
        filters->append(f2);

        Filter* const f3 = new Filter;
        f3->name         = i18nc("@item:inlistbox", "Raw Files");
        f3->mimeFilter   = QLatin1String("image/x-nikon-nef;image/x-fuji-raf;image/x-adobe-dng;"
                                         "image/x-panasonic-raw;image/x-olympus-orf;image/x-kodak-dcr;"
                                         "image/x-kodak-k25;image/x-sony-arw;image/x-minolta-mrw;"
                                         "image/x-kodak-kdc;image/x-sigma-x3f;image/x-sony-srf;"
                                         "image/x-pentax-pef;image/x-panasonic-raw2;image/x-canon-crw;"
                                         "image/x-sony-sr2;image/x-canon-cr2;image/x-canon-cr3");
        filters->append(f3);

        Filter* const f4 = new Filter;
        f4->name         = i18nc("@item:inlistbox", "JPG/TIFF Files");
        f4->mimeFilter   = QLatin1String("image/jpeg;image/tiff");
        filters->append(f4);

        Filter* const f5 = new Filter;
        f5->name         = i18nc("@item:inlistbox", "Video Files");
        f5->mimeFilter   = QLatin1String("video/quicktime;video/mp4;video/x-msvideo;video/mpeg");
        filters->append(f5);
    }
}

void ImportFilterComboBox::fillCombo()
{
    clear();

    for (Filter* const f : std::as_const(d->filters))
    {
        addItem(f->name);
    }

    setCurrentIndex(d->currentFilter);
}

void ImportFilterComboBox::indexChanged(int index)
{
    if (index != d->currentFilter)
    {
        d->currentFilter     = index;
        Filter* const filter = d->filters.value(d->currentFilter);

        Q_EMIT signalFilterChanged(filter);
    }
}

void ImportFilterComboBox::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Import Filters"));

    group.writeEntry(QLatin1String("CurrentFilter"), d->currentFilter);
}

void ImportFilterComboBox::updateFilter()
{
    int lastCurrent  = d->currentFilter;
    qDeleteAll(d->filters);
    d->filters.clear();
    d->createFilters();

    d->currentFilter = lastCurrent;
    fillCombo();
}

} // namespace Digikam

#include "moc_importfiltercombobox.cpp"
