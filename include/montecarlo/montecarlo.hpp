#pragma once
// One-stop header for the Monte Carlo bits
#include "core/concepts.hpp"
#include "core/result.hpp"
#include "core/transform.hpp"
#include "execution/sequential.hpp"
#ifdef MCLIB_PARALLEL_ENABLED
#include "execution/parallel.hpp"
#endif
#ifdef MCLIB_GPU_ENABLED
#include "execution/gpu.hpp"
#endif
#include "core/engine.hpp"
