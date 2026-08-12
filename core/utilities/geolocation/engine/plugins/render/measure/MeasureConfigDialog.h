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

#include "ui_MeasureConfigDialog.h"
#include "MeasureToolPlugin.h"

namespace Marble
{

class MeasureConfigDialog : public QDialog
{
    Q_OBJECT

public:

    explicit MeasureConfigDialog(QDialog* parent = nullptr);
    ~MeasureConfigDialog() override;

    bool showBearingLabel() const;
    bool showBearingLabelChange() const;
    bool showDistanceLabels() const;
    bool showRadius() const;
    bool showPerimeter() const;
    bool showCircumference() const;
    bool showPolygonArea() const;
    bool showCircularArea() const;
    MeasureToolPlugin::PaintMode paintMode() const;

    void setShowBearingLabel(bool);
    void setShowBearingLabelChange(bool);
    void setShowDistanceLabels(bool);
    void setShowRadius(bool);
    void setShowPerimeter(bool);
    void setShowCircumference(bool);
    void setShowPolygonArea(bool);
    void setShowCircularArea(bool);
    void setPaintMode(MeasureToolPlugin::PaintMode);

private Q_SLOTS:

    void updateTabs();

Q_SIGNALS:

    void applied();

private:

    Ui::MeasureConfigDialog* ui = nullptr;
};

} // namespace Marble
