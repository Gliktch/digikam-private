/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-14
 * Description : a #include "dconfigdlgwdgWidget to edit IPTC metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011 by Victor Dodon <dodon dot victor at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dconfigdlgwidgets.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class MetadataEditDialog;

class IPTCEditWidget : public DConfigDlgWdg
{
    Q_OBJECT

public:

    explicit IPTCEditWidget(MetadataEditDialog* const parent);
    ~IPTCEditWidget()             override;

    bool isModified()       const;
    void saveSettings();
    void apply();

Q_SIGNALS:

    void signalModified();
    void signalSetReadOnly(bool);

public Q_SLOTS:

    void slotModified();
    void slotItemChanged();

private:

    void readSettings();

    int  activePageIndex()  const;
    void showPage(int page);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericMetadataEditPlugin
