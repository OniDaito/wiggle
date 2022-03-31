/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file fluro.cc
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 10/01/2022
 * @brief Fluro - write out the fluorsence scores.
 * 
 * Given a directory of worm information, summarise
 * the flourescence scores in a single CSV file.
 *
 */

#include <getopt.h>
#include <fitsio.h>
#include <libsee/string.hpp>
#include <libsee/file.hpp>
#include <imagine/imagine.hpp>
#include <vector>

using namespace imagine;

// Our command line options, held in a struct.
typedef struct {
    std::string log_path = ".";
    std::string prefix_path = ".";
    std::string match = ".";
} Options;



/**
 * Given the output log file from multiset, create a CSV of matching
 * Fluorescence values
 *
 * @param log_path - the file path to the corresponding .dat file
 * @param replace - the bit of the image paths to replace
 * @param prefix - what to replace with.
 *
 * @return None
 */

void ProcessLog(std::string &log_path, std::string &replace, std::string &prefix) {
    std::vector<std::string> lines = libsee::ReadFileLines(log_path);

    for (std::string line : lines) {
        if (libsee::StringBeginsWith(line, "Renaming") && !libsee::StringContains(line, "AutoStack")) {

            //Renaming /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170907-QL285-d1.0/ID42_WS2.tiff to 01794
            std::string tpath = libsee::StringRemove(line, "Renaming ");
            std::vector<std::string> tokens = libsee::SplitStringString(tpath, " to ");

            size_t idx = libsee::FromString<size_t>(tokens[1]);
            std::string dat_path = libsee::StringRemove(tokens[0], "_WS2.tiff") + "_2.dat";
            std::string final_path = libsee::StringReplace(dat_path, replace, prefix); 
            std::vector<std::string> dat_lines = libsee::ReadFileLines(final_path);
            size_t asi_1 = 0;
            size_t asi_2 = 0;
            size_t asj_1 = 0;
            size_t asj_2 = 0;

            // Now process the lines in the dat files to grab the fluorescence
            for (std::string dat_line : dat_lines) {
                std::vector<std::string> dats = libsee::SplitStringWhitespace(dat_line);
                if (libsee::StringContains(dat_line, "ASI-1")) { asi_1 = libsee::FromString<size_t>(dats[1]); }
                if (libsee::StringContains(dat_line, "ASI-2")) { asi_2 = libsee::FromString<size_t>(dats[1]); }
                if (libsee::StringContains(dat_line, "ASJ-1")) { asj_1 = libsee::FromString<size_t>(dats[1]); }
                if (libsee::StringContains(dat_line, "ASJ-2")) { asj_2 = libsee::FromString<size_t>(dats[1]); }
            }

            std::cout << libsee::ToString(idx) << "," << libsee::ToString(asi_1) <<  "," << libsee::ToString(asi_2) << "," << libsee::ToString(asj_1) << "," << libsee::ToString(asj_2) << "," << final_path << std::endl;
             
        }
    }
}


int main (int argc, char ** argv) {
    // Parse command line options
    Options options;
    int c;
    static struct option long_options[] = {
        {"log_path", 1, 0, 0},
        {NULL, 0, NULL, 0}
    };

    int option_index = 0;

    while ((c = getopt_long(argc, (char **)argv, "i:p:m:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'i' :
                options.log_path = std::string(optarg);
                break;
            case 'p' :
                options.prefix_path = std::string(optarg);
                break;
            case 'm' :
                options.match = std::string(optarg);
                break;
        }
    }

    ProcessLog(options.log_path, options.match, options.prefix_path);
}