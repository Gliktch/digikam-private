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

#include <QDialog>

// Local includes

#include "digikam_export.h"

class QLineEdit;
class QDialogButtonBox;

namespace Marble
{

class OsmRelationEditorDialogPrivate;
class OsmTagEditorWidget;
class OsmPlacemarkData;
class GeoDataPlacemark;

/**
 * @brief The OsmRelationEditorDialog class small dialog that is used to edit relations
 * The dialog has a line edit for name input, and a OsmTagEditorWidget for tag editing
 */
class DIGIKAM_EXPORT OsmRelationEditorDialog : public QDialog
{
    Q_OBJECT

public:

    explicit OsmRelationEditorDialog(OsmPlacemarkData* relationData, QWidget* parent = nullptr);
    ~OsmRelationEditorDialog() override;

private Q_SLOTS:

    void checkFields();

private:

    void finish();

private:

    friend class OsmRelationEditorDialogPrivate;

    QDialogButtonBox* m_buttonBox       = nullptr;
    QLineEdit* m_nameLineEdit           = nullptr;
    OsmTagEditorWidget* m_tagEditor     = nullptr;
    GeoDataPlacemark* m_dummyPlacemark  = nullptr;
    OsmPlacemarkData* m_relationData    = nullptr;
};

} // namespace Marble
