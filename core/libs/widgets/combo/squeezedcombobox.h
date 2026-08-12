/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-21
 * Description : a combo box with a width not depending of text
 *               content size
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2008 by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText:      2005 by Tom Albers <tomalbers at kde dot nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QComboBox>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * @class SqueezedComboBox
 *
 * This widget is a QComboBox, but then a little bit
 * different. It only shows the right part of the items
 * depending on de size of the widget. When it is not
 * possible to show the complete item, it will be shortened
 * and "..." will be prepended.
 */
class DIGIKAM_EXPORT SqueezedComboBox : public QComboBox
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param parent the parent widget
     * @param name the name to give to the widget
     */
    explicit SqueezedComboBox(QWidget* const parent = nullptr, const char* name = nullptr);

    /**
     * destructor
     */
    ~SqueezedComboBox()                                                    override;

    /**
     *
     * Returns true if the combobox contains the original (not-squeezed)
     * version of text.
     * @param text the original (not-squeezed) text to check for
     */
    bool contains(const QString& text)                               const;

    /**
     * Returns the index of the combobox if found the original (not-squeezed)
     * version of text.
     * @param text the original (not-squeezed) text to find for
     * @param cs case sensitive or case insensitive search
     */
    int findOriginalText(const QString& text,
                         Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    /**
     * This inserts a item to the list. See QComboBox::insertItem()
     * for details. Please do not use QComboBox::insertItem() to this
     * widget, as that will fail.
     * @param newItem the original (long version) of the item which needs
     *                to be added to the combobox
     * @param index the position in the widget.
     * @param userData custom meta-data assigned to new item.
     */
    void insertSqueezedItem(const QString& newItem, int index,
                            const QVariant& userData = QVariant());

    /**
     * This inserts items to the list. See QComboBox::insertItems()
     * for details. Please do not use QComboBox:: insertItems() to this
     * widget, as that will fail.
     * @param newItems the originals (long version) of the items which needs
     *                 to be added to the combobox
     * @param index the position in the widget.
     */
    void insertSqueezedList(const QStringList& newItems, int index);

    /**
     * Append an item.
     * @param newItem the original (long version) of the item which needs
     *                to be added to the combobox
     * @param userData custom meta-data assigned to new item.
     */
    void addSqueezedItem(const QString& newItem,
                         const QVariant& userData = QVariant());

    /**
     * Set the current item to the one matching the given text.
     *
     * @param itemText the original (long version) of the item text
     */
    void setCurrent(const QString& itemText);

    /**
     * This method returns the full text (not squeezed) of the currently
     * highlighted item.
     * @return full text of the highlighted item
     */
    QString itemHighlighted()                                        const;

    /**
     * This method returns the full text (not squeezed) for the index.
     * @param index the position in the widget.
     * @return full text of the item
     */
    QString item(int index)                                          const;

    /**
     * Sets the sizeHint() of this widget.
     */
    QSize sizeHint()                                                 const override;

Q_SIGNALS:

    void signalItemActivated(const QString&);

private Q_SLOTS:

    void slotTimeOut();
    void slotUpdateToolTip(int index);

private:

    void    resizeEvent(QResizeEvent*)                                     override;
    QString squeezeText(const QString& original)                     const;

    /// Prevent these from being used.
    QString currentText()                                            const;
    void    setCurrentText(const QString& itemText);
    void    insertItem(const QString& text);
    void    insertItem(qint32 index, const QString& text);
    void    insertItem(int index,
                       const QIcon& icon,
                       const QString& text,
                       const QVariant& userData = QVariant());
    void    insertItems(int index, const QStringList& list);
    void    addItem(const QString& text);
    void    addItem(const QIcon& icon,
                    const QString& text,
                    const QVariant& userData = QVariant());
    void    addItems(const QStringList& texts);
    QString itemText(int index)                                      const;
    int     findText(const QString& text,
                     Qt::MatchFlags flags = Qt::MatchExactly |
                                            Qt::MatchCaseSensitive)  const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
