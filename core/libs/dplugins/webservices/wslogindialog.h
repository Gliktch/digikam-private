/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-11-12
 * Description : a common login dialog for Web Service tools
 *
 * SPDX-FileCopyrightText: 2007-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Roman Tsisyk <roman at tsisyk dot com>
 * SPDX-FileCopyrightText: 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include <QDialog>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT WSLoginDialog : public QDialog
{
    Q_OBJECT

public:

    explicit WSLoginDialog(QWidget* const parent,
                           const QString& prompt,
                           const QString& header=QString(),
                           const QString& passwd=QString());
    ~WSLoginDialog() override;

    QString login()    const;
    QString password() const;

    void setLogin(const QString&);
    void setPassword(const QString&);

protected Q_SLOTS:

    void slotAccept();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
