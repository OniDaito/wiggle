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


ROI RandROI(ROI roi) {
    ROI rroi = roi;
    roi.x = -10 + rand() % 20;
    roi.y = -10 + rand() % 20;

    rroi.x = std::max(int(roi.x),0);
    rroi.y = std::max(int(roi.x),0);

    return rroi;
}

/**
 * @brief The Image processing pipeline for source images.
 * Perform a Crop, noise subtraction and deconvolution
 *
 * 
 * @param image_in 
 * @param roi 
 * @return ImageF32L3D 
 */

ImageF32L3D ProcessPipe(ImageU16L3D const &image_in, bool autoback, float noise, bool deconv, const std::string &psf_path, int deconv_rounds, int &background, bool contrast) {
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

    float min, max;
    MinMax(converted, min, max);
    float range = max - min;
    std::function<float (float)> contrast_func = [min, range](float x) { return ((x - min) / range) * 4096; };


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
            deconved.data.push_back(last_slice);
            deconved.depth += 1;
        }

        if (contrast) {
            deconved = ApplyFunc<ImageF32L3D, float>(deconved, contrast_func);
        }

        return deconved;
    }

    if (contrast) {
        converted = ApplyFunc<ImageF32L3D, float>(converted, contrast_func);
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

void _AugSource(const Options &options, ImageF32L3D &processed, const Transform &master_t, const std::vector<Transform> &trans, std::string image_id) {
    // Now perform some rotations, sum, normalise, contrast then renormalise for the final 2D image
    // Thread this bit for a bit more speed
    std::string output_path = options.output_path + "/" + image_id + "_layered.fits";
    libcee::ThreadPool pool{ static_cast<size_t>( options.num_augs) };
    std::vector<std::future<int>> futures;

    for (int i = 0; i < options.num_augs; i++){
        futures.push_back(pool.execute( [i, output_path, trans, options, image_id, processed] () {  
            // Rotate, normalise then sum projection
            std::string aug_id  = libcee::IntToStringLeadingZeroes(i, 2);
            std::string output_path = options.output_path + "/" + image_id + "_" + aug_id + "_layered.fits";
            glm::quat q = trans[i].rot;
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

int TiffToFits(const Options &options, const Transform &master_t, const std::vector<Transform> &trans, std::string &tiff_path, int image_idx) {
    ImageU16L image = LoadTiff<ImageU16L>(tiff_path); 
    ImageU16L3D stacked(image.width, (image.height / (options.stacksize * options.channels)), options.stacksize);
    uint coff = 0;

    // Convert the TIFF into internal 3D image format
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

    // Filename for the new image
    std::vector<std::string> tokens_log = libcee::SplitStringChars(libcee::FilenameFromPath(tiff_path), "_.-");
    std::string image_id = tokens_log[3];
    image_id = libcee::StringRemove(image_id, "0xAutoStack");

    if (options.rename == true) {
        image_id  = libcee::IntToStringLeadingZeroes(image_idx, 5);
        std::string output_path = options.output_path + "/" + image_id + "_layered.fits";
        std::cout << "Renaming " << tiff_path << " to " << output_path << std::endl;
    }

    ImageF32L3D converted;

    // Do we have an ROI? If so, perform the master transform (a crop).
    // This saves time as we don't have to perform many processes like deconv multiple times
    if (!options.noroi) {
        stacked = Crop(stacked, master_t.roi.x, master_t.roi.y, master_t.roi.z, master_t.roi.xy_dim, master_t.roi.xy_dim, master_t.roi.depth);
    }

    int background = options.cutoff;

    if (!options.noprocess){
        if (options.otsu){
            auto thresh = imagine::Otsu(stacked);
            std::function<uint16_t (uint16_t)> thresh_func = [thresh](uint16_t x) { if(x >= thresh) { return x;} return static_cast<uint16_t>(0); };
            stacked = ApplyFunc<ImageU16L3D, uint16_t>(stacked, thresh_func);
            converted = imagine::Convert<ImageF32L3D>(stacked);
        } else {
            converted = ProcessPipe(stacked, options.autoback, options.cutoff, options.deconv, options.psf_path, options.deconv_rounds, background, options.contrast);
        }
    } else  {
        converted = imagine::Convert<ImageF32L3D>(stacked);
    }


    // By this point we have our master cropped and processed image (deconv, noise, etc. From here we can pe)
    if (options.num_augs > 1) {
        _AugSource(options, converted, master_t, trans, image_id);
    } else {
        _NoAugSource(options, converted, image_id);
    }

    // TODO - this is not ideal really. We should probably reconsider interfaces
    return background;
}


bool ProcessMask(Options &options, std::string &tiff_path, std::string &log_path, std::string &coord_path, int image_idx, Transform &master_t, std::vector<Transform> &transforms) {
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
    size_t start_height = image_in.height / options.stacksize;
    ImageU8L3D neuron_mask(image_in.width, start_height, options.stacksize);
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
        // Must always have 4 neurons to label
        return false;
    }

    // Find the ROI using the mask - Do this on a smaller version of the image for speed.
    // ROI is larger here than final as we need 'rotation' and 'translation' space
    ROI master_roi;

    if (!options.noroi) {
        float half_roi = static_cast<float>(options.roi_xy) / 2.0;
        int d = static_cast<int>(ceil(sqrt(2.0f * half_roi * half_roi))) * 2;
        int depth = static_cast<int>(ceil(static_cast<float>(d) / options.depth_scale));

        // Because we are going to AUG, we make the ROI a bit bigger so we can rotate OR translate around
        ImageU8L3D smaller = Resize(neuron_mask, neuron_mask.width / 2, neuron_mask.height / 2, neuron_mask.depth / 2);
        ROI roi_found = FindROI(smaller, d / 2, depth / 2);
        master_roi.x = roi_found.x * 2;
        master_roi.y = roi_found.y * 2;
        master_roi.z = roi_found.z * 2; 
        master_roi.xy_dim = roi_found.xy_dim * 2;
        master_roi.depth = roi_found.depth * 2;
        std::cout << tiff_path << ",MasterROI," << libcee::ToString(master_roi.x) << "," << libcee::ToString(master_roi.y) << "," << libcee::ToString(master_roi.z) << "," << master_roi.xy_dim << "," << master_roi.depth << std::endl;
        neuron_mask = Crop(neuron_mask, master_roi.x, master_roi.y, master_roi.z, master_roi.xy_dim, master_roi.xy_dim, master_roi.depth);
    } else if (options.num_augs > 1) {
        ASSERT(false, "Must have ROI cropping when using augmentation.");
    }
  
    // Generate a set of rotations
    transforms.clear();
    glm::quat q(1.0,0,0,0);

    // The master transform from within all other transforms take place
    master_t.roi = master_roi;
    master_t.rot = q;

    // Now create new transforms from the master transform. The first transform is never rotated and
    // the crop is always the found Region of interest.
    // This ROI is an offset from the master one. We crop twice essentially.
    ROI r;
    r.depth = options.roi_depth;
    r.xy_dim = options.roi_xy;
    r.x = (master_roi.xy_dim - options.roi_xy) / 2;
    r.y = (master_roi.xy_dim - options.roi_xy) / 2;

    // If no ROI, just set all to zero
    if (options.noroi) {
        r.x = 0;
        r.y = 0;
        r.z = 0;
        r.depth = 0;
        r.xy_dim = 0;
    }

    Transform t;
    t.roi = r;
    t.rot = q;
    transforms.push_back(t);

    for (int i = 1; i < options.num_augs; i++) {
        Transform tt;
        ROI tr;
        glm::quat tq(1.0,0,0,0);

        if (options.safeaug) {
            tr = RandROI(r);
        }
        else {
            tq = RandRot();
        }

        tt.roi = tr;
        tt.rot = tq;
        transforms.push_back(tt);
    }

    for (int i = 0; i < options.num_augs; i++){
        // Save the masks
        std::string aug_id  = libcee::IntToStringLeadingZeroes(i, 2);
        std::string csv_line =  libcee::IntToStringLeadingZeroes(image_idx, 5) + "_" + aug_id + ", ";
        std::string output_path = options.output_path + "/" +  libcee::IntToStringLeadingZeroes(image_idx, 5) + "_" + aug_id + "_mask.fits";

        ImageU8L3D prefinal = neuron_mask;
   
        if (!options.noroi) {
            if (options.safeaug) {
                Transform trans = transforms[i];
                prefinal = Crop(neuron_mask, trans.roi.x, trans.roi.y, trans.roi.z, trans.roi.xy_dim, trans.roi.xy_dim, trans.roi.depth);
            } else {
                prefinal = Augment(neuron_mask, transforms[i].rot, options.roi_xy, options.depth_scale, false, false);
            }
        } 
        
        ImageU8L mipped = Project(prefinal, ProjectionType::MAX_INTENSITY);
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
    }

    return true;
}
