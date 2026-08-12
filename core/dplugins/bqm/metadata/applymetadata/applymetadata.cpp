/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-02-11
 * Description : apply metadata batch tool with ExifTool.
 *
 * SPDX-FileCopyrightText: 2024 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "applymetadata.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QLabel>
#include <QStyle>
#include <QFileInfo>
#include <QApplication>
#include <QTemporaryFile>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "dlayoutbox.h"
#include "dfileselector.h"
#include "exiftoolparser.h"
#include "dfileoperations.h"

namespace DigikamBqmApplyMetadataPlugin
{

class Q_DECL_HIDDEN ApplyMetadata::Private
{
public:

    Private() = default;

public:

    DFileSelector* fileSelector   = nullptr;
    QLabel*        descFileLabel  = nullptr;

    bool           changeSettings = false;
};

ApplyMetadata::ApplyMetadata(QObject* const parent)
    : BatchTool(QLatin1String("ApplyMetadata"), MetadataTool, parent),
      d        (new Private)
{
}

ApplyMetadata::~ApplyMetadata()
{
    delete d;
}

BatchTool* ApplyMetadata::clone(QObject* const parent) const
{
    return new ApplyMetadata(parent);
}

void ApplyMetadata::registerSettingsWidget()
{
    const int spacing    = layoutSpacing();


    DVBox* const vbox    = new DVBox;

    d->descFileLabel     = new QLabel(vbox);
    d->descFileLabel->setText(i18nc("@label", "Select an image or JSON file supported by ExifTool "
                                              "to apply the file metadata to the images."));
    d->descFileLabel->setWordWrap(true);
    d->fileSelector      = new DFileSelector(vbox);
    d->fileSelector->setFileDlgMode(QFileDialog::ExistingFile);

    QWidget* const space = new QWidget(vbox);
    vbox->setStretchFactor(space, 10);
    vbox->setContentsMargins(spacing, spacing, spacing, spacing);

    m_settingsWidget     = vbox;

    connect(d->fileSelector, SIGNAL(signalUrlSelected(QUrl)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings ApplyMetadata::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("MetadataFile"), QString());

    return settings;
}

void ApplyMetadata::slotAssignSettings2Widget()
{
    d->changeSettings = false;

    d->fileSelector->setFileDlgPath(settings().value(QLatin1String("MetadataFile")).toString());

    d->changeSettings = true;
}

void ApplyMetadata::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;

        settings.insert(QLatin1String("MetadataFile"), d->fileSelector->fileDlgPath());

        BatchTool::slotSettingsChanged(settings);
    }
}

bool ApplyMetadata::toolOperations()
{
    bool ret = true;

    if (image().isNull())
    {
        QFile::remove(outputUrl().toLocalFile());
        ret = DFileOperations::copyFile(inputUrl().toLocalFile(), outputUrl().toLocalFile());
    }
    else
    {
        ret = savefromDImg();
    }

    QFileInfo metaInfo(settings().value(QLatin1String("MetadataFile")).toString());

    if (!ret || !metaInfo.exists())
    {
        setErrorDescription(i18nc("@info", "Apply Metadata: No image or JSON file selected."));
        return false;
    }

    QScopedPointer<ExifToolParser> const parser(new ExifToolParser(this));

    if (!parser->exifToolAvailable())
    {
        setErrorDescription(i18nc("@info", "Apply Metadata: ExifTool is not available."));
        return false;
    }

    if (metaInfo.suffix().toUpper() == QLatin1String("JSON"))
    {
        QString fileTemplate = QDir::tempPath() + QLatin1String("/ApplyMetadata-XXXXXX.json");
        QTemporaryFile tempFile(fileTemplate);

        if (!tempFile.open())
        {
            return false;
        }

        QFile jsonFileRead(metaInfo.filePath());

        if (!jsonFileRead.open(QIODevice::ReadOnly))
        {
            return false;
        }

        QString jsonSource = QString::fromUtf8("\"SourceFile\": \"%1\",");
        QString jsonString = QString::fromUtf8(jsonFileRead.readAll());
        jsonFileRead.close();

        if (!jsonString.contains(jsonSource.arg(imageInfo().name(), Qt::CaseInsensitive)))
        {
            setErrorDescription(i18nc("@info", "Apply Metadata: File name not exist in JSON file."));
            return false;
        }

        jsonString.replace(jsonSource.arg(imageInfo().name()),
                           jsonSource.arg(outputUrl().toLocalFile(), Qt::CaseInsensitive));

        QFile jsonFileWrite(tempFile.fileName());

        if (!jsonFileWrite.open(QIODevice::WriteOnly))
        {
            return false;
        }

        jsonFileWrite.write(jsonString.toUtf8());
        jsonFileWrite.close();

        ret = parser->applyMetadataFile(outputUrl().toLocalFile(), tempFile.fileName());
    }
    else
    {
        ret = parser->applyMetadataFile(outputUrl().toLocalFile(), metaInfo.filePath());
    }

    if (!ret)
    {
        setErrorDescription(i18nc("@info", "Apply Metadata: ExifTool reported an error."));
        return false;
    }

    return true;
}

} // namespace DigikamBqmApplyMetadataPlugin

#include "moc_applymetadata.cpp"
