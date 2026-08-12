/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadatapage.h"

// Qt includes

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "metaenginesettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN MetadataPage::Private
{
public:

    Private() = default;

    QRadioButton* doNothing         = nullptr;
    QRadioButton* storeInFiles      = nullptr;
    QButtonGroup* metadataStorage   = nullptr;
};

MetadataPage::MetadataPage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Configure Metadata Storage to Files</b>")),
      d          (new Private)
{
    const int spacing        = layoutSpacing();

    DVBox* const vbox        = new DVBox(this);
    QLabel* const label1     = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Set here if you want to store the information assigned to items in digiKam in the files' "
                         "metadata, to improve interoperability with other photo management programs:</p>"
                         "</qt>"));

    QWidget* const btns      = new QWidget(vbox);
    QVBoxLayout* const vlay  = new QVBoxLayout(btns);

    d->metadataStorage       = new QButtonGroup(btns);
    d->doNothing             = new QRadioButton(btns);
    d->doNothing->setText(i18n("Do nothing"));
    d->doNothing->setChecked(true);
    d->metadataStorage->addButton(d->doNothing);

    d->storeInFiles          = new QRadioButton(btns);
    d->storeInFiles->setText(i18n("Add information to files"));
    d->metadataStorage->addButton(d->storeInFiles);

    vlay->addWidget(d->doNothing);
    vlay->addWidget(d->storeInFiles);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    QLabel* const label2     = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> recording information to the files' metadata can slow down photo "
                         "management operations.</p>"
                         "</qt>"));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("format-text-code")));
}

MetadataPage::~MetadataPage()
{
    delete d;
}

void MetadataPage::saveSettings()
{
    MetaEngineSettingsContainer settings;
    settings.saveTags       = d->storeInFiles->isChecked();
    settings.saveFaceTags   = d->storeInFiles->isChecked();
    settings.saveComments   = d->storeInFiles->isChecked();
    settings.saveDateTime   = d->storeInFiles->isChecked();
    settings.saveRating     = d->storeInFiles->isChecked();
    settings.savePickLabel  = d->storeInFiles->isChecked();
    settings.saveColorLabel = d->storeInFiles->isChecked();
    settings.saveTemplate   = d->storeInFiles->isChecked();
    settings.savePosition   = d->storeInFiles->isChecked();
    MetaEngineSettings::instance()->setSettings(settings);
}

} // namespace Digikam

#include "moc_metadatapage.cpp"
