/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Completion Box for tags
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QCompleter>

// Local includes

#include "taggingaction.h"

namespace Digikam
{

class AlbumFilterModel;
class TagModel;

class TagCompleter : public QCompleter
{
    Q_OBJECT

public:

    /**
     * A completion object operating on a TagModel
     */
    explicit TagCompleter(QObject* const parent = nullptr);
    ~TagCompleter() override;

    /**
     * Update the completer for the given fragment
     */
    void update(const QString& fragment);

    /**
     * Set a parent tag which may by the user be considered as a parent for a new tag during completion
     */
    void setContextParentTag(int parentTagId);

    /**
     * Set a supporting model from which the completer may get data for its display. Optional.
     */
    void setSupportingTagModel(TagModel* const supportingModel);
    void setTagFilterModel(AlbumFilterModel* const supportingModel);

Q_SIGNALS:

    void signalActivated(const TaggingAction& action);
    void signalHighlighted(const TaggingAction& action);

private Q_SLOTS:

    void slotActivated(const QModelIndex& index);
    void slotHighlighted(const QModelIndex& index);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
