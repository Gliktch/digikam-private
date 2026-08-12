/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-09-24
 * Description : a media server to export collections through DLNA.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmediaserver.h"

// Qt includes

#include <QApplication>
#include <QFile>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "daboutdata.h"
#include "dlnaserver.h"

// Platinum includes

#include "PltDeviceHost.h"
#include "Platinum.h"

using namespace Digikam;

void NPT_Console::Output(const char* msg)
{
    qCDebug(DIGIKAM_MEDIASRV_LOG) << msg;
}

void UPnPLogger(const NPT_LogRecord* record)
{
    QString msg = QLatin1String("Platinum [")             +
                  QString::fromUtf8(record->m_LoggerName) +
                  QLatin1String("] : ")                   +
                  QString::fromUtf8(record->m_Message);

    switch (record->m_Level)
    {
        case NPT_LOG_LEVEL_FATAL:
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG_FATAL) << msg;
            break;
        }

        case NPT_LOG_LEVEL_SEVERE:
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG_CRITICAL) << msg;
            break;
        }

        case NPT_LOG_LEVEL_WARNING:
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG_WARN) << msg;
            break;
        }

        case NPT_LOG_LEVEL_INFO:
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG_INFO) << msg;
            break;
        }

        case NPT_LOG_LEVEL_FINE:
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG) << msg;
            break;
        }

        default: // NPT_LOG_LEVEL_DEBUG:
        {
            qCDebug(DIGIKAM_MEDIASRV_LOG_DEBUG) << msg;
            break;
        }
    }
}

namespace DigikamGenericMediaServerPlugin
{

class Q_DECL_HIDDEN CDeviceHostReferenceHolder
{
public:

    PLT_DeviceHostReference m_device;
};

class Q_DECL_HIDDEN DMediaServer::Private
{
public:

    Private()
    {
        NPT_LogManager::GetDefault().Configure("plist:.level=INFO;.handlers=CustomHandler;");
        NPT_LogHandler::Create("digiKam", "CustomHandler", logHandler);
        logHandler->SetCustomHandlerFunction(&UPnPLogger);
    }

public:

    PLT_UPnP*                   upnp            = nullptr;
    NPT_LogHandler*             logHandler      = nullptr;
    CDeviceHostReferenceHolder* serverHolder    = nullptr;
};

DMediaServer::DMediaServer(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->serverHolder = new CDeviceHostReferenceHolder();
    d->upnp         = new PLT_UPnP();
    d->upnp->Start();
}

DMediaServer::~DMediaServer()
{
    d->upnp->Stop();
    d->upnp->RemoveDevice(d->serverHolder->m_device);

    delete d->upnp;
    delete d->logHandler;
    delete d->serverHolder;
    delete d;
}

bool DMediaServer::init(int port)
{
    QString devDesc               = i18n("%1 Media Server", qApp->applicationName());

    DLNAMediaServer* const device = new DLNAMediaServer(devDesc.toUtf8().data(),
                                                        false,
                                                        nullptr,
                                                        port);

    device->m_ModelName           = "digiKam";
    device->m_ModelNumber         = digikam_version;
    device->m_ModelDescription    = NPT_String(DAboutData::digiKamSlogan().toUtf8().data());
    device->m_ModelURL            = NPT_String(DAboutData::webProjectUrl().toString().toUtf8().data());
    device->m_Manufacturer        = NPT_String("digiKam.org");
    device->m_ManufacturerURL     = NPT_String(DAboutData::webProjectUrl().toString().toUtf8().data());
    device->SetDelegate(device);

    d->serverHolder->m_device     = device;

    NPT_Result res                = d->upnp->AddDevice(d->serverHolder->m_device);

    qCDebug(DIGIKAM_MEDIASRV_LOG) << "Upnp device created:" << res;

    return true;
}

void DMediaServer::addAlbumsOnServer(const MediaServerMap& map)
{
    static_cast<DLNAMediaServer*>(d->serverHolder->m_device.AsPointer())->addAlbumsOnServer(map);
}

} // namespace DigikamGenericMediaServerPlugin

#include "moc_dmediaserver.cpp"
