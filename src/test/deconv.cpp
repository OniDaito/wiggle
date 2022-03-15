#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "test/doctest.h"
#include "volume.hpp"
#include "image.hpp"

using namespace masamune;

TEST_CASE("Testing Deconvolution") {
    vkn::ImageU16L3D worm_raw;
    std::string path_raw("./images/worm3d.tif");
    image::LoadTiff(path_raw, worm_raw);
    CHECK(worm_raw.width == 640);
    CHECK(worm_raw.height == 300);
    CHECK(worm_raw.depth == 51);

    vkn::ImageF32L3D converted;
    image::Convert(worm_raw, converted);


    // Create our gauss kernel with a guess at 10 sigma
    vkn::ImageF32L gauss_kernel;
    size_t gw = 61;
    size_t padding = (gw - 1) / 2;
    gauss_kernel.width = gw;
    gauss_kernel.height = gw;
    vkn::Alloc(gauss_kernel);
    float sigma = 80.0;

    for (size_t y = 0; y < gw; y ++) {
        for (size_t x = 0; x < gw; x ++) {
            float xf = static_cast<float>(x);
            float yf = static_cast<float>(y);
            float xx = xf * xf;
            float yy = yf * yf;

            float g = (1.0f / (sigma * sigma * 2.0 * M_PI)) * exp(-( xx + yy) / (2.0f*sigma*sigma) ); 
            gauss_kernel.image_data[y][x] = g;
        }
    }

    vkn::ImageF32L3D gauss_stack;
    gauss_stack.width = converted.width;
    gauss_stack.height = converted.height;
    gauss_stack.depth = converted.depth;
    vkn::Alloc(gauss_stack);

    vkn::ImageF32L first_slice = vkn::Slice(converted, 0);
    vkn::Sandwich(gauss_stack, first_slice, 0);
    vkn::ImageF32L last_slice = vkn::Slice(converted, converted.depth - 1);
    vkn::Sandwich(gauss_stack, last_slice, converted.depth - 1);

    std::function<float(float)> bythree = [](float x) { return x * 3; };

    for (size_t d = 1; d < converted.depth - 1; d++) {
        vkn::ImageF32L top_slice = vkn::Slice(converted, d - 1 );
        vkn::ImageF32L bottom_slice = vkn::Slice(converted, d + 1 );
        vkn::ImageF32L current_slice = vkn::ApplyFunc(vkn::Slice(converted, d), bythree);
        vkn::ImageF32L top_gauss = image::Convolve(top_slice, gauss_kernel, 1, padding, image::PaddingType::ZEROS);
        vkn::ImageF32L bottom_gauss = image::Convolve(bottom_slice, gauss_kernel, 1, padding, image::PaddingType::ZEROS);

        current_slice = image::Sub(current_slice, top_gauss);
        current_slice = image::Sub(current_slice, bottom_gauss);

        vkn::Sandwich(gauss_stack, current_slice, d);
    }

    std::string path_deconv("./images/test/worm3d_deconv.fits");
    WriteFITS(path_deconv, gauss_stack);

    // Normalise, constrast then sum
    std::function<float(float)> contrast = [](float x) { return x * x; };
    vkn::ImageF32L summed = vkn::Project(gauss_stack, vkn::ProjectionType::SUM);
    vkn::ImageF32L normalised = image::Normalise(summed);
    //normalised = vkn::ApplyFunc(normalised, contrast);
    std::string path_flat("./images/test/worm_deconv.fits");
    WriteFITS(path_flat, normalised);
    

    vkn::ImageF32L summed_orig = vkn::Project(converted, vkn::ProjectionType::SUM);
    vkn::ImageF32L normalised_orig = image::Normalise(summed_orig);
    std::string path_orig("./images/test/worm_orig.fits");
    WriteFITS(path_orig, normalised_orig);


    std::string path_diff("./images/test/worm_diff.fits");
    vkn::ImageF32L diff = image::Sub(normalised_orig, normalised);
    WriteFITS(path_diff, diff);


    vkn::ImageF32L contrasted = vkn::ApplyFunc(normalised, contrast);
    vkn::ImageF32L renormalised = image::Normalise(contrasted);
    std::string path_cont("./images/test/worm_contrast.fits");
    WriteFITS(path_cont, renormalised);


    std::string path_final("./images/test/worm3d_final.fits");
    vkn::ImageF32L3D normalised3d = image::Normalise(gauss_stack);
    vkn::ImageF32L3D contrast3d = vkn::ApplyFunc(normalised3d, contrast);
    vkn::ImageF32L3D renormalised3d = image::Normalise(contrast3d);
    WriteFITS(path_final, renormalised3d);


}