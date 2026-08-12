/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007      by Matthias Kretz <kretz at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "dconfigdlgwidgets.h"
#include "dconfigdlgview_p.h"

// Qt includes

#include <QLabel>
#include <QApplication>

namespace Digikam
{

class DConfigDlgWdgModel;

class Q_DECL_HIDDEN DConfigDlgWdgPrivate : public DConfigDlgViewPrivate
{
    Q_DECLARE_PUBLIC(DConfigDlgWdg)

protected:

    explicit DConfigDlgWdgPrivate(DConfigDlgWdg* const q);

    DConfigDlgWdgModel* model() const
    {
        return static_cast<DConfigDlgWdgModel*>(DConfigDlgViewPrivate::model);
    }

    void _k_slotCurrentPageChanged(const QModelIndex&, const QModelIndex&);
};

// -----------------------------------------------------------------------------------

class Q_DECL_HIDDEN DConfigDlgTitle::Private
{
public:

    explicit Private(DConfigDlgTitle* const parent)
        : q(parent)
    {
    }

    QString textStyleSheet() const
    {
        const int fontSize = qRound(QApplication::font().pointSize() * 1.4);

        return (QString::fromLatin1("QLabel { font-size: %1pt; color: %2 }")
                .arg(fontSize).arg(q->palette().color(QPalette::WindowText).name()));
    }

    QString commentStyleSheet() const
    {
        QString styleSheet;

        switch (messageType)
        {
            // FIXME: we need the usability color styles to implement different
            //        yet palette appropriate colours for the different use cases!
            //        also .. should we include an icon here,
            //        perhaps using the imageLabel?

            case InfoMessage:
            case WarningMessage:
            case ErrorMessage:
            {
                styleSheet = QString::fromLatin1("QLabel { color: palette(%1); background: palette(%2); }")
                             .arg(q->palette().color(QPalette::HighlightedText).name())
                             .arg(q->palette().color(QPalette::Highlight).name());
                break;
            }

            case PlainMessage:
            default:
            {
                break;
            }
        }

        return styleSheet;
    }

    /**
     * @brief Get the icon name from the icon type
     * @param type icon type from the enum
     * @return named icon as QString
     */
    QString iconTypeToIconName(DConfigDlgTitle::MessageType type)
    {
        switch (type)
        {
            case DConfigDlgTitle::InfoMessage:
            {
                return QLatin1String("dialog-information");
            }

            case DConfigDlgTitle::ErrorMessage:
            {
                return QLatin1String("dialog-error");
            }

            case DConfigDlgTitle::WarningMessage:
            {
                return QLatin1String("dialog-warning");
            }

            case DConfigDlgTitle::PlainMessage:
            {
                break;
            }
        }

        return QString();
    }

    void _k_timeoutFinished()
    {
        q->setVisible(false);
    }

public:

    DConfigDlgTitle* q                  = nullptr;
    QGridLayout*     headerLayout       = nullptr;
    QLabel*          imageLabel         = nullptr;
    QLabel*          textLabel          = nullptr;
    QLabel*          commentLabel       = nullptr;
    int              autoHideTimeout    = 0;
    MessageType      messageType        = InfoMessage;
};

} // namespace Digikam
