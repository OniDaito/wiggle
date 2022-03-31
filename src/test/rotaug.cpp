#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "test/doctest.h"
#include "volume.hpp"
#include "rots.hpp"
#include "image.hpp"

using namespace imagine;

TEST_CASE("Testing ROI crop") {
    std::string path0("./images/worm3d.tif");
    ImageU16L3D test_image0 = LoadTiff<ImageU16L3D>(path0);
    CHECK(test_image0.width == 640);
    CHECK(test_image0.height == 300);

    int roi_size = 200;
    float zratio = 6.2f;
    float half_roi = static_cast<float>(roi_size) / 2.0;
    int d = static_cast<int>(ceil(sqrt(2.0f * half_roi * half_roi))) * 2;
    int depth = static_cast<int>(ceil(static_cast<float>(d) / zratio));
    

    ImageU16L3D half = Resize(test_image0, test_image0.width / 2, test_image0.height/ 2, test_image0.depth/ 2);
    // Because we are going to AUG, we make the ROI a bit bigger so we can rotate around
    ROI roi_found = FindROI(half, d / 2, depth / 2);
    
    roi_found.depth *= 2;
    roi_found.x *= 2;
    roi_found.y *= 2;
    roi_found.z *= 2;
    roi_found.xy_dim *=2;

    std::cout << "ROI (x,y,z,wh,d,sum): " << roi_found.x << ", " << roi_found.y << ", " << roi_found.z << ", " << d << ", " << depth << ", " << roi_found.sum << std::endl;

    // Double up the ROI again

    ImageU16L3D cropped = Crop(test_image0, roi_found.x, roi_found.y, roi_found.z, d, d, depth);
    CHECK(cropped.width == d);
    CHECK(cropped.height == d);

    uint16_t min_val = Min(cropped);

    uint16_t min, max;
    MinMax(cropped, min, max);
    std::cout << "Min / Max " << min << ", " << max << std::endl;

    std::string path_crop("./images/worm3d_cropped.tif");
    SaveTiff(path_crop, cropped);

    ImageF32L3D converted = Convert<ImageF32L3D>(cropped);
    std::string path_converted("./images/worm3d_converted.fits");
    WriteFITS(path_converted, converted);

    std::function<float(float)> contrast = [](float x) { return x * x; };
    ImageF32L3D contrasted = ApplyFunc(converted, contrast);

    float fmin, fmax;
    MinMax(contrasted, fmin, fmax);
    std::cout << "Contrasted Min / Max " << fmin << ", " << fmax << std::endl;

    glm::quat quat = RandRot();

    ImageF32L3D augmented = Augment(contrasted, quat, roi_size, zratio);
    ImageF32L3D normalised = Normalise(augmented);

    MinMax(normalised, fmin, fmax);
    std::cout << "Min / Max " << fmin << ", " << fmax << std::endl;
    assert(fmin == 0.0f);
    assert(fmax == 1.0f);

    std::string path_aug3d("./images/worm3d_augmented.fits");
    WriteFITS(path_aug3d, normalised);

    ImageF32L summed = Project(normalised, ProjectionType::SUM);
    std::string path_aug("./images/worm_augmented.fits");
    WriteFITS(path_aug, summed);

    std::string path_jpg("./images/worm_augmented.jpg");
    Save(path_jpg, summed);

    ImageF32L mipped = Project(normalised, ProjectionType::MAX_INTENSITY);
    std::string path_mip("./images/worm_augmented_mip.fits");
    WriteFITS(path_mip, mipped);

    // Do some rotation animations
    int num_frames = 360;

    for (int i = 0; i < num_frames; i++) {
        std::string aug = libsee::IntToStringLeadingZeroes(i, 5);
        std::string path_jpg("./images/anim/worm_" + aug + ".jpg");

        glm::quat qrot = glm::quat(1.0,0,0,0);
        float angle = static_cast<float>(i) / static_cast<float>(num_frames) * M_PI * 2.0;
        qrot = glm::rotate(qrot, angle, glm::vec3(0.0,0.0,1.0));
        qrot = glm::rotate(qrot, angle, glm::vec3(0.0,1.0,0.0));
        qrot = glm::rotate(qrot, angle, glm::vec3(1.0,0.0,0.0));

        ImageF32L3D augmented = Augment(contrasted, qrot, roi_size, zratio);
        ImageF32L3D normalised = Normalise(augmented);
        ImageF32L summed = Project(normalised, ProjectionType::SUM);
        ImageU8L converted = Convert<ImageU8L>(summed);
        
        std::cout << "Writing frame " << aug << std::endl;

        Save(path_jpg, converted);
    }

}