#pragma once

/*!
 * \file gdiplus.h
 *
 * \author musicmusic
 * \date March 2015
 *
 * Contains helper functions for Gdiplus
 */

#include "stdafx.h"

#include "wic.h"

namespace cui::gdip {

/**
 * Creates a GDI+ Bitmap from an array of bytes, without needing to keep the data around once
 * the Bitmap instance has been created.
 */
std::unique_ptr<Gdiplus::Bitmap> create_bitmap_from_32bpp_data(
    unsigned width, unsigned height, unsigned stride, const uint8_t* data, size_t size);

std::unique_ptr<Gdiplus::Bitmap> create_bitmap_from_wic_data(const wic::BitmapData& bitmap_data);

void check_status(Gdiplus::Status status);

} // namespace cui::gdip
