/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-28
 * Description : a digiKam image editor tool to process image
 *               free rotation.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"

class QPixmap;
class QString;

using namespace Digikam;

namespace DigikamEditorFreeRotationToolPlugin
{

class FreeRotationTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit FreeRotationTool(QObject* const parent);
    ~FreeRotationTool()         override;

public Q_SLOTS:

    void slotAutoAdjustP1Clicked();
    void slotAutoAdjustP2Clicked();
    void slotAutoAdjustClicked();

private Q_SLOTS:

    void slotResetSettings()    override;
    void slotColorGuideChanged();

private:

    void readSettings()         override;
    void writeSettings()        override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;

    QString centerString(const QString& str, int maxLength = -1)         const;
    QString repeatString(const QString& str, int times)                  const;
    QPixmap generateBtnPixmap(const QString& label, const QColor& color) const;
    QString generateButtonLabel(const QPoint& p)                         const;
    double  calculateAutoAngle()                                         const;
    bool    pointIsValid(const QPoint& p)                                const;
    void    setPointInvalid(QPoint& p);
    void    updatePoints();
    void    resetPoints();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorFreeRotationToolPlugin
