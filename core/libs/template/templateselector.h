/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-23
 * Description : a widget to select metadata template.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"

namespace Digikam
{

class Template;

class DIGIKAM_GUI_EXPORT TemplateSelector : public DVBox
{
    Q_OBJECT

public:

    enum SelectorItems
    {
        REMOVETEMPLATE = 0,
        DONTCHANGE     = 1
    };

public:

    explicit TemplateSelector(QWidget* const parent = nullptr);
    ~TemplateSelector() override;

    Template  getTemplate() const;
    void      setTemplate(const Template& t);

    int  getTemplateIndex() const;
    void setTemplateIndex(int i);

Q_SIGNALS:

    void signalTemplateSelected();

private Q_SLOTS:

    void slotOpenSetup();
    void slotTemplateSelected();
    void slotTemplateListChanged();

private:

    void populateTemplates();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
