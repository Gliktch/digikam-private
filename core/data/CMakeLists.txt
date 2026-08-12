#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_subdirectory(icons)
add_subdirectory(pics)
add_subdirectory(colorschemes)
add_subdirectory(database)
add_subdirectory(geolocation)
add_subdirectory(metadata)
add_subdirectory(profiles)
add_subdirectory(hotplug)
add_subdirectory(scripts)
add_subdirectory(filters)
add_subdirectory(dnnmodels)

if(WIN32)

    add_subdirectory(applications)

endif()
