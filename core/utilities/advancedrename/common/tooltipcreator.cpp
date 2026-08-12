/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to build the tooltip for a renameparser and its options
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tooltipcreator.h"

// Qt includes

#include <QRegularExpression>
#include <QPalette>
#include <QApplication>
#include <QIcon>
#include <QImage>
#include <QBuffer>
#include <QByteArray>


// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "modifier.h"
#include "option.h"
#include "parser.h"

namespace Digikam
{

TooltipCreator& TooltipCreator::getInstance()
{
    static TooltipCreator m_instance;
    return m_instance;
}

QString TooltipCreator::additionalInformation()
{
    QStringList infoItems;
    infoItems << i18n("To apply a modifier, append it to the option, for instance: [file]{upper}");
    infoItems << i18n("Modifiers can be applied to every renaming option.");
    infoItems << i18n("It is possible to assign multiple modifiers to an option, "
                      "they are applied in the order you assign them.");
    infoItems << i18n("Be sure to use the quick access buttons: They might provide "
                      "additional information about renaming and modifier options.");
    infoItems << i18n("The file list can be sorted, just right-click on it to see the sort criteria (album UI only).");

    QString information;
    information += QString::fromUtf8("<div style='margin-top:20px;'");

    information += tableStart(90);
    information += QString::fromUtf8("<tr><td style='vertical-align:top;'>") + getInfoIconBase64() + QString::fromUtf8("</td>");
    information += QString::fromUtf8("<td><ol>");

    for (const QString& infoItem : std::as_const(infoItems))
    {
        // cppcheck-suppress useStlAlgorithm
        information += QString::fromUtf8("<li>") + infoItem + QString::fromUtf8("</li>");

    }

    information += QString::fromUtf8("</ol></td></tr>");
    information += tableEnd();

    information += QString::fromUtf8("</div>");

    return information;
}

QString TooltipCreator::getInfoIconBase64() const
{
    QIcon info = getInfoIcon();
    QImage img = info.pixmap(QSize(48, 48)).toImage();
    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    img.save(&buffer, "PNG");

    return (QString::fromLatin1("<img src=\"data:image/png;base64,%1\">")
            .arg(QString::fromLatin1(byteArray.toBase64().data())));
}

QIcon TooltipCreator::getInfoIcon() const
{
    return QIcon::fromTheme(QLatin1String("dialog-information"));
}

QString TooltipCreator::tooltip(const Parser* const parser)
{
    if (!parser)
    {
        return QString();
    }

    QString tooltip;
    tooltip += QString::fromUtf8("<html><head><title></title></head>");
    tooltip += QString::fromUtf8("<body>");

    tooltip += tableStart();
    tooltip += createSection(i18n("Options"),   parser->options());
    tooltip += createSection(i18n("Modifiers"), parser->modifiers(), true);
    tooltip += tableEnd();

    if (!parser->modifiers().isEmpty())
    {
        tooltip += additionalInformation();
    }

    tooltip += QString::fromUtf8("</body>");
    tooltip += QString::fromUtf8("</html>");

    return tooltip;
}

QString TooltipCreator::tableStart(int widthPercentage)
{
    QString w = QString::number(widthPercentage) + QLatin1Char('%');

    return QString::fromUtf8("<table width=\"%1\" cellspacing=\"0\" cellpadding=\"0\" border=\"0\">").arg(w);
}

QString TooltipCreator::tableStart()
{
    return tableStart(100);
}

QString TooltipCreator::tableEnd()
{
    return QString::fromUtf8("</table>");
}

QString TooltipCreator::markOption(const QString& str)
{
    QString result = str;

    QRegularExpression optionsRegExp(QLatin1String("\\|\\|(.*)\\|\\|"));
    optionsRegExp.setPatternOptions(QRegularExpression::InvertedGreedinessOption);

    result.replace(optionsRegExp, QString::fromUtf8("<i><font color=\"%1\">\\1</font></i>")
                   .arg(qApp->palette().color(QPalette::Link).name()));
    return result;
}

QString TooltipCreator::createHeader(const QString& str)
{
    QString result;
    QString templateStr = QString::fromUtf8("<tr><td style=\"background-color: %1; padding:0.25em;\" colspan=\"2\">"
                                  "<nobr><font color=\"%2\"><center><b>%3"
                                  "</b></center></font></nobr></td></tr>")
                          .arg(qApp->palette().color(QPalette::Highlight).name())
                          .arg(qApp->palette().color(QPalette::HighlightedText).name());

    result += templateStr.arg(str);

    return result;
}

QString TooltipCreator::createEntries(const RulesList& data)
{
    QString result;

    for (const Rule* const t : std::as_const(data))
    {
        for (const Token* const token : std::as_const(t->tokens()))
        {
            // cppcheck-suppress useStlAlgorithm
            result += QString::fromUtf8("<tr>"
                              "<td style=\"background-color: %1;\">"
                              "<font color=\"%2\"><b>&nbsp;%3&nbsp;</b></font></td>"
                              "<td>&nbsp;%4&nbsp;</td></tr>")
                      .arg(qApp->palette().color(QPalette::Base).name())
                      .arg(qApp->palette().color(QPalette::Text).name())
                      .arg(markOption(token->id()))
                      .arg(markOption(token->description()));
        }
    }

    return result;
}

QString TooltipCreator::createSection(const QString& sectionName, const RulesList& data, bool lastSection)
{
    if (data.isEmpty())
    {
        return QString();
    }

    QString result;

    result += createHeader(sectionName);
    result += createEntries(data);

    if (!lastSection)
    {
        result += QString::fromUtf8("<tr></tr>");
    }

    return result;
}

} // namespace Digikam
