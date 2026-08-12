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

#pragma once

// Qt includes

#include <QString>
#include <QPair>
#include <QStack>
#include <QXmlStreamReader>

// Local includes

#include "digikam_export.h"

namespace Marble
{

using GeoDataGenericSourceType = int;

class GeoDocument;
class GeoNode;
class GeoStackItem;

class DIGIKAM_EXPORT GeoParser : public QXmlStreamReader
{
public:

    typedef QPair<QString, QString> QualifiedName; // Tag Name & Namespace pair

    explicit GeoParser(GeoDataGenericSourceType sourceType);
    virtual ~GeoParser();

    /**
     * @brief Main API for reading the XML document.
     * This is the only method that is necessary to call to start the GeoParser.
     * To retrieve the resulting data see @see releaseDocument() and
     * @see releaseModel()
     */
    bool read(QIODevice*);

    /**
     * @brief retrieve the parsed document and reset the parser
     * If parsing was successful, retrieve the resulting document
     * and set the contained m_document pointer to 0.
     */
    GeoDocument* releaseDocument();
    GeoDocument* activeDocument()
    {
        return m_document;
    }

    // Used by tag handlers, to be overridden by GeoDataParser/GeoSceneParser
    virtual bool isValidElement(const QString& tagName) const;

    // Used by tag handlers, to access a parent element's associated GeoStackItem
    GeoStackItem parentElement(unsigned int depth = 0) const;

    // Used by tag handlers, to Q_EMIT a warning while parsing
    void raiseWarning(const QString&);

    // Used by tag handlers, to retrieve the value for an attribute of the currently parsed element
    QString attribute(const char* attributeName) const;

protected:

    /**
     * This method is intended to check if the current element being served by
     * the GeoParser is a valid Document Root element. This method is to be
     * implemented by GeoDataParser/GeoSceneParser and must check based on the
     * current XML Document type, e.g. KML, GPX etc.
     * @return @c true if the element is a valid document root.
     */
    virtual bool isValidRootElement() = 0;

    virtual GeoDocument* createDocument() const = 0;

protected:

    GeoDocument* m_document = nullptr;
    GeoDataGenericSourceType m_source;

private:

    void parseDocument();
    QStack<GeoStackItem> m_nodeStack;
};

class GeoStackItem
{
public:

    GeoStackItem()
        : m_qualifiedName(),
          m_node(nullptr)
    {
    }

    GeoStackItem(const GeoParser::QualifiedName& qualifiedName, GeoNode* node)
        : m_qualifiedName(qualifiedName),
          m_node(node)
    {
    }

    // Fast path for tag handlers
    bool represents(const char* tagName) const
    {
        return m_node && (QString::fromUtf8(tagName) == m_qualifiedName.first);
    }

    // Helper for tag handlers. Does NOT guard against miscasting. Use with care.
    template<class T>
    T* nodeAs()
    {
        Q_ASSERT(dynamic_cast<T*>(m_node) != nullptr);
        return static_cast<T*>(m_node);
    }

    template<class T>
    bool is() const
    {
        return nullptr != dynamic_cast<T*>(m_node);
    }

    GeoParser::QualifiedName qualifiedName() const
    {
        return m_qualifiedName;
    }
    GeoNode* associatedNode() const
    {
        return m_node;
    }

private:

    void assignNode(GeoNode* node)
    {
        m_node = node;
    }

private:

    friend class GeoParser;

    GeoParser::QualifiedName m_qualifiedName;
    GeoNode*                 m_node = nullptr;
};

} // namespace Marble
