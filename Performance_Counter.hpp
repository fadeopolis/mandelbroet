///
/// high resolution timer
///

#pragma once

#include <cstdint>

namespace mandelbroet {

uint64_t read_performance_counter();

uint64_t performance_counter_frequency();

void wait(uint64_t milliseconds);

} /// end namespace mandelbroet
