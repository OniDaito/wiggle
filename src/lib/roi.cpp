#include "roi.hpp"
#include "threadpool.hpp"

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



/**
 * 
 * Find ROI. Use the basic image, moving the ROI around to get the bit we want.
 * 
 * Unlike the one above, we look for our 4 hotspots and keep them centered.
 * 
 * @param input - a 3D image
 * @param width - ROI width
 * @param height - ROI height
 * @param depth - ROI depth
 * 
 * @return an ROI struct
 */

typedef struct {
    float val;
    size_t x, y, z;
} FourCoord;

bool CompareFour(FourCoord i1, FourCoord i2) {
    return (i1.val < i2.val);
}

ROI FindROICentred(masamune::vkn::ImageU16L3D &input, size_t xy, size_t depth) {
    size_t step_size = 5; // For speed we don't go with 1
    size_t step_depth = 1; // 1 for depth as it's shorter

    // Lambda function that we will eventually thread
    ROI roi;
    roi.xy_dim = xy;
    roi.depth = depth;
    std::deque<FourCoord> top_four;

    for (size_t z = 0; z < input.depth; z += step_depth) {
        for (size_t y = 0; y < input.height; y += step_size) {
            for (size_t x = 0; x < input.width; x += step_size) {
                uint16_t val = input.image_data[z][y][x];
                
                if (top_four.size() < 4){
                    FourCoord f = {static_cast<float>(val), x, y, z};
                    top_four.push_back(f);
                    std::sort(top_four.begin(), top_four.end(), CompareFour);
                } else {
                    if (val > top_four[0].val) {
                        FourCoord f = {val, x, y, z};
                        top_four.pop_front();
                        top_four.push_back(f);
                        std::sort(top_four.begin(), top_four.end(), CompareFour);
                    }  
                }
            }
        }
    }

    roi.x = static_cast<size_t>(std::max( ((top_four[0].x + top_four[1].x + top_four[2].x + top_four[3].x) / 4.0f - static_cast<float>(xy) / 2.0f), 0.0f));
    roi.y = static_cast<size_t>(std::max( ((top_four[0].y + top_four[1].y + top_four[2].y + top_four[3].y) / 4.0f - static_cast<float>(xy) / 2.0f), 0.0f));
    roi.z = 0;
    
    if (depth < input.depth) {
        roi.z = static_cast<size_t>(std::max( ((top_four[0].z + top_four[1].z + top_four[2].z + top_four[3].z) / 4.0f - static_cast<float>(depth) / 2.0f), 0.0f));
    }
    
    //std::cout << "Input " << input.width << ", " << input.height << ", " << input.depth << std::endl;
    //std::cout << "ROI " << roi.x << ", " << roi.y << ", " << roi.z << ", " << xy << ", " << depth << std::endl;

    if (roi.x + xy >= input.width) {
        roi.x = roi.x - (roi.x + xy - input.width + 1);
    }
    if (roi.y + xy >= input.height) {
        roi.y = roi.y - (roi.y + xy - input.height + 1);
    }
    if (roi.z + depth >= input.depth) {
        roi.z = roi.z - (roi.z + depth - input.depth + 1);
    }

    //std::cout << "ROI adjusted " << roi.x << ", " << roi.y << ", " << roi.z << ", " << xy << ", " << depth << std::endl;

    return roi;
}