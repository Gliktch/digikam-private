/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-16
 * Description : a combo box widget re-implemented with a
 *               reset button to switch to a default item
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QComboBox>
#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DComboBox : public QWidget
{

    Q_OBJECT

public:

    explicit DComboBox(QWidget* const parent = nullptr);
    ~DComboBox() override;

    void setCurrentIndex(int d);
    int  currentIndex() const;

    void setDefaultIndex(int d);
    int  defaultIndex() const;

    QComboBox* combo()  const;

    void addItem(const QString& t,
                 const QVariant& data = QVariant());
    void insertItem(int index, const QString& t,
                    const QVariant& data = QVariant());

Q_SIGNALS:

    void reset();
    void activated(int);
    void currentIndexChanged(int);

public Q_SLOTS:

    void slotReset();

private Q_SLOTS:

    void slotItemActivated(int);
    void slotCurrentIndexChanged(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
