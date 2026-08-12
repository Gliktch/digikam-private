/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "htmlwizard.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include <QComboBox>
#include <QListWidget>
#include <QTextBrowser>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dwizardpage.h"
#include "dconfigdlgmngr.h"
#include "digikam_debug.h"
#include "abstractthemeparameter.h"
#include "galleryinfo.h"
#include "invisiblebuttongroup.h"
#include "htmlintropage.h"
#include "htmloutputpage.h"
#include "htmlselectionpage.h"
#include "htmlthemepage.h"
#include "htmlparameterspage.h"
#include "htmlimagesettingspage.h"
#include "htmlfinalpage.h"

namespace DigikamGenericHtmlGalleryPlugin
{

class Q_DECL_HIDDEN HTMLWizard::Private
{
public:

    Private() = default;

public:

    GalleryInfo*           info                 = nullptr;
    DConfigDlgMngr*        configManager        = nullptr;

    HTMLIntroPage*         introPage            = nullptr;
    HTMLSelectionPage*     selectionPage        = nullptr;
    HTMLThemePage*         themePage            = nullptr;
    HTMLParametersPage*    parametersPage       = nullptr;
    HTMLImageSettingsPage* imageSettingsPage    = nullptr;
    HTMLOutputPage*        outputPage           = nullptr;
    HTMLFinalPage*         finalPage            = nullptr;
};

HTMLWizard::HTMLWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("HTML Gallery Dialog")),
      d         (new Private)
{
    setOption(QWizard::NoCancelButtonOnLastPage);
    setWindowTitle(i18nc("@title:window", "Create HTML Gallery"));

    d->info = new GalleryInfo(iface);
    d->info->load();

    d->introPage         = new HTMLIntroPage(this,         i18n("Welcome to HTML Gallery Tool"));
    d->selectionPage     = new HTMLSelectionPage(this,     i18n("Items Selection"));
    d->themePage         = new HTMLThemePage(this,         i18n("Theme Selection"));
    d->parametersPage    = new HTMLParametersPage(this,    i18n("Theme Parameters"));
    d->imageSettingsPage = new HTMLImageSettingsPage(this, i18n("Image Settings"));
    d->outputPage        = new HTMLOutputPage(this,        i18n("Output Settings"));
    d->finalPage         = new HTMLFinalPage(this,         i18n("Generating Gallery"));
    d->configManager     = new DConfigDlgMngr(this, d->info);
    d->configManager->updateWidgets();
}

HTMLWizard::~HTMLWizard()
{
    delete d;
}

void HTMLWizard::setItemsList(const QList<QUrl>& urls)
{
    d->selectionPage->setItemsList(urls);
}

bool HTMLWizard::validateCurrentPage()
{
    if (!DWizardDlg::validateCurrentPage())
    {
        return false;
    }

    if (currentPage() == d->outputPage)
    {
        GalleryTheme::Ptr curtheme                     = galleryTheme();
        QString themeInternalName                      = curtheme->internalName();
        d->info->setTheme(themeInternalName);

        GalleryTheme::ParameterList parameterList      = curtheme->parameterList();
        GalleryTheme::ParameterList::ConstIterator it  = parameterList.constBegin();
        GalleryTheme::ParameterList::ConstIterator end = parameterList.constEnd();

        for ( ; it != end ; ++it)
        {
            const AbstractThemeParameter* const themeParameter = *it;
            QByteArray parameterInternalName                   = themeParameter->internalName();
            QWidget* const widget                              = d->parametersPage->themeParameterWidgetFromName(parameterInternalName);
            QString value                                      = themeParameter->valueFromWidget(widget);

            d->info->setThemeParameterValue(themeInternalName,
                                             QString::fromLatin1(parameterInternalName),
                                             value);
        }

        d->configManager->updateSettings();
        d->info->save();
    }

    return true;
}

int HTMLWizard::nextId() const
{
    if (currentPage() == d->themePage)
    {
        GalleryTheme::Ptr theme = galleryTheme();

        if (theme && (theme->parameterList().size() > 0))
        {
            // Enable theme parameters page as next page if there is any parameter.

            return d->parametersPage->id();
        }

        return d->imageSettingsPage->id();
    }

    return DWizardDlg::nextId();
}

GalleryInfo* HTMLWizard::galleryInfo() const
{
    return d->info;
}

GalleryTheme::Ptr HTMLWizard::galleryTheme() const
{
    return d->themePage->currentTheme();
}

} // namespace DigikamGenericHtmlGalleryPlugin

#include "moc_htmlwizard.cpp"
