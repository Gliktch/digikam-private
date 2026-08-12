/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-06
 * Description : metadata template settings panel.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTabWidget>

// Local includes

#include "template.h"

namespace Digikam
{

class TemplatePanel : public QTabWidget
{
    Q_OBJECT

public:

    enum TemplateTab
    {
        RIGHTS=0,
        LOCATION,
        CONTACT,
        SUBJECTS
    };

public:

    explicit TemplatePanel(QWidget* const parent = nullptr);
    ~TemplatePanel();

    void     setTemplate(const Template& t);
    Template getTemplate() const;

    void     apply();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
