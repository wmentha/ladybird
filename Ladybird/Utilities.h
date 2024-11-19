/*
 * Copyright (c) 2022, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2023, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Error.h>
#include <AK/String.h>
#include <AK/Vector.h>

void platform_init();
void copy_default_config_files(StringView config_path);
ErrorOr<String> application_directory();
ErrorOr<Vector<String>> get_paths_for_helper_process(StringView process_name);

extern String s_ladybird_resource_root;
Optional<String const&> mach_server_name();
void set_mach_server_name(String name);
