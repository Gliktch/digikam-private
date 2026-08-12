/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-02
 * Description : an option to provide directory information to the parser
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "directorynameoption.h"

// Qt includes

#include <QFileInfo>
#include <QString>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

DirectoryNameOption::DirectoryNameOption()
    : Option(i18nc("renaming option for directory related information",
                   "Directory"),
             i18n("Add the directory name"),
             QLatin1String("folder"))
{
    addToken(QLatin1String("[dir]"),  i18n("Directory name"));
    addToken(QLatin1String("[dir.]"), i18n("Name of the parent directory, additional '.' characters move up "
                                           "in the directory hierarchy"));

    QRegularExpression reg(QLatin1String("\\[dir(\\.*)\\]"));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
}

QString DirectoryNameOption::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match)
{
    QString result;
    QFileInfo fi(settings.fileUrl.toLocalFile());

    if (!fi.exists())
    {
        return result;
    }

    QStringList folders = fi.absolutePath().split(QLatin1Char('/'), Qt::SkipEmptyParts);
    int folderCount     = folders.count();
    int matchedLength   = match.captured(1).length();

    if      (matchedLength == 0)
    {
        result = folders.last();
    }
    else if (matchedLength > (folderCount - 1))
    {
        result.clear();
    }
    else
    {
        result = folders.at(folderCount - matchedLength - 1);
    }

    return result;
}

} // namespace Digikam

#include "moc_directorynameoption.cpp"
