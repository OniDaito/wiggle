#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "test/doctest.h"
#include "volume.hpp"
#include "image.hpp"

using namespace masamune;

/*
TEST_CASE("Testing Deconvolution") {
    std::string path_raw("./images/worm3d.tif");
    vkn::ImageU16L3D worm_raw = image::LoadTiff<vkn::ImageU16L3D>(path_raw);
    CHECK(worm_raw.width == 640);
    CHECK(worm_raw.height == 300);
    CHECK(worm_raw.depth == 51);

    vkn::ImageF32L3D converted = image::Convert<vkn::ImageF32L3D>(worm_raw);

    // Convolve with a gaussian kernel
    vkn::ImageF32L gauss_kernel(41, 41);
    image::Gauss(gauss_kernel, 1.0, 30.0f);

    vkn::ImageF32L summed = vkn::Project(converted, vkn::ProjectionType::SUM);
    vkn::ImageF32L deconved = image::Deconvolve(summed, gauss_kernel, 20);

    // Normalise, constrast then sum
    std::function<float(float)> contrast = [](float x) { return x * x; };
   
    vkn::ImageF32L normalised = image::Normalise(deconved);
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

}*/

TEST_CASE("Testing Deconvolution 3D") {
    std::string path_raw("./images/worm3d.tif");
    vkn::ImageU16L3D worm_raw = image::LoadTiff<vkn::ImageU16L3D>(path_raw);
    CHECK(worm_raw.width == 640);
    CHECK(worm_raw.height == 300);
    CHECK(worm_raw.depth == 51);
    worm_raw.depth -= 1;
    worm_raw.data.pop_back();
    vkn::ImageF32L3D converted = image::Convert<vkn::ImageF32L3D>(worm_raw);

    // Convolve with a known PSF
    std::string path_kernel("./images/PSF3.tif");
    vkn::ImageF32L3D kernel = image::LoadTiff<vkn::ImageF32L3D>(path_kernel);
    vkn::ImageF32L3D deconved = image::DeconvolveFFT(converted, kernel, 10);

    // Now perform a low pass filter - convert to an FFT and multiply by gaussian
    vkn::ImageF32C3D fft_deconved = image::FFT3D(deconved);
    vkn::ImageF32L3D gauss(12,12,12);
    image::Gauss(gauss, 1.0, 2.0f);

    int pw = (fft_deconved.width - gauss.width) / 2;
    int ph = (fft_deconved.height - gauss.height) / 2;
    int pd = (fft_deconved.depth - gauss.depth) / 2;
    vkn::ImageF32L3D kernel_padded = image::Pad(gauss, pw, pw, ph, ph, pd, pd, static_cast<decltype(gauss[0])>(0));

    vkn::ImageF32C3D fft_gauss = image::FFT3D(kernel_padded);
    vkn::ImageF32C3D filtered = image::Mul(fft_deconved, fft_gauss);
    vkn::ImageF32L3D final = image::IFFT3D(filtered);

    std::string path_cont("./images/test/worm_deconved_3d.fits");
    WriteFITS(path_cont, final);

    // Normalise, constrast then sum
    /*std::function<float(float)> contrast = [](float x) { return x * x; };
   
    vkn::ImageF32L normalised = image::Normalise(deconved);
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
    WriteFITS(path_cont, renormalised);*/

}