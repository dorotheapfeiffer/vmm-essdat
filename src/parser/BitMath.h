// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Small functions for bit- and byte- manipulations, graycode, reverse
/// bits
//===----------------------------------------------------------------------===//

#pragma once

//#include <cstdint>

class BitMath {
public:
  ///
  inline static uint32_t reversebits32(uint32_t x) {
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return ((x >> 16) | (x << 16));
  }

  /// \todo test this
  inline static uint16_t reversebits16(uint16_t x) {
    uint32_t temp = reversebits32(x);
    return (temp >> 16);
  }

  inline static uint32_t gray2bin32(uint32_t num) {
    num = num ^ (num >> 16);
    num = num ^ (num >> 8);
    num = num ^ (num >> 4);
    num = num ^ (num >> 2);
    num = num ^ (num >> 1);
    return num;
  }


  /// \todo this is a hack to allow compilation of code from
  /// ROOT using cling (variant of clang) without c++14 support.
  inline static
  uint64_t NextPowerOfTwo(uint64_t n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;
    return n;
  }
};
