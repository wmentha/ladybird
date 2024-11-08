/*
 * Copyright (c) 2018-2020, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2022, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/OwnPtr.h>
#include <AK/String.h>
#include <LibIPC/Forward.h>

namespace AK {
class BufferStream;
}

namespace IPC {

class Stub {
public:
    virtual ~Stub() = default;

    virtual u32 magic() const = 0;
    virtual String name() const = 0;
    virtual ErrorOr<OwnPtr<MessageBuffer>> handle(Message const&) = 0;

protected:
    Stub() = default;

private:
    String m_name;
};

}
