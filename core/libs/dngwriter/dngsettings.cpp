/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-24
 * Description : DNG save settings widgets
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngsettings.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <QApplication>
#include <QIcon>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_globals.h"
#include "dcombobox.h"
#include "dngwriter.h"
#include "exiftoolerrorview.h"
#include "exiftoolparser.h"

namespace Digikam
{

class Q_DECL_HIDDEN DNGSettings::Private
{
public:

    Private() = default;

    QLabel*             previewModeLabel        = nullptr;

    QCheckBox*          compressLossLess        = nullptr;
    QCheckBox*          backupOriginalRawFile   = nullptr;

    DComboBox*          previewModeCB           = nullptr;

    ExifToolErrorView*  errorView               = nullptr;
};

DNGSettings::DNGSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing                    = layoutSpacing();


    QGridLayout* const settingsBoxLayout = new QGridLayout(this);

    // ------------------------------------------------------------------------

    d->backupOriginalRawFile = new QCheckBox(i18n("Embed Original File"), this);
    d->compressLossLess      = new QCheckBox(i18n("Lossless Compression"), this);

    // ------------------------------------------------------------------------

    d->previewModeLabel = new QLabel(i18n("JPEG Preview:"), this);
    d->previewModeCB    = new DComboBox(this);
    d->previewModeCB->insertItem(DNGWriter::NONE,      i18nc("embedded preview type in dng file", "None"));
    d->previewModeCB->insertItem(DNGWriter::MEDIUM,    i18nc("embedded preview type in dng file", "Medium"));
    d->previewModeCB->insertItem(DNGWriter::FULL_SIZE, i18nc("embedded preview type in dng file", "Full size"));
    d->previewModeCB->setDefaultIndex(DNGWriter::FULL_SIZE);

    d->errorView        = new ExifToolErrorView(this);
    d->errorView->setErrorText(i18n("Warning: ExifTool is not available to post-process metadata. "
                                    "Without ExifTool, DNG will not include all RAW source properties."));

    settingsBoxLayout->addWidget(d->backupOriginalRawFile, 0, 0, 1, 1);
    settingsBoxLayout->addWidget(d->compressLossLess,      1, 0, 1, 1);
    settingsBoxLayout->addWidget(d->previewModeLabel,      3 ,0, 1, 1);
    settingsBoxLayout->addWidget(d->previewModeCB,         4 ,0 ,1, 1);
    settingsBoxLayout->addWidget(d->errorView,             5 ,0 ,1, 1);
    settingsBoxLayout->setRowStretch(9, 10);
    settingsBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    settingsBoxLayout->setSpacing(spacing);

    // ------------------------------------------------------------------------

    connect(d->backupOriginalRawFile, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->compressLossLess, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->previewModeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->errorView, SIGNAL(signalSetupExifTool()),
            this, SIGNAL(signalSetupExifTool()));

    // ------------------------------------------------------------------------

    slotSetupChanged();
}

DNGSettings::~DNGSettings()
{
    delete d;
}

void DNGSettings::setDefaultSettings()
{
    d->previewModeCB->slotReset();
    setCompressLossLess(true);
    setBackupOriginalRawFile(false);
}

void DNGSettings::setPreviewMode(int mode)
{
    d->previewModeCB->setCurrentIndex(mode);
}

int DNGSettings::previewMode() const
{
    return d->previewModeCB->currentIndex();
}

void DNGSettings::setCompressLossLess(bool b)
{
    d->compressLossLess->setChecked(b);
}

bool DNGSettings::compressLossLess() const
{
    return d->compressLossLess->isChecked();
}

void DNGSettings::setBackupOriginalRawFile(bool b)
{
    d->backupOriginalRawFile->setChecked(b);
}

bool DNGSettings::backupOriginalRawFile() const
{
    return d->backupOriginalRawFile->isChecked();
}

void DNGSettings::slotSetupChanged()
{
    QScopedPointer<ExifToolParser> const parser(new ExifToolParser(this));

    d->errorView->setVisible(!parser->exifToolAvailable());
}

} // namespace Digikam

#include "moc_dngsettings.cpp"
