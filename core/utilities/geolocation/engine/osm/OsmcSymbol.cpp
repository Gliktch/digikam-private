/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "OsmcSymbol.h"

// Qt includes

#include <QFile>
#include <QPainter>

// Local includes

#include "digikam_debug.h"

namespace Marble
{

OsmcSymbol::OsmcSymbol(const QString& tag, int size)
    : m_wayColor        (Qt::white),
      m_backgroundColor (Qt::black),
      m_foreground      (nullptr),
      m_foreground2     (nullptr),
      m_textColor       (Qt::black),
      m_side            (size)
{
    m_backgroundTypes
            << QString::fromUtf8("round")
            << QString::fromUtf8("circle")
            << QString::fromUtf8("frame");

    m_foregroundTypes
            << QString::fromUtf8("dot")
            << QString::fromUtf8("bowl")
            << QString::fromUtf8("circle")
            << QString::fromUtf8("bar")
            << QString::fromUtf8("stripe")
            << QString::fromUtf8("cross")
            << QString::fromUtf8("x")
            << QString::fromUtf8("slash")
            << QString::fromUtf8("backslash")
            << QString::fromUtf8("rectangle")
            << QString::fromUtf8("rectangle_line")
            << QString::fromUtf8("triangle")
            << QString::fromUtf8("triangle_turned")
            << QString::fromUtf8("triangle_line")
            << QString::fromUtf8("diamond")
            << QString::fromUtf8("pointer")
            << QString::fromUtf8("fork")
            << QString::fromUtf8("arch")
            << QString::fromUtf8("turned_T")
            << QString::fromUtf8("L")
            << QString::fromUtf8("lower")
            << QString::fromUtf8("corner")
            << QString::fromUtf8("drop_line")
            << QString::fromUtf8("horse")
            << QString::fromUtf8("hiker");

    m_precoloredForegroundTypes
            << QString::fromUtf8("wolfshook")
            << QString::fromUtf8("shell")
            << QString::fromUtf8("shell_modern")
            << QString::fromUtf8("ammonit")
            << QString::fromUtf8("mine")
            << QString::fromUtf8("hiker")
            << QString::fromUtf8("heart")
            << QString::fromUtf8("tower")
            << QString::fromUtf8("bridleway");

    if (parseTag(tag))
    {
        render();
    }
}

OsmcSymbol::~OsmcSymbol()
{
    delete m_foreground;
    delete m_foreground2;
}

bool OsmcSymbol::parseTag(const QString& tag)
{
    QStringList parts = tag.split(QLatin1Char(':'));

    if (parts.size() < 2)
    {
        return false;
    }

    if (m_foreground)
    {
        delete m_foreground;
        m_foreground = nullptr;
    }

    if (m_foreground2)
    {
        delete m_foreground2;
        m_foreground2 = nullptr;
    }

    // Determine way color

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

    if (QColor::isValidColorName(parts.at(0)))
    {
        m_wayColor.fromString(parts.at(0));

#else

    if (QColor::isValidColor(parts.at(0)))
    {
        m_wayColor.setNamedColor(parts.at(0));

#endif

    }

    else
    {
        return false;
    }

    if (!parseBackground(parts.at(1)))
    {
        return false;
    }

    if      (parts.size() == 3)
    {
        m_foreground = parseForeground(parts.at(2));
    }
    else if (parts.size() == 4)
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

        if (QColor::isValidColorName(parts.at(3)))

#else

        if (QColor::isValidColor(parts.at(3)))

#endif

        {
            m_text      = parts.at(2);
            m_textColor = parts.at(3);
        }
        else
        {
            m_foreground  = parseForeground(parts.at(2));
            m_foreground2 = parseForeground(parts.at(3));
        }
    }
    else if (parts.size() == 5)
    {
        m_foreground = parseForeground(parts.at(2));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

        if (QColor::isValidColorName(parts.at(4)))

#else

        if (QColor::isValidColor(parts.at(4)))

#endif

        {
            m_text      = parts.at(3);
            m_textColor = parts.at(4);
        }
        else
        {
            return false;
        }
    }
    else if (parts.size() == 6)
    {
        m_foreground  = parseForeground(parts.at(2));
        m_foreground2 = parseForeground(parts.at(3));

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

        if (QColor::isValidColorName(parts.at(5)))

#else

        if (QColor::isValidColor(parts.at(5)))

#endif

        {
            m_text = parts.at(4);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

            m_textColor.fromString(parts.at(5));

#else

            m_textColor.setNamedColor(parts.at(5));

#endif
        }

        else
        {
            return false;
        }
    }

    else
    {
        return false;
    }

    return true;
}

bool OsmcSymbol::parseBackground(const QString& bg)
{
    QString color = bg.section(QString::fromUtf8("_"), 0, 0);
    QString type  = bg.section(QString::fromUtf8("_"), 1, -1);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

    if (!QColor::isValidColorName(color))

#else

    if (!QColor::isValidColor(color))

#endif

    {
        return false;
    }

    // Plain color was provided

    if      (type.isEmpty())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

        m_backgroundColor.fromString(color);

#else

        m_backgroundColor.setNamedColor(color);

#endif

        m_backgroundType = type;
    }

    else if (m_backgroundTypes.contains(type))
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

        m_backgroundColor.fromString(color);

#else

        m_backgroundColor.setNamedColor(color);

#endif

        m_backgroundType = type;
    }

    else
    {
        return false;
    }

    return true;
}

QSvgRenderer* OsmcSymbol::parseForeground(const QString& fg)
{
    if (m_precoloredForegroundTypes.contains(fg))
    {
        return new QSvgRenderer(QString::fromUtf8(":/osmc-symbols/%1.svg").arg(fg));
    }

    QString color = fg.section(QLatin1Char('_'), 0, 0);
    QString type  = fg.section(QLatin1Char('_'), 1, -1);

    if (

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))

        QColor::isValidColorName(color) &&

#else

        QColor::isValidColor(color) &&

#endif

        m_foregroundTypes.contains(type)
       )
    {
        // Open svg resource and load contents to QByteArray

        QFile file(QString::fromUtf8(":/osmc-symbols/%1.svg").arg(type));

        if (!file.open(QIODevice::ReadOnly))
        {
            return nullptr;
        }

        QByteArray baData = file.readAll();

        // Load svg contents to xml document

        QDomDocument doc;
        doc.setContent(baData);

        // Recursively change color

        QDomElement rootElement = doc.documentElement();
        setXMLAttribute(rootElement, QString::fromUtf8("path"), QString::fromUtf8("fill"), color);

        // Create and return svg renderer with edited contents

        return new QSvgRenderer(doc.toByteArray());
    }

    return nullptr;
}

void OsmcSymbol::render()
{
    m_image = QImage(m_side, m_side, QImage::Format_ARGB32);
    m_image.fill(Qt::transparent);

    QPainter painter(&m_image);
    painter.setRenderHint(QPainter::Antialiasing);

    // Default size of background

    int w = m_side;
    int h = m_side;

    // If there is some text, our background size must be recalculated

    if (!m_text.isEmpty())
    {
        QFont font = painter.font();
        font.setPixelSize(int(m_side * 0.8));
        font.setBold(true);
        painter.setFont(font);
        QFontMetrics fm = QFontMetrics(font);

        h = fm.height();
        w = qMax(h, fm.horizontalAdvance(m_text));
    }

    const QRect bgRect = QRect((m_side - w) / 2, (m_side - h) / 2, w, h);

    // Draw symbol's background

    if      (m_backgroundType.isEmpty())
    {
        painter.fillRect(bgRect, m_backgroundColor);
    }
    else if (m_backgroundType == QString::fromUtf8("round"))
    {
        painter.setBrush(m_backgroundColor);
        painter.setPen(m_backgroundColor);
        painter.drawEllipse(bgRect);
    }
    else if (m_backgroundType == QString::fromUtf8("circle"))
    {
        painter.setBrush(Qt::white);
        painter.setPen(QPen(m_backgroundColor, m_side / 10));
        painter.drawEllipse(bgRect);
    }
    else if (m_backgroundType == QString::fromUtf8("frame"))
    {
        painter.setPen(QPen(m_backgroundColor, m_side / 10));
        painter.fillRect(bgRect, Qt::white);
        painter.drawRect(bgRect);
    }

    QPixmap foregrounds(bgRect.size());
    foregrounds.fill(Qt::transparent);
    QPainter fgPainter(&foregrounds);
    m_foreground  ? m_foreground->render(&fgPainter)  : void();
    m_foreground2 ? m_foreground2->render(&fgPainter) : void();
    painter.drawPixmap(bgRect, foregrounds);

    if (!m_text.isEmpty())
    {
        // Draw text with provided color

        painter.setPen(m_textColor);
        painter.drawText(bgRect, Qt::AlignCenter, m_text);
    }

    painter.end();
}

QImage OsmcSymbol::icon() const
{
    return m_image;
}

QColor OsmcSymbol::wayColor() const
{
    return m_wayColor;
}

void OsmcSymbol::setXMLAttribute(QDomElement& elem, const QString& tag, const QString& attr, const QString& attrValue)
{
    // If elem's tag is equal to the provided one then overwrite desired attribute

    if (elem.tagName() == tag)
    {
        elem.setAttribute(attr, attrValue);
    }

    // Do the same for all the child nodes

    for (int i = 0 ; i < elem.childNodes().count() ; ++i)
    {
        if (!elem.childNodes().at(i).isElement())
        {
            continue;
        }

        QDomElement child = elem.childNodes().at(i).toElement();
        setXMLAttribute(child, tag, attr, attrValue);
    }
}

} // namespace Marble
