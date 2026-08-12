/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * SPDX-FileCopyrightText: 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "imageshackwidget.h"

// Qt includes

#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QStringList>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QApplication>
#include <QPushButton>
#include <QMimeDatabase>
#include <QMimeType>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "ditemslist.h"
#include "dtextedit.h"
#include "imageshacksession.h"

using namespace Digikam;

namespace DigikamGenericImageShackPlugin
{

class Q_DECL_HIDDEN ImageShackWidget::Private
{
public:

    Private() = default;

public:

    DItemsList*        imgList              = nullptr;
    DInfoInterface*    iface                = nullptr;
    ImageShackSession* session              = nullptr;

    QLabel*            headerLbl            = nullptr;
    QLabel*            accountNameLbl       = nullptr;

    DTextEdit*         tagsFld              = nullptr;

    QCheckBox*         privateImagesChb     = nullptr;
    QCheckBox*         remBarChb            = nullptr;

    QPushButton*       chgRegCodeBtn        = nullptr;
    QPushButton*       reloadGalleriesBtn   = nullptr;

    QComboBox*         galleriesCob         = nullptr;

    DProgressWdg*      progressBar          = nullptr;
};

} // namespace DigikamGenericImageShackPlugin
