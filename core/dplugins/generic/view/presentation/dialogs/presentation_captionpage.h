/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2008      by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "ui_presentation_captionpage.h"

namespace DigikamGenericPresentationPlugin
{

class PresentationContainer;

class PresentationCaptionPage : public QWidget,
                                public Ui::PresentationCaptionPage
{
    Q_OBJECT

public:

    explicit PresentationCaptionPage(QWidget* const parent,
                                     PresentationContainer* const sharedData);
    ~PresentationCaptionPage() override = default;

    void readSettings();
    void saveSettings();

private Q_SLOTS:

    void slotCommentsFontColorChanged();
    void slotCommentsBgColorChanged();
    void slotOpenFontDialog();

private:

    PresentationContainer* m_sharedData = nullptr;
};

} // namespace DigikamGenericPresentationPlugin
