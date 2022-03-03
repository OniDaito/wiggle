/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file data.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 24/02/2022
 * @brief Useful data finding functions
 * 
 *
 */

#include "data.hpp"

using namespace masamune;

/**
 * Given a path to the output, return the index for the next
 * run.
 *
 * @param image_path - the output path
 *
 * @return std::vector<std::string> a list of log files
 */

int GetOffetNumber(std::string output_path) {
    std::vector<std::string> files = util::ListFiles(output_path);
    int count  = 0;

    for (std::string filename : files) {
        if (util::StringContains(filename, "layered")) {
            count += 1;
        }
    }

    return count;
}


/**
 * Given a path to the annotations directory find the log files
 * that hold the data we need.
 *
 * @param annotation_path - the file path to the annotations
 *
 * @return std::vector<std::string> a list of log files
 */

std::vector<std::string> FindLogFiles(std::string annotation_path) {
    std::vector<std::string> log_files;
    std::vector<std::string> files_anno = util::ListFiles(annotation_path);

    for (std::string filename : files_anno) {
        if (util::StringContains(filename, ".log")) {
            log_files.push_back(filename);
        }
    }

    return log_files;
}


/**
 * Given a path to the annotations directory find the tiff 
 * images with the watershedded categories.
 *
 * @param annotation_path - the file path to the annotations
 *
 * @return std::vector<std::string> a list of tiff files
 */

std::vector<std::string> FindAnnotations(std::string annotation_path) {
    std::vector<std::string> files_anno = util::ListFiles(annotation_path);
    std::vector<std::string> tiff_anno_files;

    for (std::string filename : files_anno) {
        if (util::StringContains(filename, ".tif") && util::StringContains(filename, "ID")  && util::StringContains(filename, "WS")) {
            tiff_anno_files.push_back(filename);
        }
    }

    // Apparently we are missing one for some reason. Oh well
    // assert(tiff_files.size() == log_files.size());

    // We use a sort based on the last ID number - keeps it inline with the flatten program
    struct {
        bool operator()(std::string a, std::string b) const {
            std::vector<std::string> tokens1 = util::SplitStringChars(util::FilenameFromPath(a), "_.-");
            int idx = 0;
            for (std::string t : tokens1) {
                if (util::StringContains(t, "ID")){
                    break;
                }
                idx += 1;
            }
            int ida = util::FromString<int>(util::StringRemove(tokens1[idx], "ID"));
            std::vector<std::string> tokens2 = util::SplitStringChars(util::FilenameFromPath(b), "_.-");
            int idb = util::FromString<int>(util::StringRemove(tokens2[idx], "ID"));
            return ida < idb;
        }
    } SortOrderAnno;

    std::sort(tiff_anno_files.begin(), tiff_anno_files.end(), SortOrderAnno);
    return tiff_anno_files;
}

/**
 * Given a path to the raw input directory find the tiff images
 * that hold the data we need.
 *
 * @param image - the file path to the raw input data
 *
 * @return std::vector<std::string> a list of log files
 */

std::vector<std::string> FindInputFiles(std::string image_path) {
    // Browse the directory looking for files
    std::vector<std::string> files_input = util::ListFiles(image_path);
    std::vector<std::string> tiff_input_files;

    for (std::string filename : files_input) {
        if (util::StringContains(filename, ".tif") && util::StringContains(filename, "AutoStack")) {
            tiff_input_files.push_back(filename);
        }
    }

    // We use a sort based on the last ID number - keeps it inline with the sort above
    struct {
        bool operator()(std::string a, std::string b) const {
            std::vector<std::string> tokens1 = util::SplitStringChars(util::FilenameFromPath(a), "_.-");
            int idx = 0;
            for (std::string t : tokens1) {
                if (util::StringContains(t, "AutoStack")) {
                    break;
                }
                idx += 1;
            }

            int ida = util::FromString<int>(util::StringRemove(tokens1[idx], "0xAutoStack"));
            std::vector<std::string> tokens2 = util::SplitStringChars(util::FilenameFromPath(b), "_.-");
            int idb = util::FromString<int>(util::StringRemove(tokens2[idx], "0xAutoStack"));
            return ida < idb;
        }
    } SortOrderInput;

    std::sort(tiff_input_files.begin(), tiff_input_files.end(), SortOrderInput);
    return tiff_input_files;
}
