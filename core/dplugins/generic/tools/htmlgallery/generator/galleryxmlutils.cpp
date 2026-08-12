/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "galleryxmlutils.h"

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericHtmlGalleryPlugin
{

bool XMLWriter::open(const QString& name)
{
    xmlTextWriterPtr ptr = xmlNewTextWriterFilename(name.toUtf8().constData(), 0);

    if (!ptr)
    {
        return false;
    }

    m_writer.assign(ptr);

    int rc = xmlTextWriterStartDocument(ptr, nullptr, "UTF-8", nullptr);

    if (rc < 0)
    {
        m_writer.assign(nullptr);

        return false;
    }

    xmlTextWriterSetIndent(ptr, 1);

    return true;
}

XMLWriter::operator xmlTextWriterPtr() const
{
    return m_writer;
}

void XMLWriter::writeElement(const char* element, const QString& value)
{
    int rc = xmlTextWriterWriteElement(m_writer,
                                       BAD_CAST element,
                                       BAD_CAST value.toUtf8().data());

    if (rc < 0)
    {
        qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Error while calling Libxml2::xmlTextWriterWriteElement()";
    }
}

void XMLWriter::writeElement(const char* element, int value)
{
    writeElement(element, QString::number(value));
}

// ------------------------------------------------------

void XMLAttributeList::write(XMLWriter& writer) const
{
    Map::const_iterator it  = m_map.begin();
    Map::const_iterator end = m_map.end();

    for ( ; it != end ; ++it)
    {
        int rc = xmlTextWriterWriteAttribute(writer,
                                             BAD_CAST it.key().toLatin1().data(),
                                             BAD_CAST it.value().toLatin1().data());
        if (rc < 0)
        {
            qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Error while calling Libxml2::xmlTextWriterWriteAttribute()";
        }
    }
}

void XMLAttributeList::append(const QString& key, const QString& value)
{
    m_map[key] = value;
}

void XMLAttributeList::append(const QString& key, int value)
{
    m_map[key] = QString::number(value);
}

// ------------------------------------------------------

XMLElement::XMLElement(XMLWriter& writer,
                       const QString& element,
                       const XMLAttributeList* attributeList)
    : m_writer(writer)
{
    int rc = xmlTextWriterStartElement(writer,
                                       BAD_CAST element.toLatin1().data());

    if (rc < 0)
    {
        qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Error while calling Libxml2::xmlTextWriterStartElement()";
    }

    if (attributeList)
    {
        attributeList->write(writer);
    }
}

XMLElement::~XMLElement()
{
    xmlTextWriterEndElement(m_writer);
}

} // namespace DigikamGenericHtmlGalleryPlugin
