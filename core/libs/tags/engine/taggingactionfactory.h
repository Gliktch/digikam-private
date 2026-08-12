/* ============================================================
 *
 * This file is a part of Tumorprofil
 *
 * Date        : 11.09.2015
 *
 * SPDX-FileCopyrightText: 2012 by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>

// Local includes

#include "taggingaction.h"

namespace Digikam
{

class TaggingActionFactory
{
public:

    class ConstraintInterface
    {
    public:

        ConstraintInterface()           = default;
        virtual ~ConstraintInterface()  = default;
        virtual bool matches(int tagId) = 0;

    private:

        Q_DISABLE_COPY(ConstraintInterface)
    };

public:

    enum NameMatchMode
    {
        /// Default: use the "startingWith" method
        MatchStartingWithFragment,

        /// use the "contains" method
        MatchContainingFragment
    };

public:

    explicit TaggingActionFactory();
    virtual ~TaggingActionFactory();

    /// Set a fragment of a tag name to generate possible tags, as known from completers
    void setFragment(const QString& fragment);
    QString fragment() const;

    /// Set a tag which may by the user be intended to be the parent of a newly created tag
    void setParentTag(int parentTagId);
    int parentTagId() const;

    /**
     * Allows to filter the scope of suggested tags. Pass an implementation of ConstraintInterface (remains in your ownership).
     * actions() will then only suggest to assign tags for which matches() is true
     */
    void setConstraintInterface(ConstraintInterface* const iface);
    ConstraintInterface* constraintInterface() const;

    /// Set the matching mode for the tag name
    void setNameMatchMode(NameMatchMode mode);
    NameMatchMode nameMatchMode() const;

    /// reset all settings to the default (no fragment, no actions)
    void reset();

    /// Returns the sorted list of suggested tagging actions, based on the above settings
    QList<TaggingAction> actions() const;

    /// Returns one single action, which is decided to be the presumably best action based on the settings.
    TaggingAction defaultTaggingAction() const;

    /// Returns the index of the default action in the list returned by generate()
    int indexOfDefaultAction() const;

    /// Returns the index of the last recent action in the list returned by actions()
    int indexOfLastRecentAction() const;

    /// Returns a string to be used in the UI for the given TaggingAction, interpreted in the context of the current settings
    QString suggestedUIString(const TaggingAction& action) const;

    static TaggingAction defaultTaggingAction(const QString& tagName, int parentTagId = 0);

private:

    /// @note disabled
    TaggingActionFactory(const TaggingActionFactory&)            = delete;
    TaggingActionFactory& operator=(const TaggingActionFactory&) = delete;

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
