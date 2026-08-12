/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-01-29
 * Description : Camera settings container.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QPointer>

class QAction;

namespace Digikam
{

class ImportUI;

class CameraType
{
public:

    CameraType();
    CameraType(const QString& title, const QString& model,
               const QString& port, const QString& path,
               int startingNumber, QAction* const action = nullptr);
    ~CameraType();

    CameraType(const CameraType& ctype);
    CameraType& operator=(const CameraType& type);

    void setTitle(const QString& _title);
    void setModel(const QString& _model);
    void setPort(const QString& _port);
    void setPath(const QString& _path);
    void setStartingNumber(int sn);
    void setAction(QAction* const _action);
    void setValid(bool _valid);
    void setCurrentImportUI(ImportUI* const importui);

    QString   title()           const;
    QString   model()           const;
    QString   port()            const;
    QString   path()            const;
    int       startingNumber()  const;
    QAction*  action()          const;
    bool      valid()           const;
    ImportUI* currentImportUI() const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
