/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-10-13
 * Description : digiKam deep neural network Model Manager
 *
 * SPDX-FileCopyrightText: 2024 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QSettings>

// Local includes

#include "digikam_export.h"
#include "filesdownloader.h"
#include "dnnmodelbase.h"
#include "dnnmodeldefinitions.h"

namespace Digikam
{

class DIGIKAM_EXPORT DNNModelManager : public QObject
{
    Q_OBJECT

public:

    DNNModelManager();
    ~DNNModelManager() override;

    /**
     * @brief Global instance of internal model manager.
     * All accessor methods are thread-safe.
     */
    static DNNModelManager* instance();

    /**
     * @brief Used by the filesdownload to get a stream containing the files and information to download.
     */
    const QList<DownloadInfo>& getDownloadInformation(DNNModelUsage usage);

    /**
     * @brief Get a list of models that can be used for a specific usage.
     */
    const QList<QPair<QString, QStringList> > getModelList(DNNModelUsage usage);

    /**
     * @brief Retrieve a DNNModelBase pointer by name.
     * This will load and create the model on first use.
     * It will also find the best OpenCV Target and Backend for the model
     * based on computer capabilities.
     * @return nullptr if 'modelName' cannot be found.
     */
    DNNModelBase* getModel(const QString& modelName, DNNModelUsage usage) const;

private:

    /// @note disabled
    explicit DNNModelManager(QObject*) = delete;

private:

    const std::map<std::string, DNNLoaderType> str2loader
    {
        { "net",    DNNLoaderType::DNNLoaderNet    },
        { "config", DNNLoaderType::DNNLoaderConfig },
        { "yunet",  DNNLoaderType::DNNLoaderYuNet  },
        { "sface",  DNNLoaderType::DNNLoaderSFace  }
    };

    /**
     * @brief Read the configuration file. The configuration is a .ini-style .conf hardcoded in dnnmodelmanager.cpp and
     * bundled with digiKam, but could be downloaded from the Internet.
     */
    void loadConfig();      ///< Load the Model map, but don't create the cv::dnn::Net objects yet.
    void getSettings();     ///< Loads the settings from disk (or web: future).

    /**
     * @brief Retrieve display strings for a model.
     * The QStringLiteral must match the [model_section] in dnnmodels.conf.
     * @return QStringList containing the display name and tooltip.
     */
    QStringList getDisplayStrings(const QString& modelName) const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
