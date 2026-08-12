/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-08
 * Description : global macros, variables and flags
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikam_globals_p.h"

namespace Digikam
{

QString DATETIMEFORMATSTRING = QLocale().dateTimeFormat(QLocale::ShortFormat);

QShortcut* defineShortcut(QWidget* const w, const QKeySequence& key, const QObject* receiver, const char* slot)
{
    QShortcut* const s = new QShortcut(w);
    s->setKey(key);
    s->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(s, SIGNAL(activated()), receiver, slot);

    return s;
}

QStringList supportedImageMimeTypes(QIODevice::OpenModeFlag mode, QString& allTypes)
{
    QStringList       formats;
    QList<QByteArray> supported;

    switch (mode)
    {
        case QIODevice::ReadOnly:
        {
            supported = QImageReader::supportedImageFormats();
            break;
        }

        case QIODevice::WriteOnly:
        {
            supported = QImageWriter::supportedImageFormats();
            break;
        }

        case QIODevice::ReadWrite:
        {
            supported = QImageWriter::supportedImageFormats() + QImageReader::supportedImageFormats();
            break;
        }

        default:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unsupported mode!";
            break;
        }
    }

    for (const QByteArray& frm : std::as_const(supported))
    {
        if (
            QString::fromLatin1(frm).contains(QLatin1String("tif"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("tiff"), Qt::CaseInsensitive)
           )
        {
            continue;
        }

        if (
            QString::fromLatin1(frm).contains(QLatin1String("jpg"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("jpeg"), Qt::CaseInsensitive)
           )
        {
            continue;
        }

#ifdef HAVE_JASPER

        if (
            QString::fromLatin1(frm).contains(QLatin1String("jp2"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("j2k"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("jpx"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("jpc"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("pgx"),  Qt::CaseInsensitive)
           )
        {
            continue;
        }

#endif // HAVE_JASPER

#ifdef HAVE_X265

        if (
            QString::fromLatin1(frm).contains(QLatin1String("heic"), Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("heif"), Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("hif"),  Qt::CaseInsensitive)
           )
        {
            continue;
        }

#endif // HAVE_X265

#ifdef HAVE_IMAGE_MAGICK

        if (
            QString::fromLatin1(frm).contains(QLatin1String("fts"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("fit"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("fits"), Qt::CaseInsensitive)
           )
        {
            continue;
        }

#endif // HAVE_IMAGE_MAGICK

        formats.append(i18n("%1 Image (%2)", QString::fromLatin1(frm).toUpper(), QLatin1String("*.") + QLatin1String(frm)));
        allTypes.append(QString::fromLatin1("*.%1 ").arg(QLatin1String(frm)));
    }

    formats.append(i18n("TIFF Image (*.tiff *.tif)"));
    allTypes.append(QLatin1String("*.tiff *.tif "));
    formats.append(i18n("JPEG Image (*.jpg *.jpeg *.jpe)"));
    allTypes.append(QLatin1String("*.jpg *.jpeg *.jpe "));

#ifdef HAVE_JASPER

    formats.append(i18n("JPEG2000 Image (*.jp2 *.j2k *.jpx *.pgx)"));
    allTypes.append(QLatin1String("*.jp2 *.j2k *.jpx *.pgx "));

#endif // HAVE_JASPER

    formats << i18n("Progressive Graphics file (*.pgf)");
    allTypes.append(QLatin1String("*.pgf "));

#ifdef HAVE_X265

    formats << i18n("High Efficiency Image Coding (*.heic *.heif)");
    allTypes.append(QLatin1String("*.heic *.heif "));

#endif // HAVE_X265

#ifdef HAVE_IMAGE_MAGICK

    formats << i18n("Flexible Image Transport System (*.fts *.fit *.fits)");
    allTypes.append(QLatin1String("*.fts *.fit *.fits "));

#endif // HAVE_IMAGE_MAGICK

    if (mode != QIODevice::WriteOnly)
    {
        formats << i18n("Raw Images (%1)", DRawDecoder::rawFiles());
        allTypes.append(DRawDecoder::rawFiles());
        formats << i18n("All supported files (%1)", allTypes);
    }

    return formats;
}

bool isReadableImageFile(const QString& filePath)
{
    QFileInfo info(filePath);

    if (info.isFile() && !info.isSymLink() && !info.isDir() && !info.isRoot())
    {
        QString path    = info.absoluteFilePath();
        QMimeType mtype = QMimeDatabase().mimeTypeForFile(path);
        QString suffix  = info.suffix().toUpper();

        // Add extra check of the image extensions that are still
        // unknown in older Qt versions or have an application mime type.

        if (
            mtype.name().startsWith(QLatin1String("image/")) ||
            (suffix == QLatin1String("PGF"))                 ||
            (suffix == QLatin1String("KRA"))                 ||
            (suffix == QLatin1String("CR3"))                 ||
            (suffix == QLatin1String("HIF"))                 ||
            (suffix == QLatin1String("HEIC"))                ||
            (suffix == QLatin1String("HEIF"))                ||
            DRawDecoder::rawFiles().contains(suffix)
           )
        {
            return true;
        }
    }

    return false;
}

void showRawCameraList()
{
    RawCameraDlg* const dlg = new RawCameraDlg(qApp->activeWindow());
    dlg->show();
}

QString toolButtonStyleSheet()
{
    return QLatin1String("QToolButton { padding: 1px; background-color: "
                         "  qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                         "  stop: 0 rgba(100, 100, 100, 70%), "
                         "  stop: 1 rgba(170, 170, 170, 70%)); "
                         "border: 1px solid rgba(170, 170, 170, 10%); } "

                         "QToolButton:hover { border-color: white; } "

                         "QToolButton:pressed { background-color: "
                         "  qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                         "  stop: 0 rgba(40, 40, 40, 70%), "
                         "  stop: 1 rgba(90, 90, 90, 70%)); "
                         "border-color: white; } "

                         "QToolButton:checked { background-color: "
                         "  qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                         "  stop: 0 rgba(40, 40, 40, 70%), "
                         "  stop: 1 rgba(90, 90, 90, 70%)); } "

                         "QToolButton:disabled { background-color: "
                         "  qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                         "  stop: 0 rgba(40, 40, 40, 70%), "
                         "  stop: 1 rgba(50, 50, 50, 70%)); }");
}

int layoutSpacing()
{
    return (
            qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                 QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing))
           );
}

int layoutMargin()
{
    return (
            qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                 qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutTopMargin),
                      qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                           QApplication::style()->pixelMetric(QStyle::PM_LayoutBottomMargin)
                          )
                     )
                )
           );
}

int dialogExec(QDialog* const dlg)
{
    if (dlg)
    {
        QEventLoop loop;

        QObject::connect(dlg, &QDialog::finished,
                         &loop, &QEventLoop::quit);

        dlg->open();
        loop.exec();

        // cppcheck-suppress knownConditionTrueFalse
        if (dlg)
        {
            return dlg->result();
        }
    }

    return QDialog::Rejected;
}

QDateTime startOfDay(const QDate& date)
{
    return date.startOfDay();
}

QDateTime asDateTimeUTC(const QDateTime& dt)
{
    QDateTime dateTime(dt);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))

    dateTime.setTimeZone(QTimeZone::UTC);

#else

    dateTime.setTimeSpec(Qt::UTC);

#endif

    return dateTime;
}

QDateTime asDateTimeLocal(const QDateTime& dt)
{
    QDateTime dateTime(dt);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))

    dateTime.setTimeZone(QTimeZone::LocalTime);

#else

    dateTime.setTimeSpec(Qt::LocalTime);

#endif

    return dateTime;
}

QString asUserDateTime(const QDateTime& dt)
{
    return dt.toString(DATETIMEFORMATSTRING);
}

QString getUserDateFormatString()
{
    QString userFormat = DATETIMEFORMATSTRING;
    QString dateFormat = QLocale().dateFormat(QLocale::ShortFormat);

    int spos           = userFormat.indexOf(QLatin1Char(' '));
    int dpos           = userFormat.indexOf(QLatin1Char('d'));
    int mpos           = userFormat.indexOf(QLatin1Char('M'));
    int ypos           = userFormat.indexOf(QLatin1Char('y'));

    if ((dpos < spos) && (mpos < spos) && (ypos < spos))
    {
        userFormat = userFormat.left(spos);

        if (
            userFormat.contains(QLatin1Char('.')) ||
            userFormat.contains(QLatin1Char('-')) ||
            userFormat.contains(QLatin1Char('/'))
           )
        {
            dateFormat = userFormat;
        }
    }

    if (!dateFormat.contains(QLatin1String("yyyy")))
    {
        dateFormat.replace(QLatin1String("yy"),
                           QLatin1String("yyyy"));
    }

    return dateFormat;
}

QString getDateFormatLinkText()
{
    const QString dateFormatLink      = QString::fromUtf8("<a href='https://doc.qt.io/qt-6/qdatetime.html#toString'>%1</a>");
    const QString dateFormatLinkDescr = i18nc("@info: date format settings", "format settings");

    return dateFormatLink.arg(dateFormatLinkDescr);
}

void openOnlineDocumentation(const QString& section, const QString& chapter, const QString& reference)
{
    QUrl url;

    if (section.isEmpty())
    {
        if (QApplication::applicationName() == QLatin1String("showfoto"))
        {
            url = QUrl(QString::fromUtf8("https://docs.digikam.org/en/showfoto_editor.html"));
        }
        else
        {
            url = QUrl(QString::fromUtf8("https://docs.digikam.org/en/index.html"));
        }
    }
    else
    {
        if (chapter.isEmpty())
        {
            url = QUrl(QString::fromUtf8("https://docs.digikam.org/en/%1.html").arg(section));
        }
        else
        {
            if (reference.isEmpty())
            {
                url = QUrl(QString::fromUtf8("https://docs.digikam.org/en/%1/%2.html").arg(section).arg(chapter));
            }
            else
            {
                url = QUrl(QString::fromUtf8("https://docs.digikam.org/en/%1/%2.html#%3").arg(section).arg(chapter).arg(reference));
            }
        }
    }

    QWidget* const activeWindow  = qApp->activeWindow();
    WebBrowserDlg* const browser = new WebBrowserDlg(url, activeWindow);

    QObject::connect(activeWindow, SIGNAL(destroyed(QObject*)),
                     browser, SLOT(close()));

    browser->show();
}

void setOpenCLEnvironment(bool b)
{
    /*
     * WARNING: OpenCV can crash with face engine when OpenCL support is enabled.
     *
     * https://bugs.kde.org/show_bug.cgi?id=423632
     * https://bugs.kde.org/show_bug.cgi?id=426175
     */

    if (!b)
    {
        qputenv("OPENCV_OPENCL_RUNTIME", "disabled");
        qputenv("OPENCV_OPENCL_DEVICE",  "disabled");
    }
    else
    {
/*
        FIXME: for testing only for the moment.
        qputenv("DIGIKAM_DNN_TARGET",    "opencl");
*/
    }

}

} // namespace Digikam
