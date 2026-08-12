/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-30
 * Description : digiKam components info dialog.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "componentsinfodlg.h"

// Qt includes

#include <QApplication>
#include <QString>
#include <QMap>
#include <QSqlDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "applicationsettings.h"
#include "digikam_config.h"
#include "libsinfodlg.h"
#include "dbstatdlg.h"
#include "digikam_opencv.h"

// LibGphoto2 includes

#ifdef HAVE_GPHOTO2

extern "C"
{
#   include <gphoto2-version.h>
}

#endif

// Mediaplayer includes

#ifdef HAVE_MEDIAPLAYER
#   include "ffmpegconfighelper.h"

extern "C"
{
#   include <libavformat/version.h>
#   include <libavutil/version.h>
#   include <libavcodec/version.h>

/**
 * Function not exported in libavutil returning the FFMPEG version.
 */
const char* av_version_info(void);
}

#endif

namespace Digikam
{

class Q_DECL_HIDDEN ComponentsInfoDlg : public LibsInfoDlg
{
    Q_OBJECT

public:

    explicit ComponentsInfoDlg(QWidget* const parent)
        : LibsInfoDlg(parent)
    {
        // Set digiKam specific components info list.

#ifdef HAVE_GPHOTO2

        const char** gpLibrary = gp_library_version(GP_VERSION_SHORT);

        QString gpVersion      = gpLibrary ? QLatin1String(gpLibrary[0])
                                           : i18nc("@item: component info", "Unknown");

        new QTreeWidgetItem(m_libraries, QStringList() <<
                            i18nc("@item: component info", "LibGphoto2") <<             gpVersion);

#else

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "LibGphoto2 support") <<     i18nc("@item: component info", "No"));

#endif

#ifdef HAVE_KFILEMETADATA

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "Baloo support") <<          i18nc("@item: component info", "Yes"));

#else

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "Baloo support") <<          i18nc("@item: component info", "No"));

#endif

#ifdef HAVE_AKONADICONTACT

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "AddressBook support") <<    i18nc("@item: component info", "Yes"));

#else

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "AddressBook support") <<    i18nc("@item: component info", "No"));

#endif

#ifdef HAVE_MEDIAPLAYER

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "Media player support") <<   i18nc("@item: component info", "Yes"));

        new QTreeWidgetItem(m_libraries, QStringList() <<
                            i18nc("@item: component info", "LibAVFormat") <<            QString::fromLatin1("%1.%2.%3").arg(LIBAVFORMAT_VERSION_MAJOR).arg(LIBAVFORMAT_VERSION_MINOR).arg(LIBAVFORMAT_VERSION_MICRO));

        new QTreeWidgetItem(m_libraries, QStringList() <<
                            i18nc("@item: component info", "LibAVCodec") <<             QString::fromLatin1("%1.%2.%3").arg(LIBAVCODEC_VERSION_MAJOR).arg(LIBAVCODEC_VERSION_MINOR).arg(LIBAVCODEC_VERSION_MICRO));

        new QTreeWidgetItem(m_libraries, QStringList() <<
                            i18nc("@item: component info", "LibAVUtil") <<              QString::fromLatin1("%1.%2.%3").arg(LIBAVUTIL_VERSION_MAJOR).arg(LIBAVUTIL_VERSION_MINOR).arg(LIBAVUTIL_VERSION_MICRO));

        new QTreeWidgetItem(m_libraries, QStringList() <<
                            i18nc("@item: component info", "FFMpeg") <<                 QString::fromLatin1(av_version_info()));


#else

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "Media player support") <<   i18nc("@item: component info", "No"));

#endif

#ifdef HAVE_DBUS

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "DBus support") <<           i18nc("@item: component info", "Yes"));

#else

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "DBus support") <<           i18nc("@item: component info", "No"));

#endif

#ifdef HAVE_PANORAMA

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "Panorama support") <<       i18nc("@item: component info", "Yes"));

#else

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "Panorama support") <<       i18nc("@item: component info", "No"));

#endif

#ifdef HAVE_HTMLGALLERY

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "HTML Gallery support") <<   i18nc("@item: component info", "Yes"));

#else

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "HTML Gallery support") <<   i18nc("@item: component info", "No"));

#endif

#ifdef HAVE_KCALENDAR

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "Calendar support") <<       i18nc("@item: component info", "Yes"));

#else

        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "Calendar support") <<       i18nc("@item: component info", "No"));

#endif

        new QTreeWidgetItem(m_libraries, QStringList() <<
                            i18nc("@item: component info", "LibOpenCV") <<              QLatin1String(CV_VERSION));

        // Database Backend information
        // TODO: add sqlite versions here? Could be useful for debugging sqlite problems..
        // Use this sqlite request; https://www.sqlite.org/lang_corefunc.html#sqlite_version

        new QTreeWidgetItem(m_libraries, QStringList() <<
                            i18nc("@item: component info", "Qt Sql drivers") <<         QSqlDatabase::drivers().join(QLatin1Char(' ')));


        QString dbBe = ApplicationSettings::instance()->getDbEngineParameters().databaseType;
        new QTreeWidgetItem(m_features, QStringList() <<
                            i18nc("@item: component info", "Database backend") <<       dbBe);

        if (dbBe != QLatin1String("QSQLITE"))
        {
            QString internal = ApplicationSettings::instance()->getDbEngineParameters().internalServer ? i18nc("@item: component info", "Yes")
                                                                                                       : i18nc("@item: component info", "No");
            new QTreeWidgetItem(m_features, QStringList() <<
                                i18nc("@item: component info", "Database internal server") << internal);
        }

