#pragma once
#ifndef INC_PROMISE_HPP_
#define INC_PROMISE_HPP_

#ifdef __GNUC__
#include "promise_full.hpp"
#else
#include "promise_min.hpp"
#endif

#endif
