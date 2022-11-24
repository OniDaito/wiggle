/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file pipe.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 03/11/2022
 * @brief The process pipeline for the images
 * 
 * Source and mask tiffs come through here from the 
 * various other programs, to generate the output files
 * we need.
 */

#include "pipe.hpp"

using namespace imagine;

/**
 * @brief The Image processing pipeline for source images.
 * Perform a Crop, noise subtraction and deconvolution
 *
 * 
 * @param image_in 
 * @param roi 
 * @return ImageF32L3D 
 */

ImageF32L3D ProcessPipe(ImageU16L3D const &image_in,  bool autoback, float noise, bool deconv, const std::string &psf_path, int deconv_rounds, int &background) {
    // Convert to float as we need to do some operations
    ImageF32L3D converted = Convert<ImageF32L3D>(image_in);

    if (autoback){
        // Perform an automatic background subtraction based on local means
        size_t kernel_dia = 3; // A 3 x 3 x 3 kernel to compute the average
        size_t kernel_rad = 1;
        std::vector<int> modes;

        for (uint32_t z = kernel_rad; z < converted.depth - kernel_rad; z++) {
            for (uint32_t y = kernel_rad; y < converted.height - kernel_rad; y++) {
                for (uint32_t x = kernel_rad; x < converted.width - kernel_rad; x++) {
                    // Add up all the pixels - 27 of them
                    float avg = converted.data[z][y][x];
                    // First 9
                    avg += converted.data[z-kernel_rad][y-kernel_rad][x-kernel_rad];
                    avg += converted.data[z-kernel_rad][y-kernel_rad][x];
                    avg += converted.data[z-kernel_rad][y-kernel_rad][x+kernel_rad];
                    avg += converted.data[z-kernel_rad][y][x-kernel_rad];
                    avg += converted.data[z-kernel_rad][y][x];
                    avg += converted.data[z-kernel_rad][y][x+kernel_rad];
                    avg += converted.data[z-kernel_rad][y+kernel_rad][x-kernel_rad];
                    avg += converted.data[z-kernel_rad][y+kernel_rad][x];
                    avg += converted.data[z-kernel_rad][y+kernel_rad][x+kernel_rad];

                    // Mid 8
                    avg += converted.data[z][y-kernel_rad][x-kernel_rad];
                    avg += converted.data[z][y-kernel_rad][x];
                    avg += converted.data[z][y-kernel_rad][x+kernel_rad];
                    avg += converted.data[z][y][x-kernel_rad];
                    avg += converted.data[z][y][x+kernel_rad];
                    avg += converted.data[z][y+kernel_rad][x-kernel_rad];
                    avg += converted.data[z][y+kernel_rad][x];
                    avg += converted.data[z][y+kernel_rad][x+kernel_rad];

                    // Last 9
                    avg += converted.data[z+kernel_rad][y-kernel_rad][x-kernel_rad];
                    avg += converted.data[z+kernel_rad][y-kernel_rad][x];
                    avg += converted.data[z+kernel_rad][y-kernel_rad][x+kernel_rad];
                    avg += converted.data[z+kernel_rad][y][x-kernel_rad];
                    avg += converted.data[z+kernel_rad][y][x];
                    avg += converted.data[z+kernel_rad][y][x+kernel_rad];
                    avg += converted.data[z+kernel_rad][y+kernel_rad][x-kernel_rad];
                    avg += converted.data[z+kernel_rad][y+kernel_rad][x];
                    avg += converted.data[z+kernel_rad][y+kernel_rad][x+kernel_rad];

                    avg /= 27.0;
                    int mode = static_cast<int>(round(avg));
                    modes.push_back(mode);
                }
            }
        }

        typedef std::pair<int, int> mode_pair;

        struct mode_predicate {
            bool operator() (mode_pair const& lhs, mode_pair const& rhs) {
                return lhs.second < rhs.second;
            }
        };

        // Now find the mode of all the values we have
        std::map<int, int> mode_map;

        for (int n = 0; n < modes.size(); n++) {
            mode_map[modes[n]]++;
        }

        mode_predicate mp;
        int final_mode = std::max_element(mode_map.begin(), mode_map.end(), mp)->first;
        std::cout << "Background Value:" << final_mode << std::endl; 
        background = final_mode;
        converted = Sub(converted, static_cast<float>(final_mode), true);

    } else {
        converted = Sub(converted, noise, true);
    }

    // Perform a subtraction on the images, removing background
    if (deconv){ 
        // Drop the last layer if the image has an odd depth
        bool dropped = false;
        std::vector<std::vector<float>> last_slice(converted.data[converted.depth-1]);

        if (converted.depth % 2 == 1) {
            converted.data.pop_back();
            converted.depth -= 1;
            dropped == true;
        }

        // Deconvolve with a known PSF
        std::string path_kernel(psf_path);
        ImageF32L3D kernel = LoadTiff<ImageF32L3D>(path_kernel);
        ImageF32L3D deconved = DeconvolveFFT(converted, kernel, deconv_rounds);

        if (dropped) {
            converted.data.push_back(last_slice);
            converted.depth += 1;
        }
        
        return deconved;
    }
    return converted;
}



/**
 * @brief Do not perform augmentation on the final part of processing the tiff
 * 
 * @param processed 
 * @param image_id 
 */

void _NoAugSource(const Options &options, ImageF32L3D processed, std::string image_id) {
   
    // Rotate, normalise then sum projection
    std::string output_path = options.output_path + "/" + image_id + "_layered.fits";
    if (options.flatten) {
        auto ptype = ProjectionType::SUM;
        
        if (options.max_intensity) {
            ptype = ProjectionType::MAX_INTENSITY;
        }

        ImageF32L summed = Project(processed, ptype);
        FlipVerticalI(summed);

        if (options.final_width != summed.width || options.final_height != summed.height) {
            summed = Resize(summed, options.final_width, options.final_height);
        }

        SaveFITS(output_path, summed);

        // Write a JPG just in case
        ImageU8L jpeged = Convert<ImageU8L>(Convert<ImageF32L>(summed));
        std::string output_path_jpg = options.output_path + "/" +  image_id + "_raw.jpg";
        SaveJPG(output_path_jpg, jpeged);
    } else {
        // ImageF32L3D normalised = Normalise(rotated);
        //FlipVerticalI(normalised);
        //ImageF32L3D resized = Resize(normalised, options.final_width, options.final_height, options.final_depth);
        FlipVerticalI(processed);
        if (options.final_width != processed.width || options.final_height != processed.height || options.final_depth != processed.depth) {
            if (processed.depth % 2 == 1) {
                processed.data.pop_back();
                processed.depth -= 1;
            }
            ImageF32L3D resized = Resize(processed, options.final_width, options.final_height, options.final_depth);
            SaveFITS(output_path, resized);
        } else {
            SaveFITS(output_path, processed);   
        }
    }
}

/**
 * @brief Augment the source image
 * 
 * @param processed 
 * @param image_id 
 */

