/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : Preview OSD settings container.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "previewosdsettings.h"

// Qt includes

#include <QApplication>
#include <QPalette>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_globals.h"

namespace Digikam
{

void PreviewOsdSettings::readFromConfig(const QString& configGroupName)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName);

    printName                 = group.readEntry(configPreviewOsdPrintNameEntry,            true);
    printDate                 = group.readEntry(configPreviewOsdPrintDateEntry,            false);
    printApertureFocal        = group.readEntry(configPreviewOsdPrintApertureFocalEntry,   false);
    printExpoSensitivity      = group.readEntry(configPreviewOsdPrintExpoSensitivityEntry, false);
    printMakeModel            = group.readEntry(configPreviewOsdPrintMakeModelEntry,       false);
    printLensModel            = group.readEntry(configPreviewOsdPrintLensModelEntry,       false);
    printCaption              = group.readEntry(configPreviewOsdPrintCommentEntry,         false);
    printTitle                = group.readEntry(configPreviewOsdPrintTitleEntry,           false);
    printCapIfNoTitle         = group.readEntry(configPreviewOsdPrintCapIfNoTitleEntry,    false);
    printKeywords             = group.readEntry(configPreviewOsdPrintTagsEntry,            false);
    captionFont               = group.readEntry(configPreviewOsdCaptionFontEntry,
                                                QFontDatabase::systemFont(QFontDatabase::GeneralFont));
}

void PreviewOsdSettings::writeToConfig(const QString& configGroupName)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName);

    group.writeEntry(configPreviewOsdPrintNameEntry,            printName);
    group.writeEntry(configPreviewOsdPrintDateEntry,            printDate);
    group.writeEntry(configPreviewOsdPrintApertureFocalEntry,   printApertureFocal);
    group.writeEntry(configPreviewOsdPrintExpoSensitivityEntry, printExpoSensitivity);
    group.writeEntry(configPreviewOsdPrintMakeModelEntry,       printMakeModel);
    group.writeEntry(configPreviewOsdPrintLensModelEntry,       printLensModel);
    group.writeEntry(configPreviewOsdPrintCommentEntry,         printCaption);
    group.writeEntry(configPreviewOsdPrintTitleEntry,           printTitle);
    group.writeEntry(configPreviewOsdPrintCapIfNoTitleEntry,    printCapIfNoTitle);
    group.writeEntry(configPreviewOsdPrintTagsEntry,            printKeywords);
    group.writeEntry(configPreviewOsdCaptionFontEntry,          captionFont);
    group.sync();
}

void PreviewOsdSettings::printInfoText(QPainter& p, int& offset, const QString& str, const QColor& pcol)
{
    if (!str.isEmpty())
    {
        offset           += QFontMetrics(p.font()).lineSpacing();
        const int spacing = layoutSpacing();
        p.setPen(Qt::black);

        for (int x = -1 ; x <= 1 ; ++x)
        {
            for (int y = offset + 1 ; y >= offset - 1 ; --y)
            {
                p.drawText(x + spacing, p.window().height() - y, str);
            }
        }

        p.setPen(pcol);
        p.drawText(spacing, p.window().height() - offset, str);
    }
}

void PreviewOsdSettings::printComments(QPainter& p, int& offset, const QString& comments)
{
    QStringList commentsByLines;

    uint commentsIndex = 0;     // Comments QString index

    while (commentsIndex < (uint)comments.length())
    {
        QString newLine;
        bool breakLine = false; // End Of Line found
        uint currIndex;         // Comments QString current index

        // Check minimal lines dimension

        uint commentsLinesLengthLocal = 80;

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

        commentsIndex   = currIndex; // The line is ended
        QString tmpLine = newLine;

        if (commentsIndex != (uint)comments.length())
        {
            while (!newLine.endsWith(QLatin1Char(' ')))
            {
                newLine.truncate(newLine.length() - 1);

                if (commentsIndex > 0)
                {
                    --commentsIndex;
                }
                else
                {
                    // No space found, cut hard.

                    commentsIndex = currIndex;
                    newLine       = tmpLine;
                    break;
                }
            }
        }

        commentsByLines.prepend(newLine.trimmed());
    }

    for (int i = 0 ; i < (int)commentsByLines.count() ; ++i)
    {
        printInfoText(p, offset, commentsByLines.at(i));
    }
}

void PreviewOsdSettings::printTags(QPainter& p, int& offset, QStringList& tags)
{
    tags.removeDuplicates();
    tags.sort();

    QString str = tags.join(QLatin1String(", "));

    if (!str.isEmpty())
    {
        printInfoText(p, offset, str, qApp->palette().color(QPalette::Link).name());
    }
}

} // namespace Digikam
