/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-02
 * Description : plugin to export image as wallpaper - Windows version.
 *
 * SPDX-FileCopyrightText: 2019      by Igor Antropov <antropovi at yahoo dot com>
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wallpaperplugin.h"

// Qt includes

#include <QMessageBox>
#include <QProcess>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

// Windows includes

#ifdef Q_OS_WIN
#   include <windows.h>
#   include <wininet.h>
#   include <shlobj.h>
#endif

namespace DigikamGenericWallpaperPlugin
{

bool s_checkErrorCode(HRESULT status, const QString& path, const QString& context)
{
    if (FAILED(status))
    {
        // Code to catch error string from error code with Windows API.

        LPWSTR bufPtr        = nullptr;
        DWORD werr           = GetLastError();
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_FROM_SYSTEM     |
                       FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr,
                        werr,
                       0,
                       (LPWSTR)&bufPtr,
                       0,
                       nullptr);

        // cppcheck-suppress knownConditionTrueFalse
        QString errStr = (bufPtr) ? QString::fromUtf16(reinterpret_cast<unsigned short*>(bufPtr)).trimmed()
                                  : i18n("Unknown Error %1", werr);
        LocalFree(bufPtr);

        QMessageBox::warning(nullptr,
                             i18nc("@title:window",
                                   "Error While to Set Image as Wallpaper"),
                             i18n("Cannot change wallpaper image from current desktop with\n"
                                  "%1\n\nContext: %2\n\nError: %3",
                                  path,
                                  context,
                                  errStr));

        return false;
    }

    return true;
}

bool WallpaperPlugin::setWallpaper(const QString& path, int layout) const
{
    // NOTE: IDesktopWallpaper is only defined with Windows >= 8.
    //       To be compatible with Windows 7, we needs to use IActiveDesktop instead.

    CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    IActiveDesktop* iADesktop = nullptr;
    HRESULT status            = CoCreateInstance(CLSID_ActiveDesktop,
                                                 nullptr,
                                                 CLSCTX_INPROC_SERVER,
                                                 IID_IActiveDesktop,
                                                 reinterpret_cast<void**>(&iADesktop));

    if (!s_checkErrorCode(status, path, i18n("Cannot create desktop context.")))
    {
        CoUninitialize();

        return false;
    }

    WALLPAPEROPT wOption;
    ZeroMemory(&wOption, sizeof(WALLPAPEROPT));
    wOption.dwSize            = sizeof(WALLPAPEROPT);

    switch (layout)
    {
        case Mosaic:
        {
            wOption.dwStyle = WPSTYLE_TILE;
            break;
        }

        case Centered:
        {
            wOption.dwStyle = WPSTYLE_CENTER;
            break;
        }

        default:    // *Adjusted*
        {
            wOption.dwStyle = WPSTYLE_STRETCH;
            break;
        }
    }

    status = iADesktop->SetWallpaper((PCWSTR)QDir::toNativeSeparators(path).utf16(), 0);

    if (!s_checkErrorCode(status, path, i18n("Cannot set wall paper image path.")))
    {
        iADesktop->Release();
        CoUninitialize();

        return false;
    }

    status = iADesktop->SetWallpaperOptions(&wOption, 0);

    if (!s_checkErrorCode(status, path, i18n("Cannot set wallpaper properties.")))
    {
        iADesktop->Release();
        CoUninitialize();

        return false;
    }

    status = iADesktop->ApplyChanges(AD_APPLY_ALL);

    if (!s_checkErrorCode(status, path, i18n("Cannot apply changes to desktop wallpaper.")))
    {
        iADesktop->Release();
        CoUninitialize();

        return false;
    }

    iADesktop->Release();
    CoUninitialize();

    return true;
}

} // namespace DigikamGenericWallpaperPlugin