void _AugSource(const Options &options, ImageF32L3D &processed, const std::vector<glm::quat> &ROTS, std::string image_id) {
    // Now perform some rotations, sum, normalise, contrast then renormalise for the final 2D image
    // Thread this bit for a bit more speed
    std::string output_path = options.output_path + "/" + image_id + "_layered.fits";
    libcee::ThreadPool pool{ static_cast<size_t>( options.num_augs) };
    std::vector<std::future<int>> futures;

    for (int i = 0; i < options.num_augs; i++){
        futures.push_back(pool.execute( [i, output_path, ROTS, options, image_id, processed] () {  
            // Rotate, normalise then sum projection
            std::string aug_id  = libcee::IntToStringLeadingZeroes(i, 2);
            std::string output_path = options.output_path + "/" + image_id + "_" + aug_id + "_layered.fits";
            glm::quat q = ROTS[i];
            ImageF32L3D rotated = Augment(processed, q, options.roi_xy, options.depth_scale, options.subpixel, options.interz); 
            if (options.flatten) {
                auto ptype = ProjectionType::SUM;
                
                if (options.max_intensity) {
                    ptype = ProjectionType::MAX_INTENSITY;
                }

                ImageF32L summed = Project(rotated, ptype);
                FlipVerticalI(summed);

                if (options.final_width != summed.width || options.final_height != summed.height) {
                    summed = Resize(summed, options.final_width, options.final_height);
                }

                SaveFITS(output_path, summed);

                // Write a JPG just in case
                ImageU8L jpeged = Convert<ImageU8L>(Convert<ImageF32L>(summed));
                std::string output_path_jpg = options.output_path + "/" +  image_id + "_" + aug_id + "_raw.jpg";
                SaveJPG(output_path_jpg, jpeged);
            } else {
                FlipVerticalI(rotated);

                if (rotated.depth % options.final_depth == 1) {
                    rotated.data.pop_back();
                }

                ResizeMethod method = ResizeMethod::NEAREST;
                
                if (options.interz) {
                    method = ResizeMethod::TRILINEAR;
                }

                ImageF32L3D resized = Resize(rotated, options.final_width, options.final_height, options.final_depth, method);
                SaveFITS(output_path, resized);
            }
            
            return i;
        }));
    }

    for (auto &fut : futures) { fut.get(); }
}

/**
 * Given a tiff file return the same file but with one channel and 
 * as a series of layers
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 *
 * @return bool if success or not
 */

int TiffToFits(const Options &options, const std::vector<glm::quat> &ROTS, std::string &tiff_path, int image_idx, ROI &roi) {
    ImageU16L image = LoadTiff<ImageU16L>(tiff_path); 
    ImageU16L3D stacked(image.width, (image.height / (options.stacksize * options.channels)), options.stacksize);
    uint coff = 0;

    if (options.bottom) {
        coff = stacked.height;
    }

    for (uint32_t d = 0; d < stacked.depth; d++) {
        for (uint32_t y = 0; y < stacked.height; y++) {
            for (uint32_t x = 0; x < stacked.width; x++) {
                uint16_t val = image.data[(d * stacked.height * options.channels) + coff + y][x];
                stacked.data[d][y][x] = val;
            }
        }
    }

    std::vector<std::string> tokens_log = libcee::SplitStringChars(libcee::FilenameFromPath(tiff_path), "_.-");
    std::string image_id = tokens_log[3];
    image_id = libcee::StringRemove(image_id, "0xAutoStack");

    if (options.rename == true) {
        image_id  = libcee::IntToStringLeadingZeroes(image_idx, 5);
        std::string output_path = options.output_path + "/" + image_id + "_layered.fits";
        std::cout << "Renaming " << tiff_path << " to " << output_path << std::endl;
    }

    ImageF32L3D converted;

    if (!options.noroi) {
        stacked = Crop(stacked, roi.x, roi.y, roi.z, roi.xy_dim, roi.xy_dim, roi.depth);
    }

    int background = options.cutoff;

    if (!options.noprocess){
        if (options.otsu){
            auto thresh = imagine::Otsu(stacked);
            std::function<uint16_t (uint16_t)> thresh_func = [thresh](uint16_t x) { if(x >= thresh) { return x;} return static_cast<uint16_t>(0); };
            stacked = ApplyFunc<ImageU16L3D, uint16_t>(stacked, thresh_func);
            converted = imagine::Convert<ImageF32L3D>(stacked);
        } else {
            converted = ProcessPipe(stacked, options.autoback, options.cutoff, options.deconv, options.psf_path, options.deconv_rounds, background);
        }
    } else  {
        converted = imagine::Convert<ImageF32L3D>(stacked);
    }

    if (options.num_augs > 1) {
        _AugSource(options, converted, ROTS, image_id);
    } else {
        _NoAugSource(options, converted, image_id);
    }

    // TODO - this is not ideal really. We should probably reconsider interfaces
    return background;
}


