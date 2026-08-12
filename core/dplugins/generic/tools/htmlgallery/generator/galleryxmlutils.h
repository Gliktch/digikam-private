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

#pragma once

// Qt includes

#include <QString>
#include <QMap>

// libxml includes

#include <libxml/xmlwriter.h>

namespace DigikamGenericHtmlGalleryPlugin
{

/**
 * A simple wrapper for a C structure pointed to by @p Ptr, which must be freed
 * with @p freeFcn
 */
template <class Ptr, void(*freeFcn)(Ptr)>

class CWrapper
{
public:

    CWrapper() = default;

    // cppcheck-suppress noExplicitConstructor
    CWrapper(Ptr ptr)        // krazy:exclude=explicit
        : m_ptr(ptr)
    {
    }

    ~CWrapper()
    {
        freeFcn(m_ptr);
    }

    operator Ptr() const
    {
        return m_ptr;
    }

    bool operator!() const
    {
        return !m_ptr;
    }

    void assign(Ptr ptr)
    {
        if (m_ptr)
        {
            freeFcn(m_ptr);
        }

        m_ptr = ptr;
    }

private:

    Ptr m_ptr = nullptr;

private:

    /// @note disabled
    CWrapper(const CWrapper&)            = delete;
    CWrapper& operator=(const CWrapper&) = delete;
};

// --------------------------------------------------------------------------

/**
 * Simple wrapper around xmlTextWriter
 */
class XMLWriter
{
public:

    bool open(const QString& name);
    operator xmlTextWriterPtr() const;

    void writeElement(const char* element, const QString& value);
    void writeElement(const char* element, int value);

private:

    CWrapper<xmlTextWriterPtr, xmlFreeTextWriter> m_writer;
};


// --------------------------------------------------------------------------
/**
 * A list of attributes for an XML element. To be used with @ref XMLElement
 */
class XMLAttributeList
{
public:

    void write(XMLWriter& writer) const;
    void append(const QString& key, const QString& value);
    void append(const QString& key, int value);

private:

    typedef QMap<QString, QString> Map;

    Map m_map;
};

// --------------------------------------------------------------------------

/**
 * A class to generate an XML element
 */
class XMLElement
{
public:

    explicit XMLElement(XMLWriter& writer,
                        const QString& element,
                        const XMLAttributeList* attributeList = nullptr);
    ~XMLElement();

private:

    XMLWriter& m_writer;

private:

    /// @note disabled
    XMLElement(const XMLElement&)            = delete;
    XMLElement& operator=(const XMLElement&) = delete;

};

} // namespace DigikamGenericHtmlGalleryPlugin
