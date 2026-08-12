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
#include <QList>
#include <QSharedPointer>

namespace DigikamGenericHtmlGalleryPlugin
{

class AbstractThemeParameter;

/**
 * An HTML theme. This class gives access to the various theme properties,
 * including the theme parameters.
 */
class GalleryTheme
{
public:

    typedef QSharedPointer<GalleryTheme>   Ptr;
    typedef QList<Ptr>                     List;
    typedef QList<AbstractThemeParameter*> ParameterList;

public:

    ~GalleryTheme();

    // Internal theme name == name of theme folder
    QString internalName()          const;
    QString name()                  const;
    QString comment()               const;

    QString authorName()            const;
    QString authorUrl()             const;

    QString previewName()           const;
    QString previewUrl()            const;
    bool allowNonsquareThumbnails() const;

    /**
     * Theme directory on hard disk
     */
    QString directory()             const;

    ParameterList parameterList()   const;

    /**
     * Returns the list of available themes
     */
    static const List& getList();
    static Ptr findByInternalName(const QString& internalName);

private:

    /// @note disabled
    GalleryTheme();
    GalleryTheme(const GalleryTheme&)            = delete;
    GalleryTheme& operator=(const GalleryTheme&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericHtmlGalleryPlugin
