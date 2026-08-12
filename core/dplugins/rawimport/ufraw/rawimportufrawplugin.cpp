/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-18
 * Description : UFRaw raw import plugin.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rawimportufrawplugin.h"

// Qt includes

#include <QDir>
#include <QPointer>
#include <QFileInfo>
#include <QSettings>
#include <QByteArray>
#include <QMessageBox>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated
#include "dimg.h"
#include "filteraction.h"
#include "dfileoperations.h"
#include "filereadwritelock.h"
#include "loadingdescription.h"

namespace DigikamRawImportUFRawPlugin
{

class Q_DECL_HIDDEN UFRawRawImportPlugin::Private
{
public:

    Private() = default;

public:

    QProcess*          ufraw    = nullptr;
    DImg               decoded;
    LoadingDescription props;
    QFileInfo          fileInfo;
    QString            tempName;
};

UFRawRawImportPlugin::UFRawRawImportPlugin(QObject* const parent)
    : DPluginRawImport(parent),
      d               (new Private)
{
}

UFRawRawImportPlugin::~UFRawRawImportPlugin()
{
    delete d;
}

QString UFRawRawImportPlugin::name() const
{
    return QString::fromUtf8("Raw Import using UFRaw");
}

QString UFRawRawImportPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon UFRawRawImportPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-adobe-dng"));
}

QString UFRawRawImportPlugin::description() const
{
    return QString::fromUtf8("A RAW import plugin based on UFRaw");
}

QString UFRawRawImportPlugin::details() const
{
    return QString::fromUtf8("<p>This RAW Import plugin use UFRaw tool to pre-process file in Image Editor.</p>"
                             "<p>See UFRaw web site for details: "
                             "<a href='http://ufraw.sourceforge.net/'>http://ufraw.sourceforge.net/</a></p>");  // krazy:exclude=insecurenet
}

QString UFRawRawImportPlugin::handbookSection() const
{
    return QLatin1String("setup_application");
}

QString UFRawRawImportPlugin::handbookChapter() const
{
    return QLatin1String("editor_settings");
}

QString UFRawRawImportPlugin::handbookReference() const
{
    return QLatin1String("setup-raw");
}

QList<DPluginAuthor> UFRawRawImportPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2025"))
            ;
}

void UFRawRawImportPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QString UFRawRawImportPlugin::getRawProgram() const
{
    return DFileOperations::findExecutable(QLatin1String("ufraw"));
}

bool UFRawRawImportPlugin::run(const QString& filePath, const DRawDecoding& /*def*/)
{
    d->fileInfo = QFileInfo(filePath);
    d->props    = LoadingDescription(d->fileInfo.filePath(), LoadingDescription::ConvertForEditor);
    d->decoded  = DImg();

    SafeTemporaryFile* const temp = new SafeTemporaryFile(QDir::tempPath() +
                                                          QLatin1String("/UFRaw-XXXXXX"));
    temp->setAutoRemove(false);
    temp->open();
    d->tempName = temp->safeFilePath();
    delete temp;

    d->ufraw    = new QProcess(this);
    d->ufraw->setProcessChannelMode(QProcess::MergedChannels);
    d->ufraw->setWorkingDirectory(d->fileInfo.path());
    d->ufraw->setProcessEnvironment(adjustedEnvironmentForAppImage());

    connect(d->ufraw, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(slotErrorOccurred(QProcess::ProcessError)));

    connect(d->ufraw, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));

    connect(d->ufraw, SIGNAL(readyRead()),
            this, SLOT(slotProcessReadyRead()));

    // --------

    d->fileInfo = QFileInfo(filePath);

    d->ufraw->setProgram(getRawProgram());
    d->ufraw->setArguments(QStringList() << QLatin1String("--out-depth=16")   // 16 bits per color per pixels
                                         << QLatin1String("--out-type=png")   // PNG output (TIFF output generate multi-layers file)
                                         << QLatin1String("--overwrite")      // Overwrite target temporary file
                                         << QString::fromUtf8("--output=%1")
                                            .arg(d->tempName)                 // Output file
                                         << filePath);                        // Input file

    qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "UFRaw arguments:" << d->ufraw->arguments();

    d->ufraw->start();

    return d->ufraw->waitForStarted(10000);
}

void UFRawRawImportPlugin::slotErrorOccurred(QProcess::ProcessError error)
{
    switch (error)
    {
        case QProcess::FailedToStart:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "UFRaw :: Process has failed to start";
            break;
        }

        case QProcess::Crashed:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "UFRaw :: Process has crashed";
            break;
        }

        case QProcess::Timedout:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "UFRaw :: Process time-out";
            break;
        }

        case QProcess::WriteError:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "UFRaw :: Process write error";
            break;
        }

        case QProcess::ReadError:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "UFRaw :: Process read error";
            break;
        }

        default:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "UFRaw :: Process error unknown";
            break;
        }
    }
}

void UFRawRawImportPlugin::slotProcessFinished(int code, QProcess::ExitStatus status)
{
    qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "UFRaw :: return code:"
                                           << code << ":: Exit status:" << status;

    d->decoded = DImg(d->tempName);
    d->decoded.setAttribute(QLatin1String("isReadOnly"), true);

    if (d->decoded.isNull())
    {
        QString message = i18n("Error to import RAW image with UFRaw\n"
                               "Close this dialog to load RAW image with native import tool");
        QMessageBox::information(nullptr, qApp->applicationName(), message);

        qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "Decoded image is null! Load with Native tool...";
        qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << d->props.filePath;

        Q_EMIT signalLoadRaw(d->props);
    }
    else
    {
        qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "Decoded image is not null...";
        qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << d->props.filePath;
        d->props = LoadingDescription(d->tempName, LoadingDescription::ConvertForEditor);

        FilterAction action(QLatin1String("ufraw:RawConverter"), 1, FilterAction::DocumentedHistory);
        action.setDisplayableName(QString::fromUtf8(I18N_NOOP("UFRaw Raw Conversion")));
        d->decoded.addFilterAction(action);

        Q_EMIT signalDecodedImage(d->props, d->decoded);
    }

    delete d->ufraw;
    d->ufraw = nullptr;

    QFile::remove(d->tempName);
}

void UFRawRawImportPlugin::slotProcessReadyRead()
{
    QByteArray data   = d->ufraw->readAllStandardError();
    QStringList lines = QString::fromUtf8(data).split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& one : std::as_const(lines))
    {
        qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "UFRaw ::" << one;
    }
}

} // namespace DigikamRawImportUFRawPlugin

#include "moc_rawimportufrawplugin.cpp"
