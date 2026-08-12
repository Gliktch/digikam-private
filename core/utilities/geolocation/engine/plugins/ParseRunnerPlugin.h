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

#include <QObject>

// Local includes

#include "PluginInterface.h"

namespace Marble
{

class ParsingRunner;

/**
  * A plugin for Marble to execute a parsing task.
  */
class DIGIKAM_EXPORT ParseRunnerPlugin : public QObject, public PluginInterface
{
    Q_OBJECT

public:

    /** Constructor with optional parent object */
    explicit ParseRunnerPlugin(QObject* parent = nullptr);

    /** Destructor */
    ~ParseRunnerPlugin() override;

    /**
      * Returns a short description of the supported file format.
      *
      * Example: "Google Earth KML"
      */
    virtual QString fileFormatDescription() const = 0;

    /**
      * Returns the file extensions associated with the file format.
      *
      * Example: "kml", "kmz"
      */
    virtual QStringList fileExtensions() const = 0;

    /** Plugin factory method to create a new runner instance.
      * Method caller gets ownership of the returned object
      */
    virtual ParsingRunner* newRunner() const = 0;

    // Overridden methods with default implementations

    QIcon icon() const override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Marble

Q_DECLARE_INTERFACE(Marble::ParseRunnerPlugin, "org.kde.Marble.ParseRunnerPlugin/1.01")
