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

ROI FindROI(masamune::vkn::ImageU16L3D &input, size_t xy, size_t depth) {
    size_t step_size = 5; // For speed we don't go with 1
    size_t step_depth = 1; // 1 for depth as it's shorter
    size_t num_threads = 4; // One for each quadrant

    ThreadPool pool{ static_cast<size_t>(num_threads) }; // 1 thread per ROI
    std::vector<std::future<int>> futures;

    // Lambda function that we will eventually thread
    std::vector<ROI> rois;

    for (int i = 0; i < num_threads; i++) {
        ROI roi;
        roi.sum = 0;
        roi.x = 0;
        roi.y = 0;
        roi.z = 0;
        roi.xy_dim = xy;
        roi.depth = depth;
        rois.push_back(roi);
    }

    unsigned int t_width = int(input.width / 2);
    unsigned int t_height = int(input.height / 2);
    unsigned int overlap = xy / 2;

    // Set the start and end quadrants
    // Top left, top right, bottom left, bottom right
    std::vector<unsigned int> starts_x = {0, t_width - overlap, 0, t_width - overlap};
    std::vector<unsigned int> ends_x = {t_width + overlap, input.width, t_width + overlap, input.width};
    std::vector<unsigned int> starts_y = {0, t_height - overlap, 0, t_height - overlap};
    std::vector<unsigned int> ends_y = {t_height + overlap, input.height, t_height + overlap, input.height};
    
    for (int i = 0; i <num_threads; i++) {
        int xs = starts_x[i];
        int ys = starts_y[i];
        int zs = 0;
        int xe = ends_x[i];
        int ye = ends_y[i];
        int ze = input.depth;
        int w = xy;
        int h = xy;
        int d = depth;

        ROI *troi = &rois[i]; // Bit naughty, using a raw pointer here :/
 
        futures.push_back(pool.execute(

            [input, troi, xs, ys, zs, xe, ye, ze, w, h, d, step_size, step_depth] () {

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

                            if (sum > troi->sum){
                                troi->x = xi;
                                troi->y = yi;
                                troi->z = zi;
                                troi->sum = sum;
                            }
                        }
                    }
                }
                return 1;
            }
        ));
    }

    for (auto &fut : futures) { fut.get(); }

    ROI roi;
    roi.sum = 0;
    roi.x = 0;
    roi.y = 0;
    roi.z = 0;
    roi.xy_dim = xy;
    roi.depth = depth;

    for (int i = 0; i < num_threads; i++) {
        ROI troi = rois[i];
        if (troi.sum > roi.sum) {
            roi = troi;
        }
    }

    return roi;
}