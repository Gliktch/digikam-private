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

#include <QtGlobal>

class QString;

namespace Marble
{

class TemplateDocumentPrivate;

/**
 * @brief The Template Document
 *
 * The class represents a template engine for
 * Marble HTML/plain text. It allows to template big
 * documents with variables like %variable_name% or
 * to make includes like %!{include_name}%
 *
 * "Include" is a special type of template variables which
 * allows to include extra already templated text into the
 * template. E.g. %!{bootstrap}% will include bootstrap CSS
 * files into html file, where this include is called
 *
 * @see TemplateDocument()
 *
 */
class TemplateDocument
{
public:

    TemplateDocument();
    explicit TemplateDocument(const QString& templateText);
    ~TemplateDocument();

    /**
     * @brief Returns the current template value of @p key
     * @param key template key (\<here\>)
     * @return value of the template
     */
    QString value(const QString& key) const;

    /**
     * @brief Change set template value into new one
     * @param key template key
     * @param value template value
     */
    void setValue(const QString& key, const QString& value);

    /**
     * @brief Set template text
     * @param newTemplateText new template text
     */
    void setTemplate(const QString& newTemplateText);

    /**
     * @brief Indexator for template values
     * @param key template value's index
     * @return reference for the item
     */
    QString& operator[](const QString& key);

    /**
     * @brief Final proceed text
     *
     * @return ready text with all variables and includes processed
     */
    QString finalText() const;

private:

    Q_DISABLE_COPY(TemplateDocument)

private:

    TemplateDocumentPrivate* d = nullptr;
};

} // namespace Marble