/**
 * @brief Read the co-ordinates of the 
 * 
 * @param coord_path 
 * @param graph 
 * @param extra_data 
 * @param roi 
 * @return true 
 * @return false 
 */
bool _ReadGraph(std::string coord_path, std::vector<glm::vec4> &graph,  std::vector<std::string> &extra_data, const ROI &roi) {
    if (!libcee::FileExists(coord_path)) { return false; }

    // Read the dat file and write out the coordinates in order as an entry in a CSV file
    std::vector<std::string> lines = libcee::ReadFileLines(coord_path);
    if(lines.size() != 4) {  return false; }

    // Should be ASI-1, ASI-2, ASJ-1, ASJ-2

    // Get the required data from the .dat files in the annotation
    for (std::string line : lines) {
        std::vector<std::string> tokens = libcee::SplitStringWhitespace(line);
        std::string x = libcee::RemoveChar(libcee::RemoveChar(tokens[4], ','), ' ');
        std::string y = libcee::RemoveChar(libcee::RemoveChar(libcee::RemoveChar(tokens[3], ','), ' '), '[');
        std::string z = libcee::RemoveChar(libcee::RemoveChar(libcee::RemoveChar(tokens[5], ','), ' '), ']');
        glm::vec4 p = glm::vec4(libcee::FromString<float>(x), libcee::FromString<float>(y), libcee::FromString<float>(z), 1.0f);
    
        std::string fl = libcee::RemoveChar(libcee::RemoveChar(tokens[1], ','), ' ');
        std::string mode_fl = libcee::RemoveChar(libcee::RemoveChar(tokens[6], ','), ' ');
        std::string min_fl = libcee::RemoveChar(libcee::RemoveChar(tokens[7], ','), ' ');
        std::string bg = libcee::RemoveChar(libcee::RemoveChar(tokens[2], ','), ' ');
        std::string rsize = libcee::RemoveChar(libcee::RemoveChar(tokens[8], ','), ' ');

        std::string extra = fl + "," + bg + "," + mode_fl + "," + min_fl + "," + rsize;
        extra_data.push_back(extra);

        // Must have all 4 neurons for now
        if (p.x == 0 && p.y == 0 && p.z == 0) {
            return false;
        }

        p.x = (p.x - roi.x);
        p.y = (p.y - roi.y);
        p.z = (p.z - roi.z);
        graph.push_back(p);
    }
    return true;
}


/**
 * @brief Process all the files required to make a mask. Also perform augmentation via rotation.
 * 
 * TODO - merge with the other process mask function
 *
 * @param options 
 * @param tiff_path 
 * @param log_path 
 * @param coord_path 
 * @param ROTS 
 * @param image_idx 
 * @param roi 
 * @return true 
 * @return false 
 */

