/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-10-10
 * Description : Common class to provides convenient access to digiKam
 *               test data directories and files.
 *
 * SPDX-FileCopyrightText: 2022 Steve Robbins <steve at sumost dot ca>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDir>

/**
 * @brief Class that provides convenient access to digiKam test data directories and files.
 *
 * When instantiated, the test-data directory is located dynamically; this algorithm works as long as
 * the current directory is the source root directory or any sub-directory. After construction,
 * the function @ref isValid returns true if the test data was successfully located.
 */

class DTestDataDir
{
public:

    /**
     * @return true if the instance is correctly instantiated.
     * Valid means that the desired root directory was located.
     */
    bool isValid() const
    {
        return m_isValid;
    }

    /**
     * @return root directory of test data hierarchy.
     */
    QDir root() const
    {
        return m_testDataDir;
    }

    /**
     * @return path to any test file or directory, specified using relative path from root.
     */
    QString path(const QString& name) const
    {
        return root().filePath(name);
    }

    /**
     * @return any test directory, specified using relative path from root.
     */
    QDir dir(const QString& relPath) const
    {
        return QDir(path(relPath));
    }

    /**
     * @return any test file, specified using relative path from root.
     */
    QFile file(const QString& name) const
    {
        return QFile(path(name));
    }

    /**
     * @return DTestDataDir for the digiKam Test Data root directory.
     * This provides access to all files in digiKam Test Data.
     */
    static DTestDataDir TestDataRoot()
    {
        return DTestDataDir();
    }

    /**
     * @return DTestDataDir for a sub-tree of the digiKam Test Data.
     * This provides access to files in the subtree.
     *
     * @param subdirPath path of subdir, relative to the digiKam Test Data root.
     */
    static DTestDataDir TestData(const QString& subdirPath)
    {
        return DTestDataDir(subdirPath);
    }

private:

    QDir m_testDataDir;
    bool m_isValid = false;

private:

    static bool s_findDirectoryUpwards(const QDir& directory, const QString& target, QDir& result)
    {
        QDir _dir = directory;

        while (_dir.exists(target) == false)
        {
            if (!_dir.cdUp())
            {
                return false;
            }
        }

        if (!_dir.cd(target))
        {
            return false;
        }

        result = _dir;

        return true;
    }

    void initialize()
    {
        m_isValid = s_findDirectoryUpwards(QDir(), QString::fromUtf8("test-data"), m_testDataDir);
    }

protected:

    /**
     * @brief Constructor with internal instance creation.
     */
    DTestDataDir()
    {
        initialize();
    }

    explicit DTestDataDir(const QString& subdirPath)
    {
        initialize();

        if (!m_isValid)
        {
            return;
        }

        m_isValid = m_testDataDir.exists(subdirPath);

        if (!m_isValid)
        {
            return;
        }

        bool b = m_testDataDir.cd(subdirPath);

        Q_UNUSED(b);
    }
};
