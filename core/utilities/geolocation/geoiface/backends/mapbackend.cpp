/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : Base-class for backends for geolocation interface
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mapbackend.h"

// Qt includes

#include <QMessageBox>
#include <QImageWriter>
#include <QAction>
#include <QMenu>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dfiledialog.h"
#include "geolocationsettings.h"
#include "digikam_debug.h"

namespace Digikam
{

MapBackend::MapBackend(const QExplicitlySharedDataPointer<GeoIfaceSharedData>& sharedData,
                       QObject* const parent)
    : QObject(parent),
      s      (sharedData)
{
}

void MapBackend::slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap)
{
    Q_UNUSED(index)
    Q_UNUSED(pixmap)
}

void MapBackend::slotTrackManagerChanged()
{
}

void MapBackend::addCommonOptions(QMenu* const configurationMenu)
{
    QAction* const settings = new QAction(i18n("Settings..."), configurationMenu);
    configurationMenu->addAction(settings);

    connect(settings, &QAction::triggered,
            this, []()
        {
            GeolocationSettings::instance()->openSetupGeolocation(SetupGeolocation::GoogleMaps);
        }
    );

    QAction* const exportAs = new QAction(i18n("Export as..."), configurationMenu);
    configurationMenu->addAction(exportAs);

    connect(exportAs, SIGNAL(triggered()),
            this, SLOT(slotExportScreenshot()));
}

void MapBackend::slotExportScreenshot()
{
    QStringList writableMimetypes;
    QList<QByteArray> supported = QImageWriter::supportedMimeTypes();

    for (const QByteArray& mimeType : std::as_const(supported))
    {
        writableMimetypes.append(QLatin1String(mimeType));
    }

    // Put first class citizens at first place

    writableMimetypes.removeAll(QLatin1String("image/jpeg"));
    writableMimetypes.removeAll(QLatin1String("image/tiff"));
    writableMimetypes.removeAll(QLatin1String("image/png"));
    writableMimetypes.insert(0, QLatin1String("image/png"));
    writableMimetypes.insert(1, QLatin1String("image/jpeg"));
    writableMimetypes.insert(2, QLatin1String("image/tiff"));

    qCDebug(DIGIKAM_GEOIFACE_LOG) << "slotSaveImage: Offered mimetypes: " << writableMimetypes;

    QLatin1String defaultMimeType("image/png");
    QLatin1String defaultFileName("image.png");

    QPointer<DFileDialog> imageFileSaveDialog = new DFileDialog(nullptr,
                                                                i18nc("@title:window", "New Image File Name"));
    imageFileSaveDialog->setAcceptMode(QFileDialog::AcceptSave);
    imageFileSaveDialog->setMimeTypeFilters(writableMimetypes);
    imageFileSaveDialog->selectMimeTypeFilter(defaultMimeType);
    imageFileSaveDialog->selectFile(defaultFileName);

    // Start dialog and check if canceled.

    imageFileSaveDialog->exec();

    if (!imageFileSaveDialog->hasAcceptedUrls())
    {
        delete imageFileSaveDialog;

        return;
    }

    QUrl newURL                  = imageFileSaveDialog->selectedUrls().constFirst();
    QFileInfo fi(newURL.toLocalFile());

    // Parse name filter and extract file extension

    QString selectedFilterString = imageFileSaveDialog->selectedNameFilter();
    QLatin1String triggerString("*.");
    int triggerPos               = selectedFilterString.lastIndexOf(triggerString);
    QString format;

    if (triggerPos != -1)
    {
        format = selectedFilterString.mid(triggerPos + triggerString.size());
        format = format.left(format.size() - 1);
        format = format.toUpper();
    }

    // If name filter was selected, we guess image type using file extension.

    if (format.isEmpty())
    {
        format = fi.suffix().toUpper();

        QList<QByteArray> imgExtList = QImageWriter::supportedImageFormats();
        imgExtList << "TIF";
        imgExtList << "TIFF";
        imgExtList << "JPG";
        imgExtList << "JPE";

        if (!imgExtList.contains(format.toLatin1()) && !imgExtList.contains(format.toLower().toLatin1()))
        {
            QMessageBox::critical(nullptr, i18nc("@title:window", "Unsupported Format"),
                                  i18n("The target image file format \"%1\" is unsupported.", format));

            qCWarning(DIGIKAM_GEOIFACE_LOG) << "target image file format " << format << " is unsupported!";

            delete imageFileSaveDialog;

            return;
        }
    }

    if (!newURL.isValid())
    {
        QMessageBox::critical(nullptr, i18nc("@title:window", "Cannot Save File"),
                              i18n("Failed to save file\n\"%1\" to\n\"%2\".",
                              newURL.fileName(),
                              QDir::toNativeSeparators(newURL.toLocalFile().section(QLatin1Char('/'), -2, -2))));

        qCWarning(DIGIKAM_GEOIFACE_LOG) << "target URL is not valid !";

        delete imageFileSaveDialog;

        return;
    }

    // Check for overwrite ----------------------------------------------------------

    if (fi.exists())
    {
        int result = QMessageBox::warning(nullptr, i18nc("@title:window", "Overwrite File?"),
                                          i18n("A file named \"%1\" already "
                                               "exists. Are you sure you want "
                                               "to overwrite it?",
                                               newURL.fileName()),
                                               QMessageBox::Yes | QMessageBox::No);

        if (result != QMessageBox::Yes)
        {
            delete imageFileSaveDialog;

            return;
        }
    }

    delete imageFileSaveDialog;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QPixmap pixmap = mapWidget()->grab();
    pixmap.save(newURL.toLocalFile(), format.toLatin1().constData());

    QApplication::restoreOverrideCursor();
}

} // namespace Digikam

#include "moc_mapbackend.cpp"
