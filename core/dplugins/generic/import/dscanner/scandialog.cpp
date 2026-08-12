/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-09
 * Description : scanner dialog
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "scandialog.h"

// Qt includes

#include <QVBoxLayout>
#include <QDateTime>
#include <QPushButton>
#include <QPointer>
#include <QDir>
#include <QUrl>
#include <QMenu>
#include <QApplication>
#include <QMessageBox>
#include <QImageWriter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "saveimgthread.h"
#include "statusprogressbar.h"
#include "dxmlguiwindow.h"
#include "dfiledialog.h"

namespace DigikamGenericDScannerPlugin
{

class Q_DECL_HIDDEN ScanDialog::Private
{
public:

    Private() = default;

public:

    QString            targetDir;
    StatusProgressBar* progress     = nullptr;
    KSaneWidget*       saneWidget   = nullptr;
};

ScanDialog::ScanDialog(KSaneWidget* const saneWdg, QWidget* const parent)
    : DPluginDialog(parent, QLatin1String("Scan Tool Dialog")),
      d            (new Private)
{
    setWindowTitle(i18nc("@title:window", "Scan Image"));
    setModal(false);

    m_buttons->addButton(QDialogButtonBox::Close);
    m_buttons->button(QDialogButtonBox::Close)->setAutoDefault(false);

    d->saneWidget          = saneWdg;
    d->progress            = new StatusProgressBar(this);
    d->progress->setProgressBarMode(StatusProgressBar::ProgressBarMode);
    d->progress->setProgressTotalSteps(100);
    d->progress->setNotify(true);
    d->progress->setNotificationTitle(i18n("Scan Images"), QIcon::fromTheme(QLatin1String("scanner")));

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->saneWidget, 10);
    vbx->addWidget(d->progress);
    vbx->addWidget(m_buttons);
    setLayout(vbx);

    // ------------------------------------------------------------------------

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    connect(d->saneWidget, &KSaneWidget::scannedImageReady,
            this, &ScanDialog::slotSaveImage);

#elif KSANE_VERSION < QT_VERSION_CHECK(21,8,0)

    connect(d->saneWidget, &KSaneWidget::imageReady,
            this, &ScanDialog::slotSaveImage);

#else

    connect(d->saneWidget, &KSaneWidget::scannedImageReady,
            this, &ScanDialog::slotSaveImage);

#endif

    connect(this, &QDialog::finished,
            this, &ScanDialog::slotDialogFinished);

    connect(m_buttons->button(QDialogButtonBox::Close), &QPushButton::clicked,
            this, &ScanDialog::close);
}

ScanDialog::~ScanDialog()
{
    delete d;
}

void ScanDialog::setTargetDir(const QString& targetDir)
{
    d->targetDir = targetDir;
}

void ScanDialog::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotDialogFinished();
    e->accept();
}

void ScanDialog::slotDialogFinished()
{
    d->saneWidget->closeDevice();
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

void ScanDialog::slotSaveImage(const QImage& image_data)

#elif KSANE_VERSION < QT_VERSION_CHECK(21,8,0)

// cppcheck-suppress constParameterCallback
void ScanDialog::slotSaveImage(QByteArray& ksane_data, int width, int height, int bytes_per_line, int ksaneformat)

#else

void ScanDialog::slotSaveImage(const QImage& image_data)

#endif

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

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "slotSaveImage: Offered mimetypes: " << writableMimetypes;

    QLatin1String defaultMimeType("image/png");
    QLatin1String defaultFileName("image.png");

    QPointer<DFileDialog> imageFileSaveDialog = new DFileDialog(nullptr, i18nc("@title:window",
                                                                "New Image File Name"), d->targetDir);
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

            qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "target image file format " << format << " is unsupported!";

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

        qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "target URL is not valid !";

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
    setEnabled(false);

    // Perform saving ---------------------------------------------------------------

    SaveImgThread* const thread = new SaveImgThread(this);

    connect(thread, &SaveImgThread::signalProgress,
            this, &ScanDialog::slotThreadProgress);

    connect(thread, &SaveImgThread::signalComplete,
            this, &ScanDialog::slotThreadDone);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    thread->setImageData(image_data);
    thread->setScannerModel(d->saneWidget->deviceVendor(), d->saneWidget->deviceModel());

#elif KSANE_VERSION < QT_VERSION_CHECK(21,8,0)

    thread->setImageData(ksane_data, width, height, bytes_per_line, ksaneformat);
    thread->setScannerModel(d->saneWidget->make(), d->saneWidget->model());

#else

    thread->setImageData(image_data);
    thread->setScannerModel(d->saneWidget->deviceVendor(), d->saneWidget->deviceModel());

#endif

    thread->setTargetFile(newURL, format);
    thread->start();
}

void ScanDialog::slotThreadProgress(const QUrl& url, int percent)
{
    d->progress->setProgressText(i18n("Saving file %1 -", url.fileName()));
    d->progress->setProgressValue(percent);
}

void ScanDialog::slotThreadDone(const QUrl& url, bool success)
{
    if (!success)
    {
        QMessageBox::critical(nullptr, i18nc("@title:window", "File Not Saved"),
                              i18n("Cannot save \"%1\" file", url.fileName()));
    }

    d->progress->setProgressText(QString());
    QApplication::restoreOverrideCursor();
    setEnabled(true);

    if (success)
    {
        Q_EMIT signalImportedImage(url);
    }
}

} // namespace DigikamGenericDScannerPlugin

#include "moc_scandialog.cpp"
