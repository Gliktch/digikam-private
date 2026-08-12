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

#include "openfilepage.h"

// Qt includes

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "applicationsettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN OpenFilePage::Private
{
public:

    Private() = default;

    QRadioButton* openAsPreview     = nullptr;
    QRadioButton* openInEditor      = nullptr;
    QButtonGroup* openFileBehavior  = nullptr;
};

OpenFilePage::OpenFilePage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Configure Open File Behavior</b>")),
      d          (new Private)
{
    const int spacing       = layoutSpacing();

    DVBox* const vbox       = new DVBox(this);
    QLabel* const label1    = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Specify how images should be opened when left-clicked on in the icon view:</p>"
                         "</qt>"));

    QWidget* const btns     = new QWidget(vbox);
    QVBoxLayout* const vlay = new QVBoxLayout(btns);

    d->openFileBehavior     = new QButtonGroup(btns);
    d->openAsPreview        = new QRadioButton(btns);
    d->openAsPreview->setText(i18n("Open a preview"));
    d->openAsPreview->setChecked(true);
    d->openFileBehavior->addButton(d->openAsPreview);

    d->openInEditor         = new QRadioButton(btns);
    d->openInEditor->setText(i18n("Open in the editor"));
    d->openFileBehavior->addButton(d->openInEditor);

    vlay->addWidget(d->openAsPreview);
    vlay->addWidget(d->openInEditor);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    QLabel* const label2    = new QLabel(vbox);
    label2->setWordWrap(true);
    label2->setText(i18n("<qt>"
                         "<p><i>Note:</i> using a preview is always faster than using the editor, "
                         "especially when checking a series of shots. However, you cannot change or fix the image "
                         "in preview mode. "
                         "Note that if you want to compare images quickly, it is often better to use the light table: "
                         "images can be displayed side-by-side, and synchronized zooming and panning can be performed.</p>"
                         "</qt>"));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-pictures"))); //image-stack
}

OpenFilePage::~OpenFilePage()
{
    delete d;
}

void OpenFilePage::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Album Settings"));
    group.writeEntry(QLatin1String("Item Left Click Action"), (int)(d->openInEditor->isChecked() ?
                     ApplicationSettings::StartEditor : ApplicationSettings::ShowPreview));

    config->sync();
}

} // namespace Digikam

#include "moc_openfilepage.cpp"
