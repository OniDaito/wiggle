#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "test/doctest.h"
#include "volume.hpp"
#include "image.hpp"

using namespace imagine;

TEST_CASE("Testing Deconvolution with a stack") {
    std::string path_raw("./images/worm3d.tif");
    ImageU16L3D worm_raw = LoadTiff<ImageU16L3D>(path_raw);
    CHECK(worm_raw.width == 640);
    CHECK(worm_raw.height == 300);
    CHECK(worm_raw.depth == 51);

    ImageF32L3D converted = Convert<ImageF32L3D>(worm_raw);

    // Create our gauss kernel
    size_t gw = 41;
    ImageF32L gauss_kernel(gw, gw);
    Gauss(gauss_kernel, 1.0, 30.0f);
    size_t padding = (gw - 1) / 2;

    // Convolve along the stack.
    ImageF32L3D gauss_stack(converted.width, converted.height, converted.depth);

    ImageF32L first_slice = Slice(converted, 0);
    Sandwich(gauss_stack, first_slice, 0);
    ImageF32L last_slice = Slice(converted, converted.depth - 1);
    Sandwich(gauss_stack, last_slice, converted.depth - 1);

    std::function<float(float)> bythree = [](float x) { return x * 3; };

    for (size_t d = 1; d < converted.depth - 1; d++) {
        ImageF32L top_slice = Slice(converted, d - 1 );
        ImageF32L bottom_slice = Slice(converted, d + 1 );
        ImageF32L current_slice = ApplyFunc(Slice(converted, d), bythree);
        ImageF32L top_gauss = Convolve(top_slice, gauss_kernel, 1, padding, PaddingType::ZEROS);
        ImageF32L bottom_gauss = Convolve(bottom_slice, gauss_kernel, 1, padding, PaddingType::ZEROS);

        current_slice = Sub(current_slice, top_gauss);
        current_slice = Sub(current_slice, bottom_gauss);

        Sandwich(gauss_stack, current_slice, d);
    }

    std::string path_deconv("./images/test/worm3d_deconv.fits");
    WriteFITS(path_deconv, gauss_stack);

    // Normalise, constrast then sum
    std::function<float(float)> contrast = [](float x) { return x * x; };
    ImageF32L summed = Project(gauss_stack, ProjectionType::SUM);
    ImageF32L normalised = Normalise(summed);
    //normalised = ApplyFunc(normalised, contrast);
    std::string path_flat("./images/test/worm_deconv.fits");
    WriteFITS(path_flat, normalised);
    

    ImageF32L summed_orig = Project(converted, ProjectionType::SUM);
    ImageF32L normalised_orig = Normalise(summed_orig);
    std::string path_orig("./images/test/worm_orig.fits");
    WriteFITS(path_orig, normalised_orig);


    std::string path_diff("./images/test/worm_diff.fits");
    ImageF32L diff = Sub(normalised_orig, normalised);
    WriteFITS(path_diff, diff);


    ImageF32L contrasted = ApplyFunc(normalised, contrast);
    ImageF32L renormalised = Normalise(contrasted);
    std::string path_cont("./images/test/worm_contrast.fits");
    WriteFITS(path_cont, renormalised);


    std::string path_final("./images/test/worm3d_final.fits");
    ImageF32L3D normalised3d = Normalise(gauss_stack);
    ImageF32L3D contrast3d = ApplyFunc(normalised3d, contrast);
    ImageF32L3D renormalised3d = Normalise(contrast3d);
    WriteFITS(path_final, renormalised3d);

}