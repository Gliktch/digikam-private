/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "inattaxonedit.h"

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericINatPlugin
{

TaxonEdit::TaxonEdit(QWidget* const)
    : QLineEdit()
{
}

void TaxonEdit::focusInEvent(QFocusEvent* e)
{
    QLineEdit::focusInEvent(e);

    if ((e->reason() == Qt::MouseFocusReason) && text().isEmpty())
    {
        Q_EMIT inFocus();
    }
}

} // namespace DigikamGenericINatPlugin

#include "moc_inattaxonedit.cpp"
