/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-18
 * Description : DarkTable raw import plugin.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Lua script inspired from Darktable Gimp plugin by Tobias Ellinghaus
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rawimportdarktableplugin.h"

// Qt includes

#include <QDir>
#include <QPointer>
#include <QFileInfo>
#include <QSettings>
#include <QByteArray>
#include <QMessageBox>
#include <QTextStream>
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

namespace DigikamRawImportDarkTablePlugin
{

class Q_DECL_HIDDEN DarkTableRawImportPlugin::Private
{
public:

    Private() = default;

public:

    QProcess*            darktable     = nullptr;
    DImg                 decoded;
    LoadingDescription   props;
    QString              tempName;
    SafeTemporaryFile*   luaTempFile   = nullptr;

    const QString        luaScriptData = QLatin1String
    (
        "\n"                                                                                                    \
        "local dt = require \"darktable\"\n"                                                                    \
        "\n"                                                                                                    \
        "local min_api_version = \"2.1.0\"\n"                                                                   \
        "if dt.configuration.api_version_string < min_api_version then\n"                                       \
        "  dt.print(\"the exit export script requires at least darktable version 1.7.0\")\n"                    \
        "  dt.print_error(\"the exit export script requires at least darktable version 1.7.0\")\n"              \
        "  return\n"                                                                                            \
        "else\n"                                                                                                \
        "  dt.print(\"closing darktable will export the image and make image editor load it\")\n"               \
        "end\n"                                                                                                 \
        "\n"                                                                                                    \
        "local export_filename = dt.preferences.read(\"export_on_exit\", \"export_filename\", \"string\")\n"    \
        "\n"                                                                                                    \
        "function exit_function()\n"                                                                            \
        "  -- safegurad against someone using this with their library containing 50k images\n"                  \
        "  if #dt.database > 1 then\n"                                                                          \
        "    dt.print_error(\"too many images, only exporting the first\")\n"                                   \
        "  -- return\n"                                                                                         \
        "  end\n"                                                                                               \
        "\n"                                                                                                    \
        "  -- change the view first to force writing of the history stack\n"                                    \
        "  dt.gui.current_view(dt.gui.views.lighttable)\n"                                                      \
        "  -- now export\n"                                                                                     \
        "  local format = dt.new_format(\"png\")\n"                                                             \
        "  format.max_width = 0\n"                                                                              \
        "  format.max_height = 0\n"                                                                             \
        "  -- lets have the export in a loop so we could easily support > 1 images\n"                           \
        "  for _, image in ipairs(dt.database) do\n"                                                            \
        "    dt.print_error(\"exporting `\"..tostring(image)..\"' to `\"..export_filename..\"'\")\n"            \
        "    format:write_image(image, export_filename)\n"                                                      \
        "    break -- only export one image. see above for the reason\n"                                        \
        "  end\n"                                                                                               \
        "end\n"                                                                                                 \
        "\n"                                                                                                    \
        "if dt.configuration.api_version_string >= \"6.2.1\" then\n"                                            \
        "dt.register_event(\"fileraw\", \"exit\", exit_function)\n"                                             \
        "else\n"                                                                                                \
        "dt.register_event(\"exit\", exit_function)\n"                                                          \
        "end\n"                                                                                                 \
    );
};

DarkTableRawImportPlugin::DarkTableRawImportPlugin(QObject* const parent)
    : DPluginRawImport(parent),
      d               (new Private)
{
    d->luaTempFile = new SafeTemporaryFile(QDir::tempPath() +
                                           QLatin1String("/LuaScript-XXXXXX"));
    d->luaTempFile->setAutoRemove(true);
    d->luaTempFile->open();

    QTextStream stream(d->luaTempFile);
    stream << d->luaScriptData;
    stream.flush();
}

DarkTableRawImportPlugin::~DarkTableRawImportPlugin()
{
    delete d->luaTempFile;
    delete d;
}

QString DarkTableRawImportPlugin::name() const
{
    return QString::fromUtf8("Raw Import using DarkTable");
}

QString DarkTableRawImportPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon DarkTableRawImportPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-adobe-dng"));
}

QString DarkTableRawImportPlugin::description() const
{
    return QString::fromUtf8("A RAW import plugin based on DarkTable");
}

QString DarkTableRawImportPlugin::details() const
{
    return QString::fromUtf8("<p>This RAW Import plugin use DarkTable tool to pre-process file in Image Editor.</p>"
                             "<p>It requires at least DarkTable version 1.7.0 to work.</p>"
                             "<p>See DarkTable web site for details: <a href='https://www.darktable.org/'>https://www.darktable.org/</a></p>");
}

QString DarkTableRawImportPlugin::handbookSection() const
{
    return QLatin1String("setup_application");
}

