#pragma once
#include <chrono>

namespace gserver {

inline int64_t std_get_time_us() {
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    return duration_ms.count();
}

}
