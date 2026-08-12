/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-05
 * Description : film color negative inverter tool
 *
 * SPDX-FileCopyrightText: 2012 by Matthias Welwarsky <matthias at welwarsky dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "editortool.h"
#include "dcolor.h"

class QListWidgetItem;

using namespace Digikam;

namespace DigikamEditorFilmToolPlugin
{

class FilmProfile;

class FilmTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit FilmTool(QObject* const parent);
    ~FilmTool()                         override;

private Q_SLOTS:

    void slotResetSettings()            override;
    void slotScaleChanged()             override;
    void slotChannelChanged()           override;
    void slotAdjustSliders();
    void slotFilmItemActivated(QListWidgetItem* item);
    void slotExposureChanged(double val);
    void slotGammaInputChanged(double val);
    void slotColorSelectedFromTarget(const Digikam::DColor& color, const QPoint& p);
    void slotPickerColorButtonActived(bool checked);
    void slotResetWhitePoint();
    void slotColorBalanceStateChanged(int);
    void slotAutoWhitePoint(void);

private:

    void readSettings()                 override;
    void writeSettings()                override;
    void preparePreview()               override;
    void prepareFinal()                 override;
    void setPreviewImage()              override;
    void setFinalImage()                override;

    void gammaInputChanged(double val);
    void setLevelsFromFilm();
    bool eventFilter(QObject*, QEvent*) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorFilmToolPlugin
