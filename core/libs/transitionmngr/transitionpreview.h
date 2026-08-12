/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-04
 * Description : A label to show transition preview
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QLabel>
#include <QString>
#include <QList>
#include <QUrl>

// Local includes

#include "transitionmngr.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT TransitionPreview : public QLabel
{
    Q_OBJECT

public:

    explicit TransitionPreview(QWidget* const parent = nullptr);
    ~TransitionPreview() override;

    void setImagesList(const QList<QUrl>& images);

    void startPreview(TransitionMngr::TransType eff);
    void stopPreview();

private Q_SLOTS:

    void slotProgressTransition();
    void slotRestart();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
