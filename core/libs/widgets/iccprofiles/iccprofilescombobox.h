/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : a combo box containing ICC profiles
 *
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QComboBox>
#include <QIcon>
#include <QMenu>

// Local includes

#include "squeezedcombobox.h"
#include "digikam_export.h"
#include "iccprofile.h"

namespace Digikam
{

class DIGIKAM_EXPORT IccProfilesComboBox : public SqueezedComboBox
{
    Q_OBJECT

public:
    /**
     * @note Use the signal currentIndexChanged(int) for change notification
     */

    explicit IccProfilesComboBox(QWidget* const parent = nullptr);
    ~IccProfilesComboBox() override = default;

    /**
     * Checks the given profiles for validity, creates a suitable description (ICC profile description, file path),
     * removes duplicates by file path, sorts them and adds them in sorted order.
     */
    void addProfilesSqueezed(const QList<IccProfile>& profiles);

    /**
     * Add the given profile with the given description, or, if null, a standard description.
     * Does not test for duplicity, does not sort into existing profiles.
     */
    void addProfileSqueezed(const IccProfile& profile, const QString& description = QString());

    /**
     * Clears, does the same as addProfilesSqueezed, and restores the current entry if possible.
     */
    void replaceProfilesSqueezed(const QList<IccProfile>& profiles);

    /**
     * Sets a message the is displayed in the combo box and disables the combo box,
     * if the combo box is currently empty
     */
    void setNoProfileIfEmpty(const QString& message);

    /**
     * Retrieves the current profile, or a null profile if none is selected.
     */
    IccProfile currentProfile() const;

    /**
     * Sets the current profile. If profile is not in the list, sets no current item (-1)
     */
    void setCurrentProfile(const IccProfile& profile);
};

// ----------------------------------------------------------------------------

class DIGIKAM_EXPORT IccRenderingIntentComboBox : public QComboBox
{
    Q_OBJECT

public:

    explicit IccRenderingIntentComboBox(QWidget* const parent = nullptr);

    void setIntent(int intent);
    int  intent() const;
};

// ----------------------------------------------------------------------------

class DIGIKAM_EXPORT IccProfilesMenuAction : public QMenu
{
    Q_OBJECT

public:

    IccProfilesMenuAction(const QIcon& icon, const QString& text, QObject* const parent);
    IccProfilesMenuAction(const QString& text, QObject* const parent);

    /**
     * Checks the given profiles for validity, creates a suitable description (ICC profile description, file path),
     * removes duplicates (in newly added list) by file path, sorts them and adds them in sorted order.
     */
    void addProfiles(const QList<IccProfile>& profile);

    /**
     * Add the given profile with the given description, or, if null, a standard description.
     * Does not test for duplicity, does not sort into existing profiles.
     */
    void addProfile(const IccProfile& profile, const QString& description = QString());

    /**
     * Equivalent to calling clear() and addProfiles().
     */
    void replaceProfiles(const QList<IccProfile>& profile);

    /**
     * Disables if the menu is currently empty.
     */
    void disableIfEmpty();

    /**
     * Return the parent QObject.
     */
    QObject* parentObject() const;

Q_SIGNALS:

    void signalTriggeredProfile(const IccProfile& profile);

protected Q_SLOTS:

    void slotTriggered(QObject*);

protected:

    QObject*       m_parent = nullptr;

private:

    /// @note disabled
    IccProfilesMenuAction(QWidget*);
};

} // namespace Digikam
