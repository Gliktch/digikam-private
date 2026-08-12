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

#include "GeoParser.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "GeoDocument.h"
#include "GeoTagHandler.h"
#include "digikam_debug.h"

namespace Marble
{

// Set to a value greater than 0, to dump parent node chain while parsing
#define DUMP_PARENT_STACK 0

GeoParser::GeoParser(GeoDataGenericSourceType source)
    : QXmlStreamReader(),
      m_document(nullptr),
      m_source(source)
{
}

GeoParser::~GeoParser()
{
    delete m_document;
}

#if DUMP_PARENT_STACK > 0
static void dumpParentStack(const QString& name, int size, bool close)
{
    static int depth = 0;

    if (!close)
    {
        depth++;
    }

    QString result;

    for (int i = 0; i < depth; ++i)
    {
        result += QLatin1Char(' ');
    }

    if (close)
    {
        depth--;
        result += QLatin1String("</");
    }

    else
    {
        result += QLatin1Char('<');
    }

    result += name + QLatin1String("> stack size ") + QString::number(size);
    fprintf(stderr, "%s\n", qPrintable(result));
}
#endif

bool GeoParser::read(QIODevice* device)
{
    // Assert previous document got released.
    Q_ASSERT(!m_document);
    m_document = createDocument();
    Q_ASSERT(m_document);

    // Set data source
    setDevice(device);

    // Start parsing
    while (!atEnd())
    {
        readNext();

        if (isStartElement())
        {
            if (isValidRootElement())
            {
#if DUMP_PARENT_STACK > 0
                dumpParentStack(name().toString(), m_nodeStack.size(), false);
#endif

                parseDocument();

                if (!m_nodeStack.isEmpty())
                    raiseError(
                        // Keep trailing space in both strings, to match translated string
                        // TODO: check if that space is kept through the tool pipeline
                        //~ singular Parsing failed line %1. Still %n unclosed tag after document end.
                        //~ plural Parsing failed line %1. Still %n unclosed tags after document end.
                        i18n("Parsing failed line %1. Still %2 unclosed tag(s) after document end. ",
                             lineNumber(), m_nodeStack.size()) + errorString());
            }

            else
            {
                return false;
            }
        }
    }

    if (error())
    {
        if (lineNumber() == 1)
        {
            raiseError(QString());
        }

        // Defer the deletion to the dtor
        /** @todo: Remove this workaround around Marble 1.4 */
        // delete releaseDocument();
    }

    return !error();
}

bool GeoParser::isValidElement(const QString& tagName) const
{
    return name() == tagName;
}

GeoStackItem GeoParser::parentElement(unsigned int depth) const
{
    QStack<GeoStackItem>::const_iterator it = m_nodeStack.constEnd() - 1;

    if (it - depth < m_nodeStack.constBegin())
    {
        return GeoStackItem();
    }

    return *(it - depth);
}

void GeoParser::parseDocument()
{
    if (!isStartElement())
    {
        raiseError(i18n("Error parsing file at line: %1 and column %2 . ")
                   .arg(lineNumber()).arg(columnNumber())
                   +  i18n("This is an Invalid File"));
        return;
    }

    bool processChildren = true;
    QualifiedName qName(name().toString(), namespaceUri().toString());

    if (tokenType() == QXmlStreamReader::Invalid)
    {
        raiseWarning(QString::fromUtf8("%1: %2").arg(error()).arg(errorString()));
    }

    GeoStackItem stackItem(qName, nullptr);

    if (const GeoTagHandler* handler = GeoTagHandler::recognizes(qName))
    {
        stackItem.assignNode(handler->parse(*this));
        processChildren = !isEndElement();
    }

    // Only add GeoStackItem to the parent chain, if the tag handler
    // for the current element possibly contains non-textual children.
    // Consider following DGML snippet "<name>Test</name>" - the
    // DGMLNameTagHandler assumes that <name> only contains textual
    // children, and reads the joined value of all children using
    // readElementText(). This implicates that tags like <name>
    // don't contain any children that would need to be processed using
    // this parseDocument() function.
    if (processChildren)
    {
        m_nodeStack.push(stackItem);
#if DUMP_PARENT_STACK > 0
        dumpParentStack(name().toString(), m_nodeStack.size(), false);
#endif

        while (!atEnd())
        {
            readNext();

            if (isEndElement())
            {
                m_nodeStack.pop();
#if DUMP_PARENT_STACK > 0
                dumpParentStack(name().toString(), m_nodeStack.size(), true);
#endif
                break;
            }

            if (isStartElement())
            {
                parseDocument();
            }
        }
    }

#if DUMP_PARENT_STACK > 0

    else
    {
        // This is only used for debugging purposes.
        m_nodeStack.push(stackItem);
        dumpParentStack(name().toString() + QLatin1String("-discarded"), m_nodeStack.size(), false);

        m_nodeStack.pop();
        dumpParentStack(name().toString() + QLatin1String("-discarded"), m_nodeStack.size(), true);
    }

#endif
}

void GeoParser::raiseWarning(const QString& warning)
{
    // TODO: Maybe introduce a strict parsing mode where we feed the warning to
    // raiseError() (which stops parsing).
    qCDebug(DIGIKAM_GEOENGINE_LOG) << "[GeoParser::raiseWarning] -> " << warning;
}

QString GeoParser::attribute(const char* attributeName) const
{
    return attributes().value(QLatin1String(attributeName)).toString();
}

GeoDocument* GeoParser::releaseDocument()
{
    GeoDocument* document = m_document;
    m_document = nullptr;
    return document;
}

} // namespace Marble
