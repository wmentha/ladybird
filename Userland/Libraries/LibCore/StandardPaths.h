/*
 * Copyright (c) 2020, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2023, Linus Groh <linusg@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Error.h>
#include <AK/Forward.h>

namespace Core {

class StandardPaths {
public:
    static String home_directory();
    static String desktop_directory();
    static String documents_directory();
    static String downloads_directory();
    static String music_directory();
    static String pictures_directory();
    static String videos_directory();
    static String tempfile_directory();
    static String config_directory();
    static String user_data_directory();
    static Vector<String> system_data_directories();
    static ErrorOr<String> runtime_directory();
    static ErrorOr<Vector<String>> font_directories();
};

}