#ifdef HAVE_MEDIAPLAYER

        QTreeWidgetItem* const ffmpegEntry = new QTreeWidgetItem(listView(), QStringList() << i18nc("@item: component info", "FFmpeg Configuration"));
        listView()->addTopLevelItem(ffmpegEntry);

        // --- FFMPEG Video codecs ---

        FFMpegProperties propsVid = FFMpegConfigHelper::getVideoCodecsProperties();

        QTreeWidgetItem* const vidDec = new QTreeWidgetItem(ffmpegEntry, QStringList() << i18nc("@item: component info", "Video Decoders"));

        for (FFMpegProperties::const_iterator it = propsVid.constBegin() ; it != propsVid.constEnd() ; ++it)
        {
            QStringList vals = it.value();

            if ((vals.size() > 1) && (vals[1] == QLatin1String("R")))
            {
                new QTreeWidgetItem(vidDec, QStringList() << it.key() << vals[0]);
            }
        }

        if (vidDec->childCount() == 0)
        {
            new QTreeWidgetItem(vidDec, QStringList() << i18n("no entry"));
        }

        QTreeWidgetItem* const vidEnc = new QTreeWidgetItem(ffmpegEntry, QStringList() << i18nc("@item: component info", "Video Encoders"));

        for (FFMpegProperties::const_iterator it = propsVid.constBegin() ; it != propsVid.constEnd() ; ++it)
        {
            QStringList vals = it.value();

            if ((vals.size() > 2) && (vals[2] == QLatin1String("W")))
            {
                new QTreeWidgetItem(vidEnc, QStringList() << it.key() << vals[0]);
            }
        }

        if (vidEnc->childCount() == 0)
        {
            new QTreeWidgetItem(vidEnc, QStringList() << i18n("no entry"));
        }

        // --- FFMPEG Audio codecs ---

        FFMpegProperties propsAud     = FFMpegConfigHelper::getAudioCodecsProperties();
        QTreeWidgetItem* const audDec = new QTreeWidgetItem(ffmpegEntry, QStringList() << i18nc("@item: component info", "Audio Decoders"));

        for (FFMpegProperties::const_iterator it = propsAud.constBegin() ; it != propsAud.constEnd() ; ++it)
        {
            QStringList vals = it.value();

            if ((vals.size() > 1) && (vals[1] == QLatin1String("R")))
            {
                    new QTreeWidgetItem(audDec, QStringList() << it.key() << vals[0]);
            }
        }

        if (audDec->childCount() == 0)
        {
            new QTreeWidgetItem(audDec, QStringList() << i18n("no entry"));
        }

        QTreeWidgetItem* const audEnc = new QTreeWidgetItem(ffmpegEntry, QStringList() << i18nc("@item: component info", "Audio Encoders"));

        for (FFMpegProperties::const_iterator it = propsAud.constBegin() ; it != propsAud.constEnd() ; ++it)
        {
            QStringList vals = it.value();

            if ((vals.size() > 2) && (vals[2] == QLatin1String("W")))
            {
                new QTreeWidgetItem(audEnc, QStringList() << it.key() << vals[0]);
            }
        }

        if (audEnc->childCount() == 0)
        {
             new QTreeWidgetItem(audEnc, QStringList() << i18n("no entry"));
        }

        // --- FFMPEG supported extensions ---

        FFMpegProperties propsExt       = FFMpegConfigHelper::getExtensionsProperties();
        QTreeWidgetItem* const extEntry = new QTreeWidgetItem(ffmpegEntry, QStringList() << i18nc("@item: component info", "Supported Extensions"));

        for (FFMpegProperties::const_iterator it = propsExt.constBegin() ; it != propsExt.constEnd() ; ++it)
        {
            if (!it.value().isEmpty())
            {
                new QTreeWidgetItem(extEntry, QStringList() << it.key() << it.value()[0]);
            }
        }

        if (extEntry->childCount() == 0)
        {
             new QTreeWidgetItem(extEntry, QStringList() << i18n("no entry"));
        }

#endif

    }

    ~ComponentsInfoDlg() = default;
};

// --------------------------------------------------------

void showDigikamComponentsInfo()
{
    ComponentsInfoDlg* const dlg = new ComponentsInfoDlg(qApp->activeWindow());
    dlg->show();
}

void showDigikamDatabaseStat()
{
    DBStatDlg* const dlg = new DBStatDlg(qApp->activeWindow());
    dlg->show();
}

} // namespace Digikam

#include "componentsinfodlg.moc"
