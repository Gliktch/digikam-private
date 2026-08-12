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

#include "previewpage.h"

// Qt includes

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN PreviewPage::Private
{
public:

    Private() = default;

    QRadioButton* loadReduced       = nullptr;
    QRadioButton* loadimage         = nullptr;
    QButtonGroup* previewBehavior   = nullptr;
};

PreviewPage::PreviewPage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Configure Preview Behavior</b>")),
      d          (new Private)
{
    const int spacing       = layoutSpacing();

    DVBox* const vbox       = new DVBox(this);
    QLabel* const label1    = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Set here how images are displayed in preview mode and on the light table:</p>"
                         "</qt>"));

    QWidget* const btns     = new QWidget(vbox);
    QVBoxLayout* const vlay = new QVBoxLayout(btns);

    d->previewBehavior      = new QButtonGroup(btns);
    d->loadReduced          = new QRadioButton(btns);
    d->loadReduced->setText(i18n("Load reduced version of image"));
    d->loadReduced->setChecked(true);
    d->previewBehavior->addButton(d->loadReduced);

    d->loadimage            = new QRadioButton(btns);
    d->loadimage->setText(i18n("Load image"));
    d->previewBehavior->addButton(d->loadimage);

    vlay->addWidget(d->loadReduced);
    vlay->addWidget(d->loadimage);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    QLabel* const label2    = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> loading a reduced version of an image is faster but can differ "
                         "from the original, especially with Raw. In this case, a JPEG version "
                         "of Raw pre-processed by camera is loaded, instead of the real image data. This JPEG "
                         "image is embedded in the file metadata and is used by the camera to display "
                         "a Raw image faster to a TV screen.</p>"
                         "</qt>"));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("view-preview")));
}

PreviewPage::~PreviewPage()
{
    delete d;
}

void PreviewPage::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group        = config->group(QLatin1String("Album Settings"));
    group.writeEntry(QLatin1String("Preview Load Full Image Size"), d->loadimage->isChecked());

    config->sync();
}

} // namespace Digikam

#include "moc_previewpage.cpp"
