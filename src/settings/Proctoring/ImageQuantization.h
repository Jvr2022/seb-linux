#pragma once
namespace seb::settings::proctoring {
enum class ImageQuantization {
    BlackAndWhite1bpp,
    Grayscale2bpp,
    Grayscale4bpp,
    Grayscale8bpp,
    Color8bpp,
    Color16bpp,
    Color24bpp,
    Low = Grayscale2bpp,
    Medium = Grayscale8bpp,
    High = Color24bpp
};
}
