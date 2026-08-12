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

#include "rawpage.h"

// Qt includes

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN RawPage::Private
{
public:

    Private() = default;

    QRadioButton* openDirectly = nullptr;
    QRadioButton* useRawImport = nullptr;
    QButtonGroup* rawHandling  = nullptr;
};

RawPage::RawPage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Configure Raw File Handling</b>")),
      d          (new Private)
{
    const int spacing       = layoutSpacing();

    DVBox* const vbox       = new DVBox(this);
    QLabel* const label1    = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Set here how you want to open Raw images in the editor:</p>"
                         "</qt>"));

    QWidget* const btns     = new QWidget(vbox);
    QVBoxLayout* const vlay = new QVBoxLayout(btns);

    d->rawHandling          = new QButtonGroup(btns);
    d->openDirectly         = new QRadioButton(btns);
    d->openDirectly->setText(i18n("Open directly, with adjustments made automatically"));
    d->openDirectly->setChecked(true);
    d->rawHandling->addButton(d->openDirectly);

    d->useRawImport         = new QRadioButton(btns);
    d->useRawImport->setText(i18n("Use the Raw import tool to adjust corrections manually"));
    d->rawHandling->addButton(d->useRawImport);

    vlay->addWidget(d->openDirectly);
    vlay->addWidget(d->useRawImport);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    QLabel* const label2    = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> the Raw import tool is designed for advanced users who "
                         "want to have the best control over the image. "
                         "This requires more time in your workflow.</p>"
                         "</qt>"));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("image-x-adobe-dng")));
}

RawPage::~RawPage()
{
    delete d;
}

void RawPage::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    group.writeEntry(QLatin1String("UseRawImportTool"), d->useRawImport->isChecked());
    config->sync();
}

} // namespace Digikam

#include "moc_rawpage.cpp"
