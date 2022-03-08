#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "test/doctest.h"
#include "volume.hpp"
#include "rots.hpp"
#include "image.hpp"

using namespace masamune;

TEST_CASE("Testing ROI crop") {
    vkn::ImageU16L3D test_image0;
    std::string path0("./images/worm3d.tif");
    image::LoadTiff(path0, test_image0);
    CHECK(test_image0.width == 640);
    CHECK(test_image0.height == 300);

    int roi_size = 200;
    float zratio = 6.2f;
    float half_roi = static_cast<float>(roi_size) / 2.0;
    int d = static_cast<int>(ceil(sqrt(2.0f * half_roi * half_roi))) * 2;
    int depth = static_cast<int>(ceil(static_cast<float>(d) / zratio));
    

    vkn::ImageU16L3D half = image::Resize(test_image0, test_image0.width / 2, test_image0.height/ 2, test_image0.depth/ 2);
    // Because we are going to AUG, we make the ROI a bit bigger so we can rotate around
    ROI roi_found = FindROI(half, d / 2, depth / 2);
    
    roi_found.depth *= 2;
    roi_found.x *= 2;
    roi_found.y *= 2;
    roi_found.z *= 2;
    roi_found.xy_dim *=2;

    std::cout << "ROI (x,y,z,wh,d,sum): " << roi_found.x << ", " << roi_found.y << ", " << roi_found.z << ", " << d << ", " << depth << ", " << roi_found.sum << std::endl;

    // Double up the ROI again

    vkn::ImageU16L3D cropped = image::Crop(test_image0, roi_found.x, roi_found.y, roi_found.z, d, d, depth);
    CHECK(cropped.width == d);
    CHECK(cropped.height == d);

    uint16_t min_val = image::Min(cropped);

    uint16_t min, max;
    image::MinMax(cropped, min, max);
    std::cout << "Min / Max " << min << ", " << max << std::endl;

    std::string path_crop("./images/worm3d_cropped.tif");
    image::SaveTiff(path_crop, cropped);

    vkn::ImageF32L3D converted;
    vkn::Convert(cropped, converted);

    std::function<float(float)> contrast = [](float x) { return x * x; };
    vkn::ImageF32L3D contrasted = vkn::ApplyFunc(converted, contrast);

    float fmin, fmax;
    image::MinMax(contrasted, fmin, fmax);
    std::cout << "Contrasted Min / Max " << fmin << ", " << fmax << std::endl;

    glm::quat quat = RandRot();
    //glm::quat quat = glm::quat(1.0,0,0,0);
    //quat = glm::rotate(quat, static_cast <float>(M_PI/4.0), glm::vec3(0.0,0,1.0));
    //std::cout << "ROT (w,x,y,z): " << quat.w << ", " << quat.x << ", " << quat.y << ", " << quat.z << std::endl;

    vkn::ImageF32L3D augmented = Augment(contrasted, quat, roi_size, zratio);
    vkn::ImageF32L3D normalised = image::Normalise(augmented);

    image::MinMax(normalised, fmin, fmax);
    std::cout << "Min / Max " << fmin << ", " << fmax << std::endl;
    assert(fmin == 0.0f);
    assert(fmax == 1.0f);

    std::string path_aug3d("./images/worm3d_augmented.fits");
    WriteFITS(path_aug3d, normalised);

    vkn::ImageF32L summed = vkn::Project(normalised, vkn::ProjectionType::SUM);
    std::string path_aug("./images/worm_augmented.fits");
    WriteFITS(path_aug, summed);

    vkn::ImageF32L mipped = vkn::Project(normalised, vkn::ProjectionType::MAX_INTENSITY);
    std::string path_mip("./images/worm_augmented_mip.fits");
    WriteFITS(path_mip, mipped);

}