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

#include "webwidget.h"

// Qt includes

#include <QCloseEvent>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{
class Q_DECL_HIDDEN WebWidget::Private
{
public:

    Private() = default;

    QWidget* parent = nullptr;
};

WebWidget::WebWidget(QWidget* const parent)
    : QWebView(parent),
      d       (new Private)
{
    d->parent = parent;
}

void WebWidget::closeEvent(QCloseEvent* event)
{
    Q_EMIT closeView(false);

    event->accept();
}

WebWidget::~WebWidget()
{
    delete d;
}

} // namespace Digikam

#include "moc_webwidget.cpp"
