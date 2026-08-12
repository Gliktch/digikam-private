/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-19
 * Description : digiKam plugin definition for DImg image loader.
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFileInfo>
#include <QWidget>

// Local includes

#include "dplugin.h"
#include "dimgloader.h"
#include "dimgloadersettings.h"
#include "dpluginloader.h"
#include "digikam_export.h"
#include "drawdecoding.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginDImg : public DPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor with optional parent object
     */
    explicit DPluginDImg(QObject* const parent = nullptr);

    /**
     * Destructor
     */
    ~DPluginDImg()                                                                     override = default;

public:

    /**
     * This kind of plugin only provide one tool.
     */
    int count()                                                                  const override
    {
        return 1;
    };

    /**
     * This kind of plugin do not use a category.
     */
    QStringList categories()                                                     const override
    {
        return QStringList();
    };

    /**
     * This kind of plugin do not have GUI visibility attribute.
     */
    void setVisible(bool)                                                              override
    {
    };

    /**
     * Return the plugin interface identifier.
     */
    QString ifaceIid()                                                           const override
    {
        return QLatin1String(DIGIKAM_DPLUGIN_DIMG_IID);
    };

    /**
     * This kind of plugin do not need to be configurable
     */
    bool hasVisibilityProperty()                                                 const override
    {
        return false;
    };

    /**
     * With this kind of plugin, we will display the type-mimes list on about dialog.
     */
    QMap<QString, QStringList> extraAboutData()                                  const override;
    QStringList extraAboutDataRowTitles()                                        const override;
    QString extraAboutDataTitle()                                                const override;

public:

    /**
     * Return a single capitalized word to identify the format supported by the loader.
     * Ex: jpeg => "JPG" ; tiff => "TIF", etc.
     */
    virtual QString loaderName()                                                 const = 0;

    /**
     * Return the list of white-listed type-mimes supported by the loader,
     * as a string of file-name suffix separated by spaces.
     * Ex: "jpeg jpg thm"
     */
    virtual QString typeMimes()                                                  const = 0;

    /**
     * Return true if the loader can read a preview image.
     */
    virtual bool previewSupported() const { return false; };

    /**
     * Return > 0 if source file path is supported by the loader and contents can be loaded.
     * The return value (1 - 100) is a priority.
     * digiKam default loaders have a priority of 10, the
     * QImage loader has a priority of 80 and the
     * ImageMagick loader has a priority of 90.
     * If the loader is to be used before the default loader,
     * the value must be less than 10.
     */
    virtual int canRead(const QFileInfo& fileInfo, bool magic)                   const = 0;

    /**
     * Return > 0 if target file format is supported by the loader and contents can be written.
     * The return value (1 - 100) is a priority.
     */
    virtual int canWrite(const QString& format)                                  const = 0;

    /**
     * Return the image loader instance for the DImg instance.
     */
    virtual DImgLoader* loader(DImg* const image,
                               const DRawDecoding& rawSettings = DRawDecoding()) const = 0;

    /**
     * Return a new widget instance to show settings while exporting image to specified format.
     * Return nullptr if format is not supported or if no settings widget is available for this format.
     */
    virtual DImgLoaderSettings* exportWidget(const QString& format)              const = 0;
};

} // namespace Digikam

Q_DECLARE_INTERFACE(Digikam::DPluginDImg, DIGIKAM_DPLUGIN_DIMG_IID)
