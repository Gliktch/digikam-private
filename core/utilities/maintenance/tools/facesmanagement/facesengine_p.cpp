/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-11-08
 * Description : face detection, recognition, and training controller
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facesengine_p.h"

namespace Digikam
{

BenchmarkMessageDisplay::BenchmarkMessageDisplay(const QString& richText)
    : QWidget(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* const vbox     = new QVBoxLayout;
    QTextEdit* const edit       = new QTextEdit;
    vbox->addWidget(edit, 1);
    QPushButton* const okButton = new QPushButton(i18n("OK"));
    vbox->addWidget(okButton, 0, Qt::AlignRight);

    setLayout(vbox);

    connect(okButton, SIGNAL(clicked()),
            this, SLOT(close()));

    edit->setHtml(richText);
    QApplication::clipboard()->setText(edit->toPlainText());

    resize(500, 400);
    show();
    raise();
}

} // namespace Digikam
