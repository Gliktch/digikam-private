/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-10-05
 * Description : a tool bar for slideshow
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 * SPDX-FileCopyrightText:      2021 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QKeyEvent>

// Local includes

#include "digikam_export.h"
#include "slideshowsettings.h"
#include "dlayoutbox.h"

class QAction;

namespace DigikamGenericSlideShowPlugin
{

class SlideToolBar : public DHBox
{
    Q_OBJECT

public:

    explicit SlideToolBar(SlideShowSettings* const settings, QWidget* const parent);
    ~SlideToolBar()                  override;

    bool isPaused() const;
    void pause(bool val);

    void setEnabledPlay(bool val);
    void setEnabledNext(bool val);
    void setEnabledPrev(bool val);
    void setEnabledDelete(bool val);

protected:

    void keyPressEvent(QKeyEvent* e) override;

Q_SIGNALS:

    void signalNext();
    void signalPrev();
    void signalClose();
    void signalPlay();
    void signalPause();
    void signalUpdateSettings();
    void signalScreenSelected(int);
    void signalRemoveImageFromList();

private Q_SLOTS:

    void slotPlayBtnToggled();
    void slotNexPrevClicked();
    void slotRemoveImage();
    void slotScreenSelected(QAction*);
    void slotMenuSlideShowConfiguration();
    void slotChangeDelayButtonPressed();

private:

    class Private;
    Private* const d = nullptr;

    friend class SlideShowLoader;
};

} // namespace DigikamGenericSlideShowPlugin
