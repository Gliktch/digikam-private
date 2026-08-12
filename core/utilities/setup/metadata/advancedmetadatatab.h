/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-16
 * Description : Advanced Configuration tab for metadata.
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "dmetadatasettingscontainer.h"

class QStandardItemModel;
class QStandardItem;

namespace Digikam
{

class AdvancedMetadataTab : public QWidget
{
    Q_OBJECT

public:

    explicit AdvancedMetadataTab(QWidget* const parent = nullptr);
    ~AdvancedMetadataTab() override;

    void applySettings();

public Q_SLOTS:

    void slotSaveProfile();
    void slotLoadProfile();
    void slotResetToDefault();
    void slotRevertChanges();
    void slotAddNewNamespace();
    void slotEditNamespace();

private Q_SLOTS:

    void slotUnifyChecked(bool value);
    void slotAllTagsChecked(bool value);
    void slotIndexChanged();
    void slotRevertChangesAvailable();

private:

    /**
     * @brief The NsRoles enum will encode data from NamespaceEntry in
     *        model items, so we could retrieve and save it later
     */
    enum NsRoles
    {
        NAME_ROLE        = Qt::UserRole +  1,
        ISTAG_ROLE       = Qt::UserRole +  2,
        SEPARATOR_ROLE   = Qt::UserRole +  3,
        NSTYPE_ROLE      = Qt::UserRole +  5,

        ZEROSTAR_ROLE    = Qt::UserRole +  6,
        ONESTAR_ROLE     = Qt::UserRole +  7,
        TWOSTAR_ROLE     = Qt::UserRole +  8,
        THREESTAR_ROLE   = Qt::UserRole +  9,
        FOURSTAR_ROLE    = Qt::UserRole + 10,
        FIVESTAR_ROLE    = Qt::UserRole + 11,

        SPECIALOPTS_ROLE = Qt::UserRole + 12,

        ALTNAME_ROLE     = Qt::UserRole + 13,
        SUBSPACE_ROLE    = Qt::UserRole + 14,
        ALTNAMEOPTS_ROLE = Qt::UserRole + 15,

        ISDEFAULT_ROLE   = Qt::UserRole + 16
    };

private:

    void connectButtons();

    /**
     * @brief setModelData for one model
     * @param model - model to be populated
     * @param container - namespace container to get data
     */
    void setModelData(QStandardItemModel* const model, const QList<NamespaceEntry>& container);
    void setUi();

    void setDataToItem(QStandardItem* const item, const NamespaceEntry& entry);

    /**
     * @brief getModelIndex - the view can have up to 6 models
     *                        based on tags, comments, rating selection
     *                        and read/ write operation selected
     * @return              - return index of correct model in d->models
     */
    int getModelIndex();

    QList<NamespaceEntry>& getCurrentContainer();

    void setModels();

    void updateContainer();

    void saveModelData(QStandardItemModel* const model, QList<NamespaceEntry>& container);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
