/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : calendar parameters.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2012      by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QMap>
#include <QDate>
#include <QFont>
#include <QPair>
#include <QColor>
#include <QObject>
#include <QString>
#include <QPointer>
#include <QPrinter>

// Local includes

#include "digikam_config.h"
#include "calsystem.h"

namespace DigikamGenericCalendarPlugin
{

typedef QPair<QColor, QString> Day;

struct CalParams
{
public:

    enum ItemPosition
    {
        Top = 0,
        Left,
        Right
    };

public:

    QFont                 baseFont;
    QPageSize::PageSizeId pageSize          = QPageSize::A4;
    QPrinter::PrinterMode printResolution   = QPrinter::HighResolution;
    int                   paperWidth        = 210;
    int                   paperHeight       = 297;
    int                   width             = 0;
    int                   height            = 0;
    float                 ratio             = 0.0;
    bool                  drawLines         = false;
    ItemPosition          imgPos            = CalParams::Top;
    int                   year              = CalSystem().earliestValidDate().year() + 1;
};

// ---------------------------------------------------------------------------

class CalSettings : public QObject
{
    Q_OBJECT

public:

    ~CalSettings()                                            override;

    void setYear(int year);
    int  year() const;
    void setImage(int month, const QUrl& url);
    QUrl image(int month) const;
    void clearSpecial();
    void addSpecial(const QDate& date, const Day& info);
    bool isSpecial(int month, int day)                  const;

    QColor getDayColor(int month, int day)              const;
    QString getDayDescr(int month, int day)             const;
    QPrinter::PrinterMode resolution()                  const;

#ifdef HAVE_KCALENDAR

    void loadSpecial(const QUrl& url, const QColor& color);

#endif

    static CalSettings* instance(QObject* const parent = nullptr);

public:

    CalParams params;

Q_SIGNALS:

    void settingsChanged();

public Q_SLOTS:

    void setPaperSize(const QString& paperSize);
    void setResolution(const QString& resolution);
    void setImagePos(int pos);
    void setDrawLines(bool draw);
    void setRatio(int ratio);
    void setFont(const QString& font);

protected:

    bool isPrayDay(const QDate& date) const;

private:

    /// @note disabled
    explicit CalSettings(QObject* const parent);
    CalSettings(CalSettings const&)             = delete;
    CalSettings& operator=(CalSettings const&)  = delete;

private:

    static QPointer<CalSettings> s_instance;

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
