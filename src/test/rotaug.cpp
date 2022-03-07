#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "test/doctest.h"
#include "volume.hpp"
#include "rots.hpp"

using namespace masamune;

TEST_CASE("Testing ROI crop") {
    vkn::ImageU16L3D test_image0;
    std::string path0("./images/worm3d.tif");
    image::LoadTiff(path0, test_image0);
    CHECK(test_image0.width == 320);
    CHECK(test_image0.height == 150);

    ROI roi = FindROI(test_image0, 128, 25);
    std::cout << "ROI (x,y,z,sum): " << roi.x << ", " << roi.y << ", " << roi.z << ", " << roi.sum << std::endl;

    vkn::ImageU16L3D test_image0_cropped = image::Crop(test_image0, roi.x, roi.y, roi.z, 128, 128, 25);
    CHECK(test_image0_cropped.width == 128);
    CHECK(test_image0_cropped.height == 128);

    std::string path1("./images/worm3d_cropped.tif");
    image::SaveTiff(path1, test_image0_cropped);

    //glm::quat quat = RandRot();
    glm::quat quat = glm::quat(1.0,0,0,0);
    quat = glm::rotate(quat, static_cast <float>(M_PI/4.0), glm::vec3(0.0,0,1.0));
    std::cout << "ROT (w,x,y,z): " << quat.w << ", " << quat.x << ", " << quat.y << ", " << quat.z << std::endl;

    test_image0_cropped = Augment(test_image0_cropped, quat, 100, 6.2f);
    std::string path3("./images/worm3d_augmented.tif");
    image::SaveTiff(path3, test_image0_cropped);

    vkn::ImageU16L test_image0_final = vkn::Project(test_image0_cropped, vkn::ProjectionType::SUM);
    std::string path2("./images/worm_augmented.tif");
    image::SaveTiff(path2, test_image0_final);

}