#include "multiset.hpp"
#include "threadpool.hpp"

/**
 * 
 * Find ROI. Use the basic image, moving the ROI around to get the bit we want.
 * @param input - a 3D image
 * @param width - ROI width
 * @param height - ROI height
 * @param depth - ROI depth
 * 
 * @return an ROI struct
 */

ROI FindROI(masamune::vkn::ImageU16L3D &input, size_t width, size_t height, size_t depth) {
    ROI roi;
    roi.sum = 0;
    roi.x = 0;
    roi.y = 0;
    roi.z = 0;
    size_t step_size = 5; // For speed we don't go with 1
    size_t step_depth = 1; // 1 for depth as it's shorter
    size_t num_threads = 4;

    // Lambda function that we will eventually thread
    auto f = [](masamune::vkn::ImageU16L3D &input, ROI &roi, size_t xs, size_t ys, size_t zs, size_t xe, size_t ye, size_t ze, size_t w, size_t h, size_t d, size_t step_size, size_t step_depth) {

        for (size_t zi = zs; zi + d - 1 < ze; zi += step_depth) {
            for (size_t yi = ys; yi + h -1 < ye; yi += step_size) {
                for (size_t xi = xs; xi + w -1 < xe; xi += step_size) {
                    masamune::vkn::ImageU16L3D cropped = masamune::image::Crop(input, xi, yi, zi, w, h, d);
                    double sum = 0;

                    for (int i = 0; i < cropped.depth; i++) {
                        for (int j = 0; j < cropped.height; j++) {
                            for (int k = 0; k < cropped.width; k++) {
                                sum += static_cast<double>(cropped.image_data[i][j][k]);
                            }
                        }
                    }

                    if (sum > roi.sum){
                        roi.x = xi;
                        roi.y = yi;
                        roi.z = zi;
                        roi.sum = sum;
                    }
                }
            }
        }
       
    };

    f(input, roi, 0, 0, 0, input.width, input.height, input.depth, 128, 128, 25, step_size, step_depth);

    // Was debating threading this but it's perhaps not worth it :/
    /* std::vector<std::thread> workers;
    for (int i = 0; i < num_threads; i++) {

        workers.push_back(std::thread(f(input, roi, 0, 0, 0, input.width, input.height, input.depth, 128, 128, 25, step_size)));
    }

    // Join up!
    std::for_each(workers.begin(), workers.end(), [](std::thread &t) {
        t.join();
    });*/
    
    return roi;
}