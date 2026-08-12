/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : threaded image filter class.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "dynamicthread.h"
#include "filteraction.h"

class QObject;

namespace Digikam
{

class DIGIKAM_EXPORT DImgThreadedFilter : public DynamicThread
{
    Q_OBJECT

public:

    /**
     * @brief Constructs a filter without argument.
     * You need to call setupFilter() and startFilter()
     * to start the threaded computation.
     * To run filter without to use multithreading, call startFilterDirectly().
     */
    explicit DImgThreadedFilter(QObject* const parent = nullptr, const QString& name = QString());

    /**
     * @brief Constructs a filter with all arguments (ready to use).
     * The given original image will be copied.
     * You need to call startFilter() to start the threaded computation.
     * To run filter without to use multithreading, call startFilterDirectly().
     */
    DImgThreadedFilter(DImg* const orgImage,
                       QObject* const parent,
                       const QString& name = QString());

    ~DImgThreadedFilter() override;

    /**
     * @brief You need to call this and then start filter of you used
     * the constructor not setting an original image.
     * The original image's data will not be copied.
     */
    void setupFilter(const DImg& orgImage);

    /**
     * @brief Initializes the filter for use as a slave and directly starts computation (in-thread)
     */
    void setupAndStartDirectly(const DImg& orgImage,
                               DImgThreadedFilter* const master,
                               int progressBegin = 0,
                               int progressEnd = 100);

    void setOriginalImage(const DImg& orgImage);
    void setFilterName(const QString& name);

    DImg getTargetImage()
    {
        return m_destImage;
    };

    const QString& filterName()
    {
        return m_name;
    };

    /**
     * @brief This method return a list of steps to process parallelized operation in filter using QtConcurrents API.
     * Usually, start and stop are rows or columns from image to process. By default, whole image will be processed
     * and start value is 0. In this case stop will be last row or column to process.
     * Between range [start,stop], this method will divide by equal steps depending of number of CPU cores available.
     * To be sure that all values will be processed, in case of CPU core division give rest, the last step compensate
     * the difference.
     * See Blur filter loop implementation for example to see how to use this method with QtConcurrents API.
     */
    QList<int> multithreadedSteps(int stop, int start = 0)                      const;

    /**
     * @brief Start the threaded computation.
     */
    virtual void startFilter();

    /**
     * @brief Cancel the threaded computation.
     */
    virtual void cancelFilter();

    /**
     * @brief Start computation of this filter, directly in this thread.
     */
    virtual void startFilterDirectly();

    /**
     * @return The action description corresponding to currently set options.
     */
    virtual FilterAction filterAction()                                               = 0;

    virtual void readParameters(const FilterAction&)                                  = 0;

    /**
     * @return The identifier for this filter in the image history.
     */
    virtual QString filterIdentifier()                                          const = 0;

    virtual QList<int> supportedVersions()                                      const;

    /**
     *  @brief Replaying a filter action:
     *  Set the filter version. A filter may implement different versions, to preserve
     *  image history when the algorithm is changed.
     *  Any value set here must be contained in supportedVersions, otherwise
     *  this call will be ignored. Default value is 1.
     *
     *  @note If you intend to _record_ a filter action, please look at FilterAction's m_version
     */
    void setFilterVersion(int version);
    int filterVersion()                                                         const;

    /**
     * @brief Optional error handling for readParameters.
     * When readParameters() has been called, this method will return true
     * if the call was successful, and false if not.
     * If returning false, readParametersError() will give an error message.
     * The default implementation always returns success. You only need to reimplement
     * when a filter is likely to fail in a different environment, e.g.
     * depending on availability of installed files.
     * These methods have an undefined return value if readParameters() was not called
     * previously.
     */
    virtual bool parametersSuccessfullyRead()                                   const;
    virtual QString readParametersError(const FilterAction& actionThatFailed)   const;

Q_SIGNALS:

    /**
     * @brief This signal is emitted when image data is available and the computation has started.
     */
    void signalStarted();

    /**
     * @brief Emitted when progress info from the calculation is available.
     */
    void signalProgress(int progress);

