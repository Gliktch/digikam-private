/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-09-24
 * Description : a media server to export collections through DLNA.
 *               Implementation inspired on Platinum File Media Server.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QUrl>
#include <QList>
#include <QMap>
#include <QImage>
#include <QByteArray>
#include <QBuffer>

// Local includes

#include "digikam_debug.h"
#include "previewloadthread.h"
#include "dimg.h"
#include "drawdecoder.h"

// Local includes

#include "dmediaserver.h"

// Platinum includes

#include "Neptune.h"
#include "PltMediaServer.h"
#include "PltMediaCache.h"

using namespace Digikam;

namespace DigikamGenericMediaServerPlugin
{

/**
 * File Media Server Delegate for digiKam.
 * The DLNAMediaServerDelegate class is based on PLT_MediaServerDelegate
 * implementation for a file system backed Media Server.
 */
class DLNAMediaServerDelegate : public PLT_MediaServerDelegate
{
public:

    /// Constructor and destructor
    explicit DLNAMediaServerDelegate(const char* url_root,
                                     bool use_cache = false);
    ~DLNAMediaServerDelegate()                                                  override;

    /// Class methods
    static NPT_String BuildSafeResourceUri(const NPT_HttpUrl& base_uri,
                                           const char*        host,
                                           const char*        file_path);

    virtual void addAlbumsOnServer(const MediaServerMap& map);

protected:

    /// PLT_MediaServerDelegate methods
    NPT_Result OnBrowseMetadata(PLT_ActionReference&          action,
                                const char*                   object_id,
                                const char*                   filter,
                                NPT_UInt32                    starting_index,
                                NPT_UInt32                    requested_count,
                                const char*                   sort_criteria,
                                const PLT_HttpRequestContext& context)          override;

    NPT_Result OnBrowseDirectChildren(PLT_ActionReference&          action,
                                      const char*                   object_id,
                                      const char*                   filter,
                                      NPT_UInt32                    starting_index,
                                      NPT_UInt32                    requested_count,
                                      const char*                   sort_criteria,
                                      const PLT_HttpRequestContext& context)    override;

    NPT_Result OnSearchContainer(PLT_ActionReference&          action,
                                 const char*                   object_id,
                                 const char*                   search_criteria,
                                 const char*                   filter,
                                 NPT_UInt32                    starting_index,
                                 NPT_UInt32                    requested_count,
                                 const char*                   sort_criteria,
                                 const PLT_HttpRequestContext& context)         override;

    NPT_Result ProcessFileRequest(NPT_HttpRequest&              request,
                                  const NPT_HttpRequestContext& context,
                                  NPT_HttpResponse&             response)       override;

    /// Overridable methods
    virtual NPT_Result ExtractResourcePath(const NPT_HttpUrl& url,
                                           NPT_String& file_path);

    virtual NPT_String BuildResourceUri(const NPT_HttpUrl& base_uri,
                                        const char* host,
                                        const char* file_path);

    virtual NPT_Result ServeFile(const NPT_HttpRequest&        request,
                                 const NPT_HttpRequestContext& context,
                                 NPT_HttpResponse&             response,
                                 const NPT_String&             file_path);

    virtual NPT_Result GetFilePath(const char* object_id,
                                   NPT_String& filepath);

    virtual bool       ProcessFile(const NPT_String&,
                                   const char* filter = nullptr);

    virtual PLT_MediaObject* BuildFromFilePath(const NPT_String&             filepath,
                                               const PLT_HttpRequestContext& context,
                                               bool                          with_count = true,
                                               bool                          keep_extension_in_title = false,
                                               bool                          allip = false);

protected:

    friend class PLT_MediaItem;

    class Private;
    Private* const d = nullptr;

private:

    /// @note disabled
    DLNAMediaServerDelegate(const DLNAMediaServerDelegate&)            = delete;
    DLNAMediaServerDelegate& operator=(const DLNAMediaServerDelegate&) = delete;
};

} // namespace DigikamGenericMediaServerPlugin