bool ProcessMask(Options &options, std::string &tiff_path, std::string &log_path, std::string &coord_path, std::vector<glm::quat> &ROTS,  int image_idx, ROI &roi) {
    ImageU16L image_in = LoadTiff<ImageU16L>(tiff_path);
    std::vector<std::vector<size_t>> neurons; // 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2
    size_t idx = 0;

    // Read the log file and extract the neuron numbers
    // Making the asssumption that all log files have the neurons in the same order
    for (int i = 0; i < 5; i++) {
        neurons.push_back(std::vector<size_t>());
    }

    std::vector<std::string> lines_log = libcee::ReadFileLines(log_path);

    for (std::string line : lines_log) {
        std::vector<std::string> tokens = libcee::SplitStringWhitespace(line);
        if (libcee::ToLower(tokens[0]) == "associate") {
            idx += 1;
        } else {
            neurons[idx].push_back(libcee::FromString<size_t>(tokens[0]));
        }
    }

    // Join all our neurons
    ImageU8L3D neuron_mask(image_in.width, image_in.height / options.stacksize, options.stacksize);
    bool n1 = false, n2 = false, n3 = false, n4 = false;

    if(options.threeclass){
      n1 = SetNeuron(image_in, neuron_mask, neurons, 1, true, true, 1);
      n2 = SetNeuron(image_in, neuron_mask, neurons, 2, true, true, 1);
      n3 = SetNeuron(image_in, neuron_mask, neurons, 3, true, true, 2);
      n4 = SetNeuron(image_in, neuron_mask, neurons, 4, true, true, 2);
    } else {
      n1 = SetNeuron(image_in, neuron_mask, neurons, 1, true, true, 1);
      n2 = SetNeuron(image_in, neuron_mask, neurons, 2, true, true, 2);
      n3 = SetNeuron(image_in, neuron_mask, neurons, 3, true, true, 3);
      n4 = SetNeuron(image_in, neuron_mask, neurons, 4, true, true, 4);
    }
    
    if (!n1 || !n2 || !n3 || !n4) {
        return false;
    }

    // Find the ROI using the mask - Do this on a smaller version of the image for speed.
    // Perform some augmentation by moving the ROI around a bit. Save these augs for the masking
    // that comes later as the mask must match
    // ROI is larger here than final as we need 'rotation' space

    if (!options.noroi) {
        float half_roi = static_cast<float>(options.roi_xy) / 2.0;
        int d = static_cast<int>(ceil(sqrt(2.0f * half_roi * half_roi))) * 2;
        int depth = static_cast<int>(ceil(static_cast<float>(d) / options.depth_scale));

        // Because we are going to AUG, we make the ROI a bit bigger so we can rotate around
        ImageU8L3D smaller = Resize(neuron_mask, neuron_mask.width / 2, neuron_mask.height / 2, neuron_mask.depth / 2);
        ROI roi_found = FindROI(smaller, d / 2, depth / 2);
        roi.x = roi_found.x * 2;
        roi.y = roi_found.y * 2;
        roi.z = roi_found.z * 2; 
        roi.xy_dim = roi_found.xy_dim * 2;
        roi.depth = roi_found.depth * 2;

        std::cout << tiff_path << ",ROI," << libcee::ToString(roi.x) << "," << libcee::ToString(roi.y) << "," << libcee::ToString(roi.z) << "," << roi.xy_dim << "," << roi.depth << std::endl;
        neuron_mask = Crop(neuron_mask, roi.x, roi.y, roi.z, roi.xy_dim, roi.xy_dim, roi.depth);
    } else {
        ASSERT(false, "Must have ROI cropping when using augmentation.");
    }
  
    std::vector<glm::vec4> graph;
    std::vector<std::string> extra_data;
    bool graphaug = _ReadGraph(coord_path, graph, extra_data, roi);

    float rw = static_cast<float>(options.final_width) / static_cast<float>(options.roi_xy);
    float rh = static_cast<float>(options.final_height) / static_cast<float>(options.roi_xy);
    float rd = static_cast<float>(options.final_depth) / static_cast<float>(options.roi_xy);

    // Generate a set of rotations
    ROTS.clear();
    glm::quat q(1.0,0,0,0);
    ROTS.push_back(q);
    
    for (int i = 1; i < options.num_augs; i++) {
        ROTS.push_back(RandRot());
    }

    for (int i = 0; i < options.num_augs; i++){
        // Save the masks
        std::string aug_id  = libcee::IntToStringLeadingZeroes(i, 2);
        std::string csv_line =  libcee::IntToStringLeadingZeroes(image_idx, 5) + "_" + aug_id + ", ";
        std::string output_path = options.output_path + "/" +  libcee::IntToStringLeadingZeroes(image_idx, 5) + "_" + aug_id + "_mask.fits";
        ImageU8L3D prefinal = Augment(neuron_mask, ROTS[i], options.roi_xy, options.depth_scale, false, false);
        ImageU8L mipped = Project(prefinal, ProjectionType::MAX_INTENSITY);

        std::vector<glm::vec4> tgraph;
        // Not sure why the inverse. GLM versus our sampling I suppose
        if (graphaug) {
            AugmentGraph(graph, tgraph, glm::inverse(ROTS[i]), neuron_mask.width, options.roi_xy, options.depth_scale);
        }

        ImageU8L resized = Resize(mipped, options.final_width, options.final_height);
        FlipVerticalI(resized);

        if (options.flatten){
            SaveFITS(output_path, resized);
        } else {
            if (prefinal.depth % 2 == 1) {
                prefinal.data.pop_back();
            }
            ImageU8L3D resized3d = Resize(prefinal, options.final_width, options.final_height, options.final_depth);
            FlipVerticalI(resized3d);
            SaveFITS(output_path, resized3d);
        }

        // Write a JPG just in case
        ImageU8L jpeged = Convert<ImageU8L>(Convert<ImageF32L>(resized));
        std::string output_path_jpg = options.output_path + "/" +  libcee::IntToStringLeadingZeroes(image_idx, 5) + "_" + aug_id + "_mask.jpg";
        SaveJPG(output_path_jpg, jpeged);
    
        // Write out to the CSV file
        // TODO - save graph to DB instead
        /* 
        for (int j = 0; j < 4; j++){
            glm::vec4 av = tgraph[j];
            std::string x = libcee::ToString(av.x * rw);
            std::string y = libcee::ToString((static_cast<float>(options.roi_xy) - av.y) * rh);
            std::string z = libcee::ToString(av.z * rd);
            csv_line += x + "," + y + "," + z + "," + extra_data[j] + ",";
        }

        csv_line += tiff_path;
        out_stream << csv_line << std::endl;
        */
    }

    return true;
}