QString DarkTableRawImportPlugin::handbookChapter() const
{
    return QLatin1String("editor_settings");
}

QString DarkTableRawImportPlugin::handbookReference() const
{
    return QLatin1String("setup-raw");
}

QList<DPluginAuthor> DarkTableRawImportPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2025"))
            ;
}

void DarkTableRawImportPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

QString DarkTableRawImportPlugin::getRawProgram() const
{
    QStringList hints;

#ifdef Q_OS_MACOS

    hints << QLatin1String("/Applications/darktable.app/Contents/MacOS");

#endif

    return DFileOperations::findExecutable(QLatin1String("darktable"), hints);
}

bool DarkTableRawImportPlugin::run(const QString& filePath, const DRawDecoding& /*def*/)
{
    QFileInfo fileInfo(filePath);
    d->props    = LoadingDescription(fileInfo.filePath(),
                                    LoadingDescription::ConvertForEditor);
    d->decoded  = DImg();

    SafeTemporaryFile* const temp = new SafeTemporaryFile(QDir::tempPath() +
                                                          QLatin1String("/Darktable-XXXXXX"));
    temp->setAutoRemove(false);
    temp->open();
    d->tempName = temp->safeFilePath();
    delete temp;

    d->darktable = new QProcess(this);
    d->darktable->setProcessChannelMode(QProcess::MergedChannels);
    d->darktable->setWorkingDirectory(fileInfo.path());
    d->darktable->setProcessEnvironment(adjustedEnvironmentForAppImage());

    connect(d->darktable, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(slotErrorOccurred(QProcess::ProcessError)));

    connect(d->darktable, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));

    connect(d->darktable, SIGNAL(readyRead()),
            this, SLOT(slotProcessReadyRead()));

    // --------

    d->darktable->setProgram(getRawProgram());
    d->darktable->setArguments(QStringList() << QLatin1String("--library")
                                             << QLatin1String(":memory:")                                  // Run DarkTable to process only one file
                                             << QLatin1String("--luacmd")
                                             << QString::fromUtf8("dofile('%1')")
                                                .arg(d->luaTempFile->safeFilePath())                       // LUA script to run in DarkTable
                                             << QLatin1String("--conf")
                                             << QLatin1String("plugins/lighttable/export/icctype=-1")      // Output color-space
                                             << QLatin1String("--conf")
                                             << QString::fromUtf8("lua/export_on_exit/export_filename=%1")
                                                .arg(d->tempName)                                          // Output file
                                             << filePath);                                                 // Input file

    qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "DarkTable arguments:" << d->darktable->arguments();

    d->darktable->start();

    return d->darktable->waitForStarted(10000);
}

void DarkTableRawImportPlugin::slotErrorOccurred(QProcess::ProcessError error)
{
    switch (error)
    {
        case QProcess::FailedToStart:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "DarkTable :: Process has failed to start";
            break;
        }

        case QProcess::Crashed:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "DarkTable :: Process has crashed";
            break;
        }

        case QProcess::Timedout:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "DarkTable :: Process time-out";
            break;
        }

        case QProcess::WriteError:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "DarkTable :: Process write error";
            break;
        }

        case QProcess::ReadError:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "DarkTable :: Process read error";
            break;
        }

        default:
        {
            qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "DarkTable :: Process error unknown";
            break;
        }
    }
}

void DarkTableRawImportPlugin::slotProcessFinished(int code, QProcess::ExitStatus status)
{
    qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "DarkTable :: return code:"
                                           << code << ":: Exit status:" << status;

    d->decoded = DImg(d->tempName);
    d->decoded.setAttribute(QLatin1String("isReadOnly"), true);

    if (d->decoded.isNull())
    {
        QString message = i18n("Error to import RAW image with DarkTable\n"
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

        FilterAction action(QLatin1String("darktable:RawConverter"), 1, FilterAction::DocumentedHistory);
        action.setDisplayableName(QString::fromUtf8(I18N_NOOP("DarkTable Raw Conversion")));
        d->decoded.addFilterAction(action);

        Q_EMIT signalDecodedImage(d->props, d->decoded);
    }

    delete d->darktable;
    d->darktable = nullptr;

    QFile::remove(d->tempName);
}

void DarkTableRawImportPlugin::slotProcessReadyRead()
{
    QByteArray data   = d->darktable->readAllStandardError();
    QStringList lines = QString::fromUtf8(data).split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& one : std::as_const(lines))
    {
        qCDebug(DIGIKAM_DPLUGIN_RAWIMPORT_LOG) << "DarkTable ::" << one;
    }
}

} // namespace DigikamRawImportDarkTablePlugin

#include "moc_rawimportdarktableplugin.cpp"
