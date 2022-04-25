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
                            ImageU16L3D cropped = Crop(input, xi, yi, zi, w, h, d);
                            double sum = 0;

                            for (int i = 0; i < cropped.depth; i++) {
                                for (int j = 0; j < cropped.height; j++) {
                                    for (int k = 0; k < cropped.width; k++) {
                                        sum += static_cast<double>(cropped.data[i][j][k]);
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


void FindCOM(ImageU8L3D &input, int &cx, int &cy, int &cz, double &sum) {
    double dx, dy, dz = 0;

    for (int z = 0; z < input.depth; z++) {
        for (int y = 0; y < input.height; y++) {
            for (int x = 0; x < input.width; x++) {
                if (input.data[z][y][x] != 0) {
                    dx += x;
                    dy += y;
                    dz += z;
                    sum += input.data[z][y][x];
                }
            }
        }
    }
    cx = int(dx / sum);
    cy = int(dy / sum);
    cz = int(dz / sum);
}

ROI FindROI(ImageU8L3D &input, size_t xy, size_t depth) {
    size_t step_size = 2;
    size_t step_depth = 1; 
    size_t num_threads = 4;

    libcee::ThreadPool pool{ static_cast<size_t>(num_threads) }; // 1 thread per ROI
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
                double dd = w * w + h * h + d * d;
                double hw = w / 2;
                double hh = h / 2;
                double hd = d / 2;

                for (size_t zi = zs; zi + d - 1 < ze; zi += step_depth) {
                    for (size_t yi = ys; yi + h -1 < ye; yi += step_size) {
                        for (size_t xi = xs; xi + w -1 < xe; xi += step_size) {
                            ImageU8L3D cropped = Crop(input, xi, yi, zi, w, h, d);
                            int cx, cy, cz = 0;
                            double sum = 0;
                            FindCOM(cropped, cx, cy, cz, sum);

                            if (sum >= troi->sum) {                                
                                double df = (cx - hw) *  (cx - hw) + (cy - hh) * (cy - hh) + (cz - hd) * (cz - hd);
                                
                                if (df < dd) {
                                    troi->x = xi;
                                    troi->y = yi;
                                    troi->z = zi;
                                    troi->sum = sum;
                                    dd = df;
                                }
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
