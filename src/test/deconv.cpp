#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "test/doctest.h"
#include "volume.hpp"
#include "image.hpp"

using namespace imagine;

/*
TEST_CASE("Testing Deconvolution") {
    std::string path_raw("./images/worm3d.tif");
    ImageU16L3D worm_raw = LoadTiff<ImageU16L3D>(path_raw);
    CHECK(worm_raw.width == 640);
    CHECK(worm_raw.height == 300);
    CHECK(worm_raw.depth == 51);

    ImageF32L3D converted = Convert<ImageF32L3D>(worm_raw);

    // Convolve with a gaussian kernel
    ImageF32L gauss_kernel(41, 41);
    Gauss(gauss_kernel, 1.0, 30.0f);

    ImageF32L summed = Project(converted, ProjectionType::SUM);
    ImageF32L deconved = Deconvolve(summed, gauss_kernel, 20);

    // Normalise, constrast then sum
    std::function<float(float)> contrast = [](float x) { return x * x; };
   
    ImageF32L normalised = Normalise(deconved);
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

}*/

TEST_CASE("Testing Deconvolution 3D") {
    std::string path_raw("./images/worm3d.tif");
    ImageU16L3D worm_raw = LoadTiff<ImageU16L3D>(path_raw);
    CHECK(worm_raw.width == 640);
    CHECK(worm_raw.height == 300);
    CHECK(worm_raw.depth == 51);
    worm_raw.depth -= 1;
    worm_raw.data.pop_back();
    ImageF32L3D converted = Convert<ImageF32L3D>(worm_raw);

    // Convolve with a known PSF
    std::string path_kernel("./images/PSF3.tif");
    ImageF32L3D kernel = LoadTiff<ImageF32L3D>(path_kernel);
    ImageF32L3D deconved = DeconvolveFFT(converted, kernel, 10);

    // Now perform a low pass filter - convert to an FFT and multiply by gaussian
    ImageF32C3D fft_deconved = FFT3D(deconved);
    ImageF32L3D gauss(12,12,12);
    Gauss(gauss, 1.0, 2.0f);

    int pw = (fft_deconved.width - gauss.width) / 2;
    int ph = (fft_deconved.height - gauss.height) / 2;
    int pd = (fft_deconved.depth - gauss.depth) / 2;
    ImageF32L3D kernel_padded = Pad(gauss, pw, pw, ph, ph, pd, pd, static_cast<decltype(gauss[0])>(0));

    ImageF32C3D fft_gauss = FFT3D(kernel_padded);
    ImageF32C3D filtered = Mul(fft_deconved, fft_gauss);
    ImageF32L3D final = IFFT3D(filtered);

    std::string path_cont("./images/test/worm_deconved_3d.fits");
    SaveFITS(path_cont, final);

    // Normalise, constrast then sum
    /*std::function<float(float)> contrast = [](float x) { return x * x; };
   
    ImageF32L normalised = Normalise(deconved);
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
    WriteFITS(path_cont, renormalised);*/

}