    /**
     * @brief Emitted when the computation has completed.
     * @param success True if computation finished without interruption on valid data
     *                False if the thread was canceled, or no data is available.
     */
    void signalFinished(bool success);

protected:

    /**
     * @brief Start filter operation before threaded method. Must be called by your constructor.
     */
    virtual void initFilter();

    /**
     * @brief List of threaded operations by filter.
     */
    void run() override;

    /**
     * @brief Main image filter method. Override in subclass.
     */
    virtual void filterImage() = 0;

    /**
     * @brief Clean up filter data if necessary, called by stopComputation() method.
     * Override in subclass.
     */
    virtual void cleanupFilter() {};

    /**
     * @brief Emit progress info.
     */
    virtual void postProgress(int progress);

protected:

    /**
     * @brief Support for chaining two filters as master and thread.
     *
     * Do not call startFilter() or startFilterDirectly() on this.
     * The computation will be started from initFilter() which you must
     * call from the derived class constructor.
     *
     * Constructor for slave mode:
     * Constructs a new slave filter with the specified master.
     * The filter will be executed in the current thread.
     * orgImage and destImage will not be copied.
     * Note that the slave is still free to reallocate his destImage.
     * progressBegin and progressEnd can indicate the progress span
     * that the slave filter uses in the parent filter's progress.
     * Any derived filter class that is publicly available to other filters
     * should implement an additional constructor using this constructor.
     */
    DImgThreadedFilter(DImgThreadedFilter* const master,
                       const DImg& orgImage,
                       const DImg& destImage,
                       int progressBegin = 0,
                       int progressEnd = 100,
                       const QString& name = QString());

    /**
     * @brief Initialize the filter for use as a slave - reroutes progress info to master.
     * @note Computation will be started from setupFilter().
     */
    void initSlave(DImgThreadedFilter* const master,
                   int progressBegin = 0,
                   int progressEnd = 100);

    /**
     * @brief Inform the master that there is currently a slave. At destruction of the slave, call with slave=0.
     */
    void setSlave(DImgThreadedFilter* const slave);

    /**
     * @brief This method modulates the progress value from the 0..100 span to the span of this slave.
     * Called by postProgress if master is not null.
     */
    virtual int modulateProgress(int progress);

    void initMaster();
    virtual void prepareDestImage();

    /**
     * @brief Convenience class to spare the few repeating lines of code
     */
    template <class Filter>

    class DefaultFilterAction : public FilterAction
    {
    public:

        explicit DefaultFilterAction(FilterAction::Category category = FilterAction::ReproducibleFilter)
            : FilterAction(Filter::FilterIdentifier(), Filter::CurrentVersion(), category)
        {
            setDisplayableName(Filter::DisplayableName());
        }

        explicit DefaultFilterAction(bool isReproducible)
            : FilterAction(Filter::FilterIdentifier(), Filter::CurrentVersion(),
                           isReproducible ? FilterAction::ReproducibleFilter : FilterAction::ComplexFilter)
        {
            setDisplayableName(Filter::DisplayableName());
        }

        /**
         * @brief Preserve backwards compatibility
         * If a given condition (some new feature is not used) is true,
         * decrease the version so that older digikam versions can still replay the action
         */
        void supportOlderVersionIf(int version, bool condition)
        {
            if (condition && (version <= m_version))
            {
                m_version = version;
            }
        }
    };

protected:

    int                 m_version           = 1;

    bool                m_wasCancelled      = false;

    /**
     * @brief The progress span that a slave filter uses in the parent filter's progress.
     */
    int                 m_progressBegin     = 0;
    int                 m_progressSpan      = 0;
    int                 m_progressCurrent   = 0;  ///< To prevent signals bombarding with progress indicator value in postProgress().

    /**
     * @brief Filter name.
     */
    QString             m_name;

    /**
     * @brief Copy of original Image data.
     */
    DImg                m_orgImage;

    /**
     * @brief Output image data.
     */
    DImg                m_destImage;

    /**
     * @brief The current slave. Any filter might want to use another filter while processing.
     */
    DImgThreadedFilter* m_slave             = nullptr;

    /**
     * @brief The master of this slave filter. Progress info will be routed to this one.
     */
    DImgThreadedFilter* m_master            = nullptr;
};

} // namespace Digikam
