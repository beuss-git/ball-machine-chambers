#ifndef IMAGE_HPP
#define IMAGE_HPP
#include "math.hpp"
#include <canvas_ity/canvas_ity.hpp>
#include <vector>

struct Image {
    int width;
    int height;
    std::vector<int32_t> data;
};

bounds calculate_image_bounds(Image const& image);

Image crop_and_resize_image(Image const& sourceImage, bounds const& b);

void draw_image(canvas_ity::canvas& ctx, Image const& img, int x, int y);

void draw_image(canvas_ity::canvas& ctx, uint32_t const* image_data, int width, int height, int x, int y);

#endif // IMAGE_HPP
