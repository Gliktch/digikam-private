/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a token class
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QObject>
#include <QString>

class QAction;

namespace Digikam
{

/**
 * @brief %Token is the smallest parsing unit in AdvancedRename utility
 *
 * The %Token class represents the smallest parsing unit for the Parser class. Every string you enter as a renaming pattern
 * is a combination of tokens and literal text. For example
 * @code
 * "[file]{upper}_###_abc.[ext]{lower}"
 * @endcode
 * is composed of five tokens
 * @code
 * [file]
 * {upper}
 * ###
 * .[ext]
 * {lower}
 * @endcode
 * and two literals
 * @code
 * _
 * _abc
 * @endcode
 * A rule must assign at least one token object, to make parsing work. More than one token can be assigned to a %Rule.
 * @see Rule::addToken()
 *
 */
class Token : public QObject
{
    Q_OBJECT

public:

    explicit Token(const QString& id, const QString& description);
    ~Token() override;

    /**
     * @return The ID of the token. This is the actual token string, for example
     * @code
     * "[file]"
     * @endcode
     * This id will be emitted as a signal by slotTriggered().
     */
    QString id()            const;

    /**
     * @return The description of the token. It can be used for example in the tooltip of the AdvancedRenameWidget.
     */
    QString description()   const;

    /**
     * @return The action of the token. This action can be connected to a button or menu item. If triggered, high-level classes
     * like AdvancedRenameWidget can connect to the signal and display the emitted text in the line edit input widget.
     */
    QAction* action()       const;

Q_SIGNALS:

    /**
     * This signal is emitted when the action of the token is triggered.
     */
    void signalTokenTriggered(const QString&);

private Q_SLOTS:

    /**
     * This slot will emit signalTokenTriggered() when the action of the token is triggered.
     */
    void slotTriggered();

private:

    /// @note disabled
    Token(QObject*)                = delete;
    Token(const Token&)            = delete;
    Token& operator=(const Token&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

typedef QList<Token*> TokenList;

} // namespace Digikam
