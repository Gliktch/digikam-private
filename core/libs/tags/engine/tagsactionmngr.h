/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QWidget>
#include <QMap>

class KActionCollection;

namespace Digikam
{

class Album;
class ImageTagChangeset;

class TagsActionMngr : public QObject
{
    Q_OBJECT

public:

    explicit TagsActionMngr(QWidget* const parent);
    ~TagsActionMngr() override;

    /**
     * Register all tag actions to collections managed with keyboard shortcuts.
     * Because Tags shortcuts are stored in database this method must be called after
     * database initialization and after that all root window instances have been created.
     */
    void registerTagsActionCollections();

    /**
     * Register all labels actions to collections managed with keyboard shortcuts.
     * Unlike tags actions, labels shortcuts are stored in XML GUI file of each root windows,
     * to be able to customize it through KDE keyboards shortcuts config panel.
     * This method must be called before to DXmlGuiWindow::createGUI(), typically
     * when window actions are registered to ActionCollection instance.
     */
    void registerLabelsActions(KActionCollection* const ac);

    void registerActionsToWidget(QWidget* const wdg);

    /**
     * Return the list of whole action collections managed.
     */
    QList<KActionCollection*> actionCollections()   const;

    /**
     * Updates the shortcut action for a tag. Call this when a shortcut was
     * added, removed or changed.
     */
    void updateTagShortcut(int tagId, const QKeySequence& ks, bool delAction = true);

    QString ratingShortcutPrefix()                  const;
    QString tagShortcutPrefix()                     const;
    QString pickShortcutPrefix()                    const;
    QString colorShortcutPrefix()                   const;

    /**
     * @brief return the current map of color label names ordered by ColorLabel enum.
     */
    QMap<int, QString> colorLabelNames()            const;

    static QStringList colorSet();

public:

    static TagsActionMngr* defaultManager();

Q_SIGNALS:

    void signalShortcutPressed(const QString& shortcut, int val);
    void signalTagsActionCollectionsRegistered();
    void signalColorLabelNamesUpdated();

public Q_SLOTS:

    void slotColorNameChanged(int label, const QString& name);

private Q_SLOTS:

    /**
     * Removes the shortcut actions associated with a tag.
     */
    void slotAlbumDeleted(Album*);

    /**
     * Wrapper around windows to run relevant code about keyboard shortcuts in GUI.
     */
    void slotAssignFromShortcut();

    /**
     * Called by config shortcuts dialog, when user change action properties.
     */
    void slotTagActionChanged();

private:

    bool createTagActionShortcut(int tagId);
    bool removeTagActionShortcut(int tagId, bool delAction = true);

    bool createRatingActionShortcut(KActionCollection* const ac, int rating);
    bool createPickLabelActionShortcut(KActionCollection* const ac, int pickId);
    bool createColorLabelActionShortcut(KActionCollection* const ac, int colorId);

    void loadColorNames();
    void saveColorNames();

private:

    static TagsActionMngr* m_defaultManager;

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
