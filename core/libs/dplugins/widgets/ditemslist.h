/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-21
 * Description : widget to display a list of items
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2010 by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Luka Renko <lure at kubuntu dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QTreeWidget>
#include <QWidget>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QIcon>
#include <QUrl>

// Local includes

#include "loadingdescription.h"
#include "digikam_export.h"
#include "dinfointerface.h"

namespace Digikam
{

class DItemsList;
class DItemsListView;

/**
 * Type of static function used to customize sort items in list.
 * Sort items call this method in DItemsListViewItem::operator<.
 * To setup this method, uses DItemList::setIsLessThanHandler().
 */
typedef bool (*DItemsListIsLessThanHandler)(const QTreeWidgetItem* current, const QTreeWidgetItem& other);

class DIGIKAM_EXPORT DItemsListViewItem : public QTreeWidgetItem
{

public:

    enum State
    {
        Waiting,
        Success,
        Failed
    };

public:

    explicit DItemsListViewItem(DItemsListView* const view, const QUrl& url);
    ~DItemsListViewItem()             override;

    bool hasValidThumbnail()    const;

    void setUrl(const QUrl& url);
    QUrl url()                  const;

    void setComments(const QString& comments);
    QString comments()          const;

    void setTags(const QStringList& tags);
    QStringList tags()          const;

    void setRating(int rating);
    int rating()                const;

    void setThumb(const QPixmap& pix, bool hasThumb=true);
    void setProgressAnimation(const QPixmap& pix);

    void setProcessedIcon(const QIcon& icon);
    void setState(State state);
    State state()               const;

    void updateInformation();

    void setIsLessThanHandler(DItemsListIsLessThanHandler fncptr);

    /**
     * Implement this, if you have special item widgets, e.g. an edit line
     * they will be set automatically when adding items, changing order, etc.
     */
    virtual void updateItemWidgets() {};

protected:

    DItemsListView* view()      const;

private:

    void setPixmap(const QPixmap& pix);
    bool operator<(const QTreeWidgetItem& other) const override;

private:

    class Private;
    Private* const d = nullptr;

private:

    Q_DISABLE_COPY(DItemsListViewItem)
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DItemsListView : public QTreeWidget
{
    Q_OBJECT

public:

    enum ColumnType
    {
        Thumbnail = 0,
        Filename,
        User1,
        User2,
        User3,
        User4,
        User5,
        User6
    };

public:

    explicit DItemsListView(DItemsList* const parent);
    ~DItemsListView()                                     override = default;

    void setColumnLabel(ColumnType column, const QString& label);
    void setColumnEnabled(ColumnType column, bool enable);
    void setColumn(ColumnType column, const QString& label, bool enable);

    DItemsListViewItem* findItem(const QUrl& url);
    QModelIndex indexFromItem(DItemsListViewItem* item,
                              int column = 0)       const;

    DItemsListViewItem* getCurrentItem()            const;

    DInfoInterface* iface()                         const;
    DItemsListIsLessThanHandler isLessThanHandler() const;

Q_SIGNALS:

    void signalAddedDropedItems(const QList<QUrl>&);
    void signalItemClicked(QTreeWidgetItem*);
    void signalContextMenuRequested();

private Q_SLOTS:

    void slotItemClicked(QTreeWidgetItem* item, int column);

public:

    void enableDragAndDrop(const bool enable = true);

private:

    void dragEnterEvent(QDragEnterEvent* e)               override;
    void dragMoveEvent(QDragMoveEvent* e)                 override;
    void dropEvent(QDropEvent* e)                         override;
    void contextMenuEvent(QContextMenuEvent* e)           override;

    void drawRow(QPainter* p,
                 const QStyleOptionViewItem& opt,
                 const QModelIndex& index)          const override;

private:

    QTreeWidgetItem* m_itemDraged = nullptr;
};

// -------------------------------------------------------------------------

class DIGIKAM_EXPORT DItemsList : public QWidget
{
    Q_OBJECT

public:

    enum ControlButtonPlacement
    {
        NoControlButtons = 0,
        ControlButtonsLeft,
        ControlButtonsRight,
        ControlButtonsAbove,
        ControlButtonsBelow
    };

