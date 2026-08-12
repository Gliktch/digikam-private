/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "htmlthemepage.h"

// Qt includes

#include <QIcon>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "galleryinfo.h"
#include "htmlwizard.h"
#include "dlayoutbox.h"

namespace DigikamGenericHtmlGalleryPlugin
{

class Q_DECL_HIDDEN HTMLThemePage::Private
{
public:

    Private() = default;

public:

    QListWidget*  themeList = nullptr;
    QTextBrowser* themeInfo = nullptr;
};

HTMLThemePage::HTMLThemePage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private)
{
    setObjectName(QLatin1String("ThemePage"));

    DHBox* const hbox = new DHBox(this);

    d->themeList      = new QListWidget(hbox);
    d->themeList->setObjectName(QLatin1String("ThemeList"));

    d->themeInfo      = new QTextBrowser(hbox);
    d->themeInfo->setObjectName(QLatin1String("ThemeInfo"));

    hbox->setContentsMargins(QMargins());
    hbox->setSpacing(layoutSpacing());

    connect(d->themeList, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotThemeSelectionChanged()));

    setPageWidget(hbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("application-x-theme")));
}

HTMLThemePage::~HTMLThemePage()
{
    delete d;
}

void HTMLThemePage::initializePage()
{
    const HTMLWizard* const wizard        = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
    {
        return;
    }

    const GalleryInfo* const info         = wizard->galleryInfo();
    GalleryTheme::List list               = GalleryTheme::getList();
    GalleryTheme::List::ConstIterator it  = list.constBegin();
    GalleryTheme::List::ConstIterator end = list.constEnd();

    d->themeList->clear();

    for ( ; it != end ; ++it)
    {
        GalleryTheme::Ptr theme      = *it;
        ThemeListBoxItem* const item = new ThemeListBoxItem(theme);
        d->themeList->addItem(item);

        if (theme->internalName() == info->theme())
        {
            d->themeList->setCurrentItem(item);
        }
    }

    // Set page states, whoch can only be disabled after they have *all* been added.

    slotThemeSelectionChanged();
}

bool HTMLThemePage::validatePage()
{
    if (!d->themeList->currentItem())
    {
        return false;
    }

    return true;
}

void HTMLThemePage::slotThemeSelectionChanged()
{
    if (d->themeList->currentItem())
    {
        GalleryTheme::Ptr curTheme = currentTheme();
        QString url                = curTheme->authorUrl();
        QString author             = curTheme->authorName();

        if (!url.isEmpty())
        {
            author = QString::fromUtf8("<a href='%1'>%2</a>").arg(url).arg(author);
        }

        QString preview               = curTheme->previewUrl();
        QString image                 = QLatin1String("");

        if (!preview.isEmpty())
        {
            image = QString::fromUtf8("<img src='%1/%2' /><br><br>")
                        .arg(curTheme->directory(), curTheme->previewUrl());
        }

        QString advSet = (curTheme->parameterList().size() > 0) ? i18n("can be customized")
                                                                : i18n("no customization available");
        QString txt    = image + QString::fromUtf8("<b>%3</b><br><br>%4<br><br>")
                                   .arg(curTheme->name(), curTheme->comment())
                               + i18n("Author: %1<br><br>", author)
                               + QString::fromUtf8("<i>%1</i>").arg(advSet);

        d->themeInfo->setHtml(txt);
    }
    else
    {
        d->themeInfo->clear();
    }
}

GalleryTheme::Ptr HTMLThemePage::currentTheme() const
{
    const ThemeListBoxItem* const item = dynamic_cast<ThemeListBoxItem*>(d->themeList->currentItem());

    if (item)
    {
        return item->m_theme;
    }

    return GalleryTheme::Ptr(nullptr);
}

} // namespace DigikamGenericHtmlGalleryPlugin

#include "moc_htmlthemepage.cpp"
