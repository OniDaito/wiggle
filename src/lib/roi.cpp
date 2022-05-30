#include "roi.hpp"

/**
 * 
 * Find ROI. Use the basic image, moving the ROI around to get the bit we want.
 * 
 * @param input - a 3D image
 * @param width - ROI width
 * @param height - ROI height
 * @param depth - ROI depth
 * 
 * @return an ROI struct
 */

using namespace imagine;


ROI FindROI(ImageU16L3D &input, size_t xy, size_t depth) {
    size_t step_size = 5; // For speed we don't go with 1
    size_t step_depth = 1; // 1 for depth as it's shorter
    size_t num_threads = 4; // One for each quadrant

    libcee::ThreadPool pool{ static_cast<size_t>(num_threads) }; // 1 thread per ROI
    std::vector<std::future<ROI>> futures;

    // Lambda function that we will eventually thread
    std::vector<ROI> rois;

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
 
        futures.push_back(pool.execute(

            [input, xs, ys, zs, xe, ye, ze, w, h, d, step_size, step_depth] () {
                ROI troi;
                troi.sum = 0;
                troi.x = 0;
                troi.y = 0;
                troi.z = 0;

                for (size_t zi = zs; zi + d - 1 < ze; zi += step_depth) {
                    for (size_t yi = ys; yi + h -1 < ye; yi += step_size) {
                        for (size_t xi = xs; xi + w -1 < xe; xi += step_size) {
                            ImageU16L3D cropped = Crop(input, xi, yi, zi, w, h, d);
                            double sum = 0;

                            for (int i = 0; i < cropped.depth; i++) {
                                for (int j = 0; j < cropped.height; j++) {
                                    for (int k = 0; k < cropped.width; k++) {
                                        sum += std::min(1.0, std::max(static_cast<double>(cropped.data[i][j][k]), 0.0));
                                    }
                                }
                            }

                            if (sum > troi.sum){
                                troi.x = xi;
                                troi.y = yi;
                                troi.z = zi;
                                troi.sum = sum;
                            }
                        }
                    }
                }
                return troi;
            }
        ));
    }

    for (auto &fut : futures) { rois.push_back(fut.get()); }

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


void FindCOM(ImageU8L3D &input, int &cx, int &cy, int &cz, int &sum) {
    float dx = 0, dy = 0, dz = 0, tsum = 0;

    for (int z = 0; z < input.depth; z++) {
        for (int y = 0; y < input.height; y++) {
            for (int x = 0; x < input.width; x++) {
                if (input.data[z][y][x] != 0) {
                    dx += static_cast<float>(x);
                    dy += static_cast<float>(y);
                    dz += static_cast<float>(z);
                    tsum += std::min(1.0f, std::max(static_cast<float>(input.data[z][y][x]), 0.0f));
                }
            }
        }
    }

    if (tsum > 0) {
        cx = static_cast<int>(floor(dx / tsum));
        cy = static_cast<int>(floor(dy / tsum));
        cz = static_cast<int>(floor(dz / tsum));
    }

}

ROI FindROI(ImageU8L3D &input, size_t xy, size_t depth) {
    size_t step_size = 2;
    size_t step_depth = 1; 
    size_t num_threads = 4;

    libcee::ThreadPool pool{ static_cast<size_t>(num_threads) }; // 1 thread per ROI
    std::vector<std::future<ROI>> futures;

    // Lambda function that we will eventually thread
    std::vector<ROI> rois;

    unsigned int t_width = int(input.width / 2);
    unsigned int t_height = int(input.height / 2);
    unsigned int overlap = xy / 2;

    // Set the start and end quadrants
    // Top left, top right, bottom left, bottom right
    std::vector<unsigned int> starts_x = {0, t_width - overlap, 0, t_width - overlap};
    std::vector<unsigned int> ends_x = {t_width + overlap, input.width, t_width + overlap, input.width};
    std::vector<unsigned int> starts_y = {0, t_height - overlap, 0, t_height - overlap};
    std::vector<unsigned int> ends_y = {t_height + overlap, input.height, t_height + overlap, input.height};
    
    for (int thread_id = 0; thread_id <num_threads; thread_id++) {
        int xs = starts_x[thread_id];
        int ys = starts_y[thread_id];
        int zs = 0;
        int xe = ends_x[thread_id];
        int ye = ends_y[thread_id];
        int ze = input.depth;
        int w = xy;
        int h = xy;
        int d = depth;

        futures.push_back(pool.execute(

            [input, xs, ys, zs, xe, ye, ze, w, h, d, step_size, step_depth] () {
                double dd = w * w + h * h + d * d;
                double hw = w / 2;
                double hh = h / 2;
                double hd = d / 2;
                ROI troi;
                troi.sum = 0;
                troi.x = 0;
                troi.y = 0;
                troi.z = 0;

                for (size_t zi = zs; zi + d - 1 < ze; zi += step_depth) {
                    for (size_t yi = ys; yi + h -1 < ye; yi += step_size) {
                        for (size_t xi = xs; xi + w -1 < xe; xi += step_size) {
                            ImageU8L3D cropped = Crop(input, xi, yi, zi, w, h, d);
                            int cx = 0, cy = 0, cz = 0;
                            int sum = 0;
                            FindCOM(cropped, cx, cy, cz, sum);

                            if (sum >= troi.sum) {                                
                                double df = (cx - hw) *  (cx - hw) + (cy - hh) * (cy - hh) + (cz - hd) * (cz - hd);

                                if (df < dd) {
                                    troi.x = xi;
                                    troi.y = yi;
                                    troi.z = zi;
                                    troi.sum = sum;
                                    dd = df;
                                }
                            }
                        }
                    }
                }
      
                return troi;
            }
        ));
    }

    for (auto &fut : futures) { rois.push_back(fut.get()); }

    ROI roi;
    roi.sum = 0;
    roi.x = 0;
    roi.y = 0;
    roi.z = 0;
    roi.xy_dim = xy;
    roi.depth = depth;

    double hw = xy / 2;
    double hh = xy / 2;
    double hd = depth / 2;
    double dd = xy * xy + xy * xy+ depth * depth;


    for (int i = 0; i < num_threads; i++) {
        ROI troi = rois[i];

        if (troi.sum >= roi.sum) {
            ImageU8L3D cropped = Crop(input, troi.x, troi.y, troi.z, roi.xy_dim, roi.xy_dim, roi.depth);
            int cx, cy, cz = 0;
            int sum = 0;
            FindCOM(cropped, cx, cy, cz, sum);

            double df = (cx - hw) *  (cx - hw) + (cy - hh) * (cy - hh) + (cz - hd) * (cz - hd);
                
            if (df < dd) {
                roi.sum = troi.sum;
                roi.x = troi.x;
                roi.y = troi.y;
                roi.z = troi.z;
                dd = df;
            }
        }
    }

    return roi;
}
