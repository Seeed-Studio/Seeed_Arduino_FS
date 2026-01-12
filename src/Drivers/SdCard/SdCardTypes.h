#pragma once

#include <stdint.h>

namespace seeedfs
{

    enum class SdCardType : uint8_t
    {
        None,
        Mmc,
        Sd,
        Sdhc,
        Unknown,
    };

} // namespace seeedfs
