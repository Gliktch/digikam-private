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

#include <QByteArray>
#include <QString>
#include <QWidget>

class KConfigGroup;

namespace DigikamGenericHtmlGalleryPlugin
{

/**
 * Represents a theme parameter. For each type of parameter, one should inherit
 * from this class and add the necessary code in the Theme class to load the
 * new type.
 */
class AbstractThemeParameter
{
public:

    AbstractThemeParameter();
    virtual ~AbstractThemeParameter();

    /**
     * Reads theme parameters from configGroup. Initializes the internalName,
     * name and defaultValue fields.
     */
    virtual void init(const QByteArray& internalName, const KConfigGroup* configGroup);

    QByteArray internalName() const;
    QString name()            const;
    QString defaultValue()    const;

    /**
     * This method should return a QWidget representing the parameter,
     * initialized with value.
     */
    virtual QWidget* createWidget(QWidget* parent, const QString& value) const = 0;

    /**
     * The opposite of createWidget: given a widget previously created with
     * createWidget, this method returns the current widget value.
     */
    virtual QString valueFromWidget(QWidget*) const = 0;

private:

    /// @note disabled
    AbstractThemeParameter(const AbstractThemeParameter&)            = delete;
    AbstractThemeParameter& operator=(const AbstractThemeParameter&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericHtmlGalleryPlugin
