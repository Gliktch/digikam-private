/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-17
 * Description : Metadata tags selector config panel.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "exiftoolparser.h"

class QTabWidget;

namespace Digikam
{

class MetadataSelectorView;

class DIGIKAM_EXPORT MetadataPanel : public QObject
{
    Q_OBJECT

public:

    explicit MetadataPanel(QTabWidget* const tab);
    ~MetadataPanel() override;

    void                         applySettings();

    QStringList                  getAllCheckedTags()    const;
    QList<MetadataSelectorView*> viewers()              const;

public:

    static QStringList defaultExifFilter();
    static QStringList defaultMknoteFilter();
    static QStringList defaultIptcFilter();
    static QStringList defaultXmpFilter();
    static QStringList defaultExifToolFilter();

private Q_SLOTS:

    void slotTabChanged(int);
    void slotExifToolAsyncData(const ExifToolParser::ExifToolData& data);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
