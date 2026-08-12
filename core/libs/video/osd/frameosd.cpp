/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : frame on screen display.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "frameosd.h"

// Qt includes

#include <QString>
#include <QBuffer>
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "itempropertiestab.h"

namespace Digikam
{

FrameOsd::FrameOsd()
{
    m_descFnt.setStyleHint(QFont::Monospace);
    m_descFnt.setPixelSize(8);
    m_descFnt.setBold(true);
}

void FrameOsd::populateOSD(const QUrl& url,
                           const FrameOsdSettings& settings,
                           const DInfoInterface* const iface)
{
    // First stage is to get the information map from host application.

    DItemInfo item(iface->itemInfo(url));

    QString comment               = item.comment();
    QString title                 = item.title();
    QStringList tags              = item.keywords();
    int rating                    = item.rating();
    m_descFnt                     = settings.osdFont;

    QString str;

    // Display image File Name.

    if (settings.printName)
    {
        str.clear();
        QString name = url.fileName();

        if (!name.isEmpty())
        {
            str     += name;
            m_desc.append(QString::fromLatin1("\n%1").arg(str));
        }
    }

    // Display Creation Date.

    if (settings.printDate)
    {
        str.clear();
        QDateTime dateTime = item.dateTime();

        if (dateTime.isValid())
        {
            str = asUserDateTime(dateTime);
            m_desc.append(QString::fromLatin1("\n%1").arg(str));
        }
    }

    // Display tag names.

    if (settings.printTags)
    {
        printTags(tags);
    }

    // Display descs.

    if (settings.printTitle)
    {
        str.clear();

        if (!title.isEmpty())
        {
            str     += title;
            m_desc.append(QString::fromLatin1("\n%1").arg(str));
        }
    }

    // Display Captions if no Titles.

    if (settings.printCapIfNoTitle)
    {
        str.clear();

        if (title.isEmpty())
        {
            str     += comment;
            printComments(str);
        }
    }

    //  Display Comments.

    if (settings.printComment)
    {
        str.clear();

        if (!comment.isEmpty())
        {
            str     += comment;
            printComments(str);
        }
    }

    // Display Make and Model.

    if (settings.printMakeModel)
    {
        str.clear();

        QString make  = item.make();
        QString model = item.model();

        if (!make.isEmpty())
        {
            ItemPropertiesTab::shortenedMakeInfo(make);
            str      = make;
        }

        if (!model.isEmpty())
        {
            if (!make.isEmpty())
            {
                str += QLatin1String(" / ");
            }

            ItemPropertiesTab::shortenedModelInfo(model);
            str     += model;
        }

        if (!str.isEmpty())
        {
            m_desc.append(QString::fromLatin1("\n%1").arg(str));
        }
    }

    // Display Lens model.

    if (settings.printLensModel)
    {
        str.clear();

        QString lens = item.lens();

        if (!lens.isEmpty())
        {
            str     += lens;
            m_desc.append(QString::fromLatin1("\n%1").arg(str));
        }
    }

    // Display Exposure and Sensitivity.

    if (settings.printExpoSensitivity)
    {
        str.clear();

        QString exposureTime = item.exposureTime();
        QString sensitivity  = item.sensitivity();

        if (!exposureTime.isEmpty())
        {
            str = exposureTime;
        }

        if (!sensitivity.isEmpty())
        {
            if (!exposureTime.isEmpty())
            {
                str += QLatin1String(" / ");
            }

            str += i18n("%1 ISO", sensitivity);
        }

        if (!str.isEmpty())
        {
            m_desc.append(QString::fromLatin1("\n%1").arg(str));
        }
    }

    // Display Aperture and Focal.

    if (settings.printApertureFocal)
    {
        str.clear();

        QString aperture        = item.aperture();
        QString focalLength     = item.focalLength();
        QString focalLength35mm = item.focalLength35mm();

        if (!aperture.isEmpty())
        {
            str = aperture;
        }

        if (focalLength35mm.isEmpty())
        {
            if (!focalLength.isEmpty())
            {
                if (!aperture.isEmpty())
                {
                    str += QLatin1String(" / ");
                }

                str += focalLength;
            }
        }
        else
        {
            if (!aperture.isEmpty())
            {
                str += QLatin1String(" / ");
            }

            if (!focalLength.isEmpty())
            {
                str += QString::fromUtf8("%1 (%2)").arg(focalLength).arg(focalLength35mm);
            }
            else
            {
                str += QString::fromUtf8("%1").arg(focalLength35mm);
            }
        }

        if (!str.isEmpty())
        {
            m_desc.append(QString::fromLatin1("\n%1").arg(str));
        }
    }

    // Display rating

    if (settings.printRating)
    {
        if (rating != -1)
        {
            m_desc.append(QString::fromLatin1("\n%1/5").arg(rating));
        }
    }

    m_desc.append(QString::fromLatin1("\n"));
}

void FrameOsd::printTags(QStringList& tags)
{
    tags.removeDuplicates();
    tags.sort();

    QString str = tags.join(QLatin1String(", "));

    if (!str.isEmpty())
    {
        m_desc.append(QString::fromLatin1("%1").arg(str));
    }
}

void FrameOsd::printComments(const QString& comments)
{
    QStringList commentsByLines;

    uint commentsIndex = 0;     // Comments QString index

    while (commentsIndex < (uint)comments.length())
    {
        QString newLine;
        bool breakLine = false; // End Of Line found
        uint currIndex;         // Comments QString current index

        // Check minimal lines dimension

        uint maxStringLen = 80;

        uint commentsLinesLengthLocal = maxStringLen;

        for (currIndex = commentsIndex ;
             (currIndex < (uint)comments.length()) && !breakLine ; ++currIndex)
        {
            if ((comments.at(currIndex) == QLatin1Char('\n')) || comments.at(currIndex).isSpace())
            {
                breakLine = true;
            }
        }

        if (commentsLinesLengthLocal <= (currIndex - commentsIndex))
        {
            commentsLinesLengthLocal = (currIndex - commentsIndex);
        }

        breakLine = false;

        for (currIndex = commentsIndex ;
             (currIndex <= (commentsIndex + commentsLinesLengthLocal)) &&
             (currIndex < (uint)comments.length()) && !breakLine ;
             ++currIndex)
        {
            breakLine = (comments.at(currIndex) == QLatin1Char('\n')) ? true : false;

            if (breakLine)
            {
                newLine.append(QLatin1Char(' '));
            }
            else
            {
                newLine.append(comments.at(currIndex));
            }
        }

        commentsIndex = currIndex; // The line is ended

        if (commentsIndex != (uint)comments.length())
        {
            while (!newLine.endsWith(QLatin1Char(' ')))
            {
                newLine.truncate(newLine.length() - 1);

                if (commentsIndex > 0)
                {
                    --commentsIndex;
                }
            }
        }

        commentsByLines.prepend(newLine.trimmed());
    }

    for (int i = 0 ; i < (int)commentsByLines.count() ; ++i)
    {
        m_desc.append(QString::fromLatin1("\n%1").arg(commentsByLines.at(i)));
    }
}

void FrameOsd::insertOsdToFrame(QImage& frm,
                                const QUrl& url,
                                const FrameOsdSettings& settings,
                                const DInfoInterface* const iface)
{
    // Populate items properties based on url,
    // eg. album name, rating, tags, labels, date, etc.

    populateOSD(url, settings, iface);

    QPainter p(&frm);

    QFontMetrics descMt(m_descFnt);
    p.setFont(m_descFnt);

    QRect descRect = descMt.boundingRect(0, 0, frm.width(), frm.height(), 0, m_desc);
    QRect bgdescRect(m_descPos.x(),
                     m_descPos.y(),
                     descRect.width(),
                     descRect.height() + 3);

    p.fillRect(bgdescRect, m_descBg);

    p.setPen(QPen(Qt::white));
    p.drawText(bgdescRect, m_descAlign, m_desc);

    m_desc.clear();
}

void FrameOsd::insertMessageOsdToFrame(QImage& frm,
                                       const QSize& JPEGsize,
                                       const QString& mess)
{
    QPoint messPos          = QPoint(10, 10);
    Qt::Alignment messAlign = Qt::AlignLeft;
    QColor messBg           = Qt::darkGray;
    QFont messFnt           = QFont(QLatin1String("Monospace"));

    messFnt.setStyleHint(QFont::Monospace);
    messFnt.setBold(true);

    if ((JPEGsize.width() <= 480) && (JPEGsize.height() <= 480))
    {
        messFnt.setPixelSize(18);
    }
    else
    {
        messFnt.setPixelSize(60);
    }

    //---

    QPainter p(&frm);
    p.setFont(messFnt);

    QFontMetrics messMt(messFnt);

    QRect messRect = messMt.boundingRect(0, 0, frm.width(), frm.height(), 0, mess);

    // print message in the middle of frame

    QRect bgErreurRect(messPos.x(),
                       messPos.y(),
                       messRect.width(),
                       messRect.height() + 3);

    p.fillRect(bgErreurRect, messBg);

    p.setPen(QPen(Qt::white));
    p.drawText(bgErreurRect, messAlign, mess);
}

} // namespace Digikam
