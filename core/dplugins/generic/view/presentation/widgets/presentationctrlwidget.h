/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-10-05
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2008      by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QKeyEvent>

// Local includes

#include "ui_presentationctrlwidget.h"
#include "presentationcontainer.h"

namespace DigikamGenericPresentationPlugin
{

class PresentationCtrlWidget : public QWidget,
                               public Ui::PresentationCtrlWidget
{
    Q_OBJECT

public:

    explicit PresentationCtrlWidget(QWidget* const parent,
                                    PresentationContainer* const sharedData);
    ~PresentationCtrlWidget()             override = default;

    bool canHide()                  const;
    bool isPaused()                 const;
    void setPaused(bool val);

    void setEnabledPlay(bool val);
    void setEnabledNext(bool val);
    void setEnabledPrev(bool val);

Q_SIGNALS:

    void signalNext();
    void signalPrev();
    void signalClose();
    void signalPlay();
    void signalPause();
    void signalRemoveImageFromList();

protected:

    void keyPressEvent(QKeyEvent* event) override;

private Q_SLOTS:

    void slotPlayButtonToggled();
    void slotNexPrevClicked();
    void slotChangeDelayButtonPressed();
    void slotMoveToTrash();

private:

    bool                    m_canHide       = true;
    PresentationContainer*  m_sharedData    = nullptr;

    friend class PresentationWidget;
    friend class PresentationGL;
};

} // namespace DigikamGenericPresentationPlugin
