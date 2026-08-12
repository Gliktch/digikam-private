/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-7-30
 * Description : Widget for displaying HTML in the backends
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <qwebview.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT WebWidget : public QWebView
{
    Q_OBJECT

public:

    explicit WebWidget(QWidget* const parent = nullptr);
    ~WebWidget();

Q_SIGNALS:

    void closeView(bool val);

protected:

    void closeEvent(QCloseEvent* event) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
