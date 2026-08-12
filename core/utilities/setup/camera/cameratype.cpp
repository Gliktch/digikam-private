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

#include "cameratype.h"

// Qt includes

#include <QAction>

// Local includes

#include "importui.h"

namespace Digikam
{

class Q_DECL_HIDDEN CameraType::Private
{
public:

    Private() = default;

public:

    bool               valid            = false;

    int                startingNumber   = 1;

    QString            title;
    QString            model;
    QString            port;
    QString            path;

    QAction*           action           = nullptr;

    QPointer<ImportUI> currentImportUI;
};

CameraType::CameraType()
    : d(new Private)
{
}

CameraType::CameraType(const QString& title, const QString& model,
                       const QString& port, const QString& path,
                       int startingNumber, QAction* const action)
    : d(new Private)
{
    setTitle(title);
    setModel(model);
    setPort(port);
    setPath(path);
    setStartingNumber(startingNumber);
    setAction(action);
    d->valid = true;
}

CameraType::~CameraType()
{
    delete d;
}

CameraType::CameraType(const CameraType& ctype)
    : d(new Private)
{
    d->title          = ctype.d->title;
    d->model          = ctype.d->model;
    d->port           = ctype.d->port;
    d->path           = ctype.d->path;
    d->startingNumber = ctype.d->startingNumber;
    d->action         = ctype.d->action;
    d->valid          = ctype.d->valid;
}

CameraType& CameraType::operator=(const CameraType& ctype)
{
    if (this != &ctype)
    {
        d->title          = ctype.d->title;
        d->model          = ctype.d->model;
        d->port           = ctype.d->port;
        d->path           = ctype.d->path;
        d->startingNumber = ctype.d->startingNumber;
        d->action         = ctype.d->action;
        d->valid          = ctype.d->valid;
    }

    return *this;
}

void CameraType::setTitle(const QString& _title)
{
    d->title = _title;
}

void CameraType::setModel(const QString& _model)
{
    d->model = _model;
}

void CameraType::setPort(const QString& _port)
{
    d->port = _port;
}

void CameraType::setPath(const QString& _path)
{
    d->path = _path;
}

void CameraType::setStartingNumber(int sn)
{
    d->startingNumber = sn;
}

void CameraType::setAction(QAction* const _action)
{
    d->action = _action;
}

void CameraType::setValid(bool _valid)
{
    d->valid = _valid;
}

void CameraType::setCurrentImportUI(ImportUI* const importui)
{
    d->currentImportUI = importui;
}

QString CameraType::title() const
{
    return d->title;
}

QString CameraType::model() const
{
    return d->model;
}

QString CameraType::port() const
{
    return d->port;
}

QString CameraType::path() const
{
    return d->path;
}

int CameraType::startingNumber() const
{
    return d->startingNumber;
}

QAction* CameraType::action() const
{
    return d->action;
}

bool CameraType::valid() const
{
    return d->valid;
}

ImportUI* CameraType::currentImportUI() const
{
    return d->currentImportUI;
}

} // namespace Digikam
