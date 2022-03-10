#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "test/doctest.h"
#include "multiset.hpp"

using namespace masamune;

TEST_CASE("Testing ROI crop") {
    vkn::ImageU16L3D test_image0;
    std::string path0("./images/worm3d.tif");
    image::LoadTiff(path0, test_image0);
    CHECK(test_image0.width == 640);
    CHECK(test_image0.height == 300);

    ROI roi = FindROI(test_image0, 128, 25);
    std::cout << "ROI (x,y,z,sum): " << roi.x << ", " << roi.y << ", " << roi.z << ", " << roi.sum << std::endl;

    vkn::ImageU16L3D test_image0_cropped = image::Crop(test_image0, roi.x, roi.y, roi.z, 128, 128, 25);
    CHECK(test_image0_cropped.width == 128);
    CHECK(test_image0_cropped.height == 128);

    std::string path1("./images/worm3d_cropped.tif");
    image::SaveTiff(path1, test_image0_cropped);

    ROI roi2 = FindROICentred(test_image0, 200, 40);
    vkn::ImageU16L3D test_image1_cropped = image::Crop(test_image0, roi2.x, roi2.y, roi2.z, 200, 200, 40);
    CHECK(test_image1_cropped.width == 200);
    CHECK(test_image1_cropped.height == 200);

    std::string path2("./images/worm3d_cropped_1.tif");
    image::SaveTiff(path2, test_image1_cropped);

}