#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(POLICY CMP0072)

    # Prefers GLVND (GL Vendor-Neutral Dispatch) instead legacy OpenGL by default when available.
    # Option was introduced in CMake version 3.11
    # Details: https://cmake.org/cmake/help/git-stage/policy/CMP0072.html

    cmake_policy(SET CMP0072 NEW)

endif()

find_package(OpenGL)                               # For Presentation tool and MediaPlayer.

message(STATUS "OpenGL found:                                 ${OpenGL_FOUND}")
message(STATUS "OpenGL GLU extension found:                   ${OPENGL_GLU_FOUND}")
message(STATUS "OpenGL GLX extension found:                   ${OpenGL_GLX_FOUND}")
message(STATUS "Qt OpenGL found:                              ${Qt${QT_VERSION_MAJOR}OpenGL_FOUND}")

if(Qt6_FOUND)

    message(STATUS "Qt6 OpenGLWidgets found:                      ${Qt${QT_VERSION_MAJOR}OpenGLWidgets_FOUND}")

endif()

# decide if Presentation tool can be built with OpenGL

if(OpenGL_FOUND AND OPENGL_GLU_FOUND AND OpenGL_GLX_FOUND AND Qt${QT_VERSION_MAJOR}OpenGL_FOUND)

    set(HAVE_OPENGL TRUE)

else()

    set(HAVE_OPENGL FALSE)

endif()

if(Qt6_FOUND AND Qt${QT_VERSION_MAJOR}OpenGLWidgets_FOUND)

    set(HAVE_OPENGL TRUE)

else()

    set(HAVE_OPENGL FALSE)

endif()

# For MediaPlayer compilation

MACRO_BOOL_TO_01(OpenGL_FOUND     HAVE_LIBOPENGL)
MACRO_BOOL_TO_01(OpenGL_EGL_FOUND HAVE_LIBOPENGL_EGL)
