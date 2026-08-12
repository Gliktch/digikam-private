/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-15
 * Description : localize selector widget
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QPoint>

// Local includes

#include "digikam_export.h"
#include "dlayoutbox.h"

class QListWidgetItem;

namespace Digikam
{

class DIGIKAM_EXPORT LocalizeSelector : public DHBox
{
    Q_OBJECT

public:

    explicit LocalizeSelector(QWidget* const parent);
    ~LocalizeSelector()                               override;

    void populateTranslationEntries();

Q_SIGNALS:

    void signalTranslate(const QString& lang);

private Q_SLOTS:

    void slotLocalizeChanged();
    void slotOpenLocalizeSetup();
    void slotTranslate(QListWidgetItem*);

private:

    class Private;
    Private* const d = nullptr;
};

// -----------------------------------------------------------------

class DIGIKAM_EXPORT LocalizeSelectorList : public QWidget
{
    Q_OBJECT

public:

    explicit LocalizeSelectorList(QWidget* const parent);
    ~LocalizeSelectorList()                               override;

    void setTitle(const QString& title);

    void clearLanguages();
    void addLanguage(const QString& code);
    QStringList languagesList() const;

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotAppendTranslation(const QString& lang);
    void slotShowContextMenu(const QPoint& pos);

private:

    class Private;
    Private* const d = nullptr;
};

// -----------------------------------------------------------------

/**
 * Helpher re-entrant static method to translate a string with online translator.
 * Language from string is auto-detected, and target language is specified to 'trCode'.
 * If string can be processed, translation is returned to 'tr' and function return true,
 * else false is returned with a dysfuntion description in 'error'.
 */
bool DIGIKAM_EXPORT s_inlineTranslateString(const QString& text,
                                            const QString& trCode,
                                            QString& tr,
                                            QString& error);

} // namespace Digikam
