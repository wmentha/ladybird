/*
 * Copyright (c) 2022, Lucas Chollet <lucas.chollet@free.fr>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Error.h>
#include <AK/Function.h>
#include <AK/String.h>

namespace Web {

class FileRequest {
public:
    FileRequest(String path, ESCAPING Function<void(ErrorOr<i32>)> on_file_request_finish);

    String path() const;

    Function<void(ErrorOr<i32>)> on_file_request_finish;

private:
    String m_path {};
};

}
