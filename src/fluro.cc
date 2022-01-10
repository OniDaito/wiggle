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
#include <masamune/util/string.h>
#include <masamune/util/file.h>
#include <vector>

using namespace masamune;

// Our command line options, held in a struct.
typedef struct {
    std::string log_path = ".";
    std::string prefix_path = ".";
    std::string match = ".";
} Options;



/**
 * Given a tiff file and a log file, create a set of 
 * images for each neuron we are interested in.
 * 
 * @param options - the options struct
 * @param log_path - the file path to the corresponding .dat file
 *
 * @return bool if success or not
 */

void ProcessLog(std::string &log_path, std::string &replace, std::string &prefix) {
    std::vector<std::string> lines = util::ReadFileLines(log_path);

    for (std::string line : lines) {
        if (util::StringBeginsWith(line, "Renaming") && !util::StringContains(line, "AutoStack")) {

            //Renaming /phd/wormz/queelim/ins-6-mCherry_2/Annotations/LH ARZ analysis/20170907-QL285-d1.0/ID42_WS2.tiff to 01794
            std::string tpath = util::StringRemove(line, "Renaming ");
            std::vector<std::string> tokens = util::SplitStringString(tpath, " to ");

            size_t idx = util::FromString<size_t>(tokens[1]);
            std::string dat_path = util::StringRemove(tokens[0], "_WS2.tiff") + "_2.dat";
            std::string final_path = util::StringReplace(dat_path, replace, prefix); 
            std::vector<std::string> dat_lines = util::ReadFileLines(final_path);
            size_t asi_1 = 0;
            size_t asi_2 = 0;
            size_t asj_1 = 0;
            size_t asj_2 = 0;

            // Now process the lines in the dat files to grab the fluorescence
            for (std::string dat_line : dat_lines) {
                std::vector<std::string> dats = util::SplitStringWhitespace(dat_line);
                if (util::StringContains(dat_line, "ASI-1")) { asi_1 = util::FromString<size_t>(dats[1]); }
                if (util::StringContains(dat_line, "ASI-2")) { asi_2 = util::FromString<size_t>(dats[1]); }
                if (util::StringContains(dat_line, "ASJ-1")) { asj_1 = util::FromString<size_t>(dats[1]); }
                if (util::StringContains(dat_line, "ASJ-2")) { asj_2 = util::FromString<size_t>(dats[1]); }
            }

            std::cout << util::ToString(idx) << "," << util::ToString(asi_1) <<  "," << util::ToString(asi_2) << "," << util::ToString(asj_1) << "," << util::ToString(asj_2) << "," << final_path << std::endl;
             
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