    enum ControlButton
    {
       Add       = 0x1,
       Remove    = 0x2,
       MoveUp    = 0x4,
       MoveDown  = 0x8,
       Clear     = 0x10,
       Load      = 0x20,
       Save      = 0x40
    };
    Q_DECLARE_FLAGS(ControlButtons, ControlButton)

public:

    explicit DItemsList(QWidget* const parent);
    ~DItemsList()                                                     override;

    void                setAllowRAW(bool allow);
    void                setAllowDuplicate(bool allow);

    void                loadImagesFromCurrentSelection();

    /**
     * A function to load all the images from the album if no image has been selected by user.
     */
    void                loadImagesFromCurrentAlbum();

    /**
     * a function to check whether an image has been selected or not.
     */
    bool                checkSelection();

    void setIconSize(int size);
    int                 iconSize()                                  const;

    DItemsListView*     listView()                                  const;

    void                processing(const QUrl& url);
    void                processed(const QUrl& url, bool success);
    void                cancelProcess();
    void                clearProcessedStatus();

    void                setControlButtons(ControlButtons buttonMask);

    /**
     * Plug the control buttons near to the list, following 'placement' position.
     * Return the instance of the layout supporting the control buttons, if any.
     * This method must be calls after to use appendControlButtonsWidget().
     */
    QBoxLayout*         setControlButtonsPlacement(ControlButtonPlacement placement);

   /**
    * Append a extra widget to the end of Control Button layout (as a progress bar for example).
    * This method must be call before setControlButtonsPlacement().
    * Ownership of the widget is not transferred to the DItemList.
    */
    void                appendControlButtonsWidget(QWidget* const widget);

    void                enableControlButtons(bool enable = true);
    void                enableDragAndDrop(const bool enable = true);

    void                updateThumbnail(const QUrl& url);

    virtual QList<QUrl> imageUrls(bool onlyUnprocessed = false)     const;
    virtual void        removeItemByUrl(const QUrl& url);

    void                setCurrentUrl(const QUrl& url);
    QUrl                getCurrentUrl()                             const;

    void setIface(DInfoInterface* const iface);
    DInfoInterface* iface()                                         const;

    ///@{
    /**
     * Methods to handle function pointer used to customize sort items in list.
     * See DItemsListIsLessThanHandler type for details.
     */
    void setIsLessThanHandler(DItemsListIsLessThanHandler fncptr);
    DItemsListIsLessThanHandler isLessThanHandler() const;
    ///@}

Q_SIGNALS:

    void signalAddItems(const QList<QUrl>&);
    void signalMoveUpItem();
    void signalMoveDownItem();
    void signalRemovedItems(const QList<int>&);
    void signalImageListChanged();
    void signalFoundRAWImages(bool);
    void signalItemClicked(QTreeWidgetItem*);
    void signalContextMenuRequested();
    void signalXMLSaveItem(QXmlStreamWriter&, int);         // clazy:exclude=signal-with-return-value
    void signalXMLLoadImageElement(QXmlStreamReader&);      // clazy:exclude=signal-with-return-value
    void signalXMLCustomElementsToWrite(QXmlStreamWriter&); // clazy:exclude=signal-with-return-value
    void signalXMLCustomElementsToRead(QXmlStreamReader&);  // clazy:exclude=signal-with-return-value

public Q_SLOTS:

    virtual void slotAddImages(const QList<QUrl>& list);
    virtual void slotRemoveItems();

protected Q_SLOTS:

    void slotProgressTimerDone();

    virtual void slotAddItems();
    virtual void slotMoveUpItems();
    virtual void slotMoveDownItems();
    virtual void slotClearItems();
    virtual void slotLoadItems();
    virtual void slotSaveItems();
    virtual void slotThumbnail(const LoadingDescription&, const QPixmap&);      // cppcheck-suppress virtualCallInConstructor
    virtual void slotImageListChanged();

private:

    /// @note disabled
    DItemsList() = delete;

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::DItemsList::ControlButtons)
