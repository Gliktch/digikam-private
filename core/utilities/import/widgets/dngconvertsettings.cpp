/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2016-05-03
 * Description : DNG convert settings for camera interface.
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2016 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngconvertsettings.h"

// Qt includes

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "dngsettings.h"
#include "setup.h"

namespace Digikam
{

class Q_DECL_HIDDEN DNGConvertSettings::Private
{
public:

    Private() = default;

    QCheckBox*   convertDng  = nullptr;
    QLabel*      dngLink     = nullptr;
    DNGSettings* dngSettings = nullptr;
};

DNGConvertSettings::DNGConvertSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing           = layoutSpacing();


    QVBoxLayout* const mainVlay = new QVBoxLayout(this);
    QHBoxLayout* const convHlay = new QHBoxLayout;

    d->convertDng               = new QCheckBox(i18nc("@option:check", "Convert RAW images to"), this);
    d->dngSettings              = new DNGSettings(this);

    d->dngLink                  = new QLabel(this);
    d->dngLink->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->dngLink->setOpenExternalLinks(true);
    d->dngLink->setWordWrap(false);
    d->dngLink->setText(QString::fromUtf8("<p><a href='https://en.wikipedia.org/wiki/Digital_Negative_(file_format)'>%1</a></p>")
                        .arg(i18nc("@label", "DNG container")));

    convHlay->addWidget(d->convertDng);
    convHlay->addWidget(d->dngLink);
    convHlay->addStretch(10);
    convHlay->setContentsMargins(spacing, spacing, spacing, spacing);
    convHlay->setSpacing(0);

    mainVlay->addLayout(convHlay);
    mainVlay->addWidget(d->dngSettings);
    mainVlay->setContentsMargins(QMargins());
    mainVlay->setSpacing(0);

    setWhatsThis(i18nc("@info", "Set here all options to convert RAW images to DNG container automatically "
                                "as they are downloaded."));
    d->convertDng->setWhatsThis(i18nc("@info", "Enable this option to convert RAW images to DNG container automatically."));

    setLayout(mainVlay);

    // ---------------------------------------------------------------------------------------

    connect(d->convertDng, SIGNAL(toggled(bool)),
            d->dngSettings, SLOT(setEnabled(bool)));

    connect(d->convertDng, SIGNAL(toggled(bool)),
            this, SIGNAL(signalDownloadNameChanged()));

    connect(d->dngSettings, SIGNAL(signalSetupExifTool()),
            this, SLOT(slotSetupExifTool()));
}

DNGConvertSettings::~DNGConvertSettings()
{
    delete d;
}

void DNGConvertSettings::readSettings(const KConfigGroup& group)
{
    d->dngSettings->setBackupOriginalRawFile(group.readEntry(QLatin1String("BackupOriginalRawFile"), false));
    d->dngSettings->setCompressLossLess(group.readEntry(QLatin1String("CompressLossLess"),           true));
    d->dngSettings->setPreviewMode(group.readEntry(QLatin1String("PreviewMode"),                     (int)DNGWriter::FULL_SIZE));
    d->convertDng->setChecked(group.readEntry(QLatin1String("ConvertDng"),                           false));

    d->dngSettings->setEnabled(d->convertDng->isChecked());
}

void DNGConvertSettings::saveSettings(KConfigGroup& group)
{
    group.writeEntry(QLatin1String("BackupOriginalRawFile"), d->dngSettings->backupOriginalRawFile());
    group.writeEntry(QLatin1String("CompressLossLess"),      d->dngSettings->compressLossLess());
    group.writeEntry(QLatin1String("PreviewMode"),           d->dngSettings->previewMode());
    group.writeEntry(QLatin1String("ConvertDng"),            d->convertDng->isChecked());
}

void DNGConvertSettings::settings(DownloadSettings* const settings)
{
    settings->backupRaw   = d->dngSettings->backupOriginalRawFile();
    settings->compressDng = d->dngSettings->compressLossLess();
    settings->previewMode = d->dngSettings->previewMode();
    settings->convertDng  = d->convertDng->isChecked();
}

void DNGConvertSettings::slotSetupExifTool()
{
    if (Setup::execExifTool(nullptr))
    {
        d->dngSettings->slotSetupChanged();
    }
}

} // namespace Digikam

#include "moc_dngconvertsettings.cpp"
