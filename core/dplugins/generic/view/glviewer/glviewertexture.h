/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-11
 * Description : a tool to show image using an OpenGL interface.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Markus Leuthold <kusi at forum dot titlis dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QImage>
#include <QString>

// Local includes

#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericGLViewerPlugin
{

class GLViewerTexture : public QOpenGLTexture
{

public:

    explicit GLViewerTexture(DInfoInterface* const iface, QWidget* const display);
    virtual ~GLViewerTexture();

    bool load(const QString& fn, const QSize& size);
    bool load(const QImage& im);
    bool loadFullSize();

    GLfloat vertex_bottom() const;
    GLfloat vertex_top()    const;
    GLfloat vertex_left()   const;
    GLfloat vertex_right()  const;

    void setViewport(int w, int h);
    void zoom(float delta, const QPoint& mousepos);
    void reset(bool resetFullImage = false);
    void move(const QPoint& diff);
    bool setNewSize(QSize size);
    void rotate();
    void zoomToOriginal();

private:

    bool loadInternal();
    void calcVertex();

private:

    /// @note disabled
    GLViewerTexture(const GLViewerTexture&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericGLViewerPlugin
