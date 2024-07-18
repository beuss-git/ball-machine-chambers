#include "image.hpp"

bounds calculate_image_bounds(Image const& image)
{
    int minX = image.width;
    int maxX = 0;
    int minY = image.height;
    int maxY = 0;

    // Lambda to check if a pixel is non-transparent
    auto const is_pixel_visible = [&](int index) {
        int alpha = (image.data[index] >> 24) & 0xFF;
        return alpha != 0;
    };

    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            int index = y * image.width + x;
            if (is_pixel_visible(index)) {
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
        }
    }

    // Adjust the bounds to ensure the sprite is enclosed
    bounds result {};
    result.left = static_cast<float>(minX);
    result.right = static_cast<float>(maxX);
    result.top = static_cast<float>(minY);
    result.bottom = static_cast<float>(maxY);

    return result;
}

Image crop_and_resize_image(Image const& sourceImage, bounds const& b)
{
    // Calculate the new dimensions
    int new_width = static_cast<int>(b.right - b.left + 1);
    int new_height = static_cast<int>(b.bottom - b.top + 1);

    // Create a new image with the new dimensions
    Image cropped_image;
    cropped_image.width = new_width;
    cropped_image.height = new_height;
    cropped_image.data.resize(static_cast<size_t>(new_width) * static_cast<size_t>(new_height));

    // Copy the pixels from the source image to the new image
    for (auto y = static_cast<size_t>(b.top); y <= static_cast<size_t>(b.bottom); ++y) {
        for (auto x = static_cast<size_t>(b.left); x <= static_cast<size_t>(b.right); ++x) {
            size_t source_index = y * sourceImage.width + x;
            size_t dest_index = (y - static_cast<size_t>(b.top)) * new_width + (x - static_cast<size_t>(b.left));
            cropped_image.data[dest_index] = sourceImage.data[source_index];
        }
    }

    return cropped_image;
}

void draw_image(canvas_ity::canvas& ctx, Image const& img, int x, int y)
{
    ctx.draw_image(reinterpret_cast<unsigned char const*>(img.data.data()), img.width, img.height, img.width * 4, x - (img.width / 2.F), y - (img.height / 2.F), img.width, img.height);
}