/**
 * @brief Process the various files required to create a mask. Don't perform any aug.
 * 
 * @param options 
 * @param tiff_path 
 * @param log_path 
 * @param coord_path 
 * @param image_idx 
 * @param roi 
 * @return true 
 * @return false 
 */

bool ProcessMaskNoAug(Options &options, std::string &tiff_path, std::string &log_path, std::string &coord_path, int image_idx, ROI &roi) {
    ImageU16L image_in = LoadTiff<ImageU16L>(tiff_path);
    std::vector<std::vector<size_t>> neurons; // 0: None, 1: ASI-1, 2: ASI-2, 3: ASJ-1, 4: ASJ-2
    size_t idx = 0;

    // Read the log file and extract the neuron numbers
    // Making the asssumption that all log files have the neurons in the same order
    for (int i = 0; i < 5; i++) {
        neurons.push_back(std::vector<size_t>());
    }

    std::vector<std::string> lines_log = libcee::ReadFileLines(log_path);

    for (std::string line : lines_log) {
        std::vector<std::string> tokens = libcee::SplitStringWhitespace(line);
        if (libcee::ToLower(tokens[0]) == "associate") {
            idx += 1;
        } else {
            neurons[idx].push_back(libcee::FromString<size_t>(tokens[0]));
        }
    }

    // Join all our neurons
    ImageU8L3D neuron_mask(image_in.width, image_in.height / options.stacksize, options.stacksize);

    bool n1 = false, n2 = false, n3 = false, n4 = false;

    if(options.threeclass){
      n1 = SetNeuron(image_in, neuron_mask, neurons, 1, true, true, 1);
      n2 = SetNeuron(image_in, neuron_mask, neurons, 2, true, true, 1);
      n3 = SetNeuron(image_in, neuron_mask, neurons, 3, true, true, 2);
      n4 = SetNeuron(image_in, neuron_mask, neurons, 4, true, true, 2);
    } else {
      n1 = SetNeuron(image_in, neuron_mask, neurons, 1, true, true, 1);
      n2 = SetNeuron(image_in, neuron_mask, neurons, 2, true, true, 2);
      n3 = SetNeuron(image_in, neuron_mask, neurons, 3, true, true, 3);
      n4 = SetNeuron(image_in, neuron_mask, neurons, 4, true, true, 4);
    }
    
    if (!n1 || !n2 || !n3 || !n4) {
        return false;
    }

    // Find the ROI using the mask - Do this on a smaller version of the image for speed.
    // Perform some augmentation by moving the ROI around a bit. Save these augs for the masking
    // that comes later as the mask must match
    // ROI is larger here than final as we need 'rotation' space

    if (!options.noroi) { 
        int half_roi_xy = static_cast<int>(ceil(static_cast<float>(options.roi_xy) / 2.0));
        int half_roi_depth = static_cast<int>(ceil(static_cast<float>(options.roi_depth) / 2.0));

        // Because we are going to AUG, we make the ROI a bit bigger so we can rotate around
        ImageU8L3D smaller = Resize(neuron_mask, static_cast<int>(ceil(static_cast<float>(neuron_mask.width) / 2.0)), 
                                        static_cast<int>(ceil(static_cast<float>(neuron_mask.height) / 2.0)), 
                                        static_cast<int>(ceil(static_cast<float>(neuron_mask.depth) / 2.0)));

        ROI roi_found = FindROI(smaller, half_roi_xy, half_roi_depth);
        roi.x = roi_found.x * 2;
        roi.y = roi_found.y * 2;
        roi.z = roi_found.z * 2;
        roi.xy_dim = options.roi_xy;
        roi.depth = options.roi_depth;

        // Check on ROI-Z as it's likely to not shift
        if (options.roi_depth == neuron_mask.depth) {
            roi.z = 0;
        }
        
        std::cout << tiff_path << ",ROI," << libcee::ToString(roi.x) << "," << libcee::ToString(roi.y) << "," << libcee::ToString(roi.z) << "," << roi.xy_dim << "," << roi.depth << std::endl;
        neuron_mask = Crop(neuron_mask, roi.x, roi.y, roi.z, roi.xy_dim, roi.xy_dim, roi.depth);
    }
 
    FlipVerticalI(neuron_mask);
    std::vector<glm::vec4> graph;
    std::vector<std::string> extra_data;
    bool graphaug = _ReadGraph(coord_path, graph, extra_data, roi);

    float rw = static_cast<float>(options.final_width) / static_cast<float>(options.roi_xy);
    float rh = static_cast<float>(options.final_height) / static_cast<float>(options.roi_xy);
    float rd = static_cast<float>(options.final_depth) / static_cast<float>(options.roi_depth);

    // Save the masks
    std::string output_path = options.output_path + "/" +  libcee::IntToStringLeadingZeroes(image_idx, 5) + "_mask.fits";
    ImageU8L mipped = Project(neuron_mask, ProjectionType::MAX_INTENSITY);

    // Not sure why the inverse. GLM versus our sampling I suppose
    ImageU8L resized = Resize(mipped, options.final_width, options.final_height);

    if (options.flatten){
        SaveFITS(output_path, resized);
    } else {
        if (options.final_width != neuron_mask.width || options.final_depth != neuron_mask.depth || options.final_height != neuron_mask.height) {
            if (neuron_mask.depth % 2 == 1) {
                neuron_mask.data.pop_back();
            }

            ImageU8L3D resized3d = Resize(neuron_mask, options.final_width, options.final_height, options.final_depth);
            SaveFITS(output_path, resized3d);
        } else {
            SaveFITS(output_path, neuron_mask);
        }
    }

    // Write a JPG just in case
    // JPG uses bottom left as origin point, not top left as FITS generally does so it will look vertically flipped.
    ImageU8L jpeged = Convert<ImageU8L>(Convert<ImageF32L>(resized));
    std::string output_path_jpg = options.output_path + "/" +  libcee::IntToStringLeadingZeroes(image_idx, 5) + "_mask.jpg";
    SaveJPG(output_path_jpg, jpeged);

    // Write out to the CSV file
    // TODO - no more CSV I'd say - move to DB
    /*
    for (int j = 0; j < 4; j++){
        glm::vec4 av = graph[j];
        std::string x = libcee::ToString(av.x * rw);
        std::string y = libcee::ToString((static_cast<float>(options.roi_xy) - av.y) * rh);
        std::string z = libcee::ToString(av.z * rd);
        csv_line += x + "," + y + "," + z + "," + extra_data[j] + ",";
    }

    csv_line += tiff_path; */

    return true;
}
