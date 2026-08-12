/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QListWidget>
#include <QTextBrowser>
#include <QWidget>

// Local includes

#include "dwizardpage.h"
#include "gallerytheme.h"

using namespace Digikam;

namespace DigikamGenericHtmlGalleryPlugin
{

class GalleryInfo;

class ThemeListBoxItem : public QListWidgetItem
{
public:

    explicit ThemeListBoxItem(GalleryTheme::Ptr theme)
        : QListWidgetItem(theme->name()),
          m_theme        (theme)
    {
    }

    ~ThemeListBoxItem() = default;

public:

    GalleryTheme::Ptr m_theme;

private:

    Q_DISABLE_COPY(ThemeListBoxItem)
};

// ------------------------------------------------------------------------

class HTMLThemePage : public DWizardPage
{
    Q_OBJECT

public:

    explicit HTMLThemePage(QWizard* const dialog, const QString& title);
    ~HTMLThemePage()                        override;

    void initializePage()                   override;
    bool validatePage()                     override;

    GalleryTheme::Ptr currentTheme()  const;

private Q_SLOTS:

    void slotThemeSelectionChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericHtmlGalleryPlugin
