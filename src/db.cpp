/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file db.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 09/08/2022
 * @brief Write stats to a postgresql db
 * 
 * 
 *
 */

#include <libcee/string.hpp>
#include <libcee/file.hpp>
#include <getopt.h>
#include <iostream>
#include <pqxx/pqxx>
#include "data.hpp"


using namespace pqxx;


// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
    std::string output_path = ".";
    std::string annotation_path = ".";
    std::string prefix = "";
} Options;


/**
 * Given a tiff file and a log file, create a set of 
 * images for each neuron we are interested in.
 * 
 * @param options - the options struct
 * @param tiff_path - the file path to the tiff
 * @param log_path - the file path to the corresponding .log file
 *
 * @return bool if success or not
 */


bool ProcessMask(Options &options, std::string &tiff_path, std::string &log_path, std::string &coord_path, int image_idx, connection &C) {

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

    // Read the dat file and write out the coordinates in order as an entry in a CSV file
    std::vector<std::string> lines = libcee::ReadFileLines(coord_path);
    if(lines.size() != 4) {  return false; }

    // Should be ASI-1, ASI-2, ASJ-1, ASJ-2

    int type_idx = 1;
    // Get the required data from the .dat files in the annotation
    for (std::string line : lines) {
        std::vector<std::string> tokens = libcee::SplitStringWhitespace(line);
        std::string x = libcee::RemoveChar(libcee::RemoveChar(tokens[4], ','), ' ');
        std::string y = libcee::RemoveChar(libcee::RemoveChar(libcee::RemoveChar(tokens[3], ','), ' '), '[');
        std::string z = libcee::RemoveChar(libcee::RemoveChar(libcee::RemoveChar(tokens[5], ','), ' '), ']');
    
        std::string fl = libcee::RemoveChar(libcee::RemoveChar(tokens[1], ','), ' ');
        std::string mode_fl = libcee::RemoveChar(libcee::RemoveChar(tokens[6], ','), ' ');
        std::string min_fl = libcee::RemoveChar(libcee::RemoveChar(tokens[7], ','), ' ');
        std::string bg = libcee::RemoveChar(libcee::RemoveChar(tokens[2], ','), ' ');
        std::string rsize = libcee::RemoveChar(libcee::RemoveChar(tokens[8], ','), ' ');
        std::string type = libcee::ToString(type_idx);

        try {
            // Create SQL statement
            std::string sql_string = "INSERT INTO wormz (tifffile, logfile, datfile, x, y, z, fl, mode_fl, bg, rsize, type) VALUES (";
            std::string D = ", ";
            sql_string += tiff_path + D + log_path + D + coord_path + D + x + D + y + D + z + D + fl + D + mode_fl + D + bg + D + rsize + D + type;
            sql_string += ")";
            // Create a transactional object
            work W(C);
            
            // Execute SQL query
            W.exec(sql_string.c_str());
            W.commit();
            std::cout << "Records created successfully" << std::endl;

        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }

        type_idx++;

    }

    return true;
}



int main (int argc, char ** argv) {
    // Parse command line options
    Options options;
    int c;
    static struct option long_options[] = {
        {"image-path", 1, 0, 0},
        {"output-path", 1, 0, 0},
        {"prefix", 1, 0, 0},
        {NULL, 0, NULL, 0}
    };

    int option_index = 0;
    int image_idx = 0;

    while ((c = getopt_long(argc, (char **)argv, "i:a:?", long_options, &option_index)) != -1) {
        switch (c) {
            case 0 :
                break;
            case 'i' :
                options.image_path = std::string(optarg);
                break;
            case 'a' :
                options.annotation_path = std::string(optarg);
                break;
        }
    }

    connection C("dbname = phd user = postgres hostaddr = 127.0.0.1 port = 5432");

    // Lets connect to the psql db
    try {
        if (C.is_open()) {
            std::cout << "Opened database successfully: " << C.dbname() << std::endl;
            // char * sql = "drop table wormz if exists;";

            // Create a transactional object
            work W(C);
            // Execute SQL query
            //W.exec( sql );
            //W.commit();

            // Create the table for our worm stats
            char * sql = "CREATE TABLE IF NOT EXISTS wormz("  \
                "id INT PRIMARY KEY  NOT NULL," \
                "tifffile     TEXT    NOT NULL," \
                "logfile      TEXT    NOT NULL," \
                "datfile     TEXT    NOT NULL," \
                "x            INT     NOT NULL," \
                "y            INT     NOT NULL," \
                "z            INT     NOT NULL," \
                "fl           INT     NOT NULL," \
                "mode_fl      INT     NOT NULL," \
                "min_fl       INT     NOT NULL," \
                "bg           INT     NOT NULL," \
                "rsize        INT     NOT NULL," \
                "type         INT     NOT NULL);";

            W.exec( sql );
            W.commit();
            std::cout << "Table created successfully" << std::endl;

        } else {
            std::cout << "Can't open database" << std::endl;
            return 1;
        }

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }


    std::cout << "Loading annotation images from " << options.annotation_path << std::endl;

    // First, find and sort the annotation files
    std::vector<std::string> tiff_anno_files = FindAnnotations(options.annotation_path);

    // Next, find the annotation dat files
    std::vector<std::string> dat_files = FindDatFiles(options.annotation_path);

    // Next, find the annotation log files
    std::vector<std::string> log_files = FindLogFiles(options.annotation_path);

    std::vector<std::string> tiff_input_files = FindInputFiles(options.image_path);

    // Pair up the tiffs with their log file and then the input and process them.
    for (std::string tiff_anno : tiff_anno_files) {
        bool paired = false;
        std::vector<std::string> tokens = libcee::SplitStringChars(libcee::FilenameFromPath(tiff_anno), "_.-");
        std::string id = tokens[0];

        for (std::string log : log_files) {
            std::vector<std::string> tokens_log = libcee::SplitStringChars(libcee::FilenameFromPath(log), "_.-");
            if (tokens_log[0] == id) {

                for (std::string dat : dat_files) {
                    std::vector<std::string> tokens_dat = libcee::SplitStringChars(libcee::FilenameFromPath(dat), "_.-");
                    if (tokens_dat[0] == id) {
                        
                        for (std::string tiff_input : tiff_input_files) {
                            // Find the matching input stack
                            std::vector<std::string> tokens1 = libcee::SplitStringChars(libcee::FilenameFromPath(tiff_input), "_.-");
                            int tidx = 0;

                            for (std::string t : tokens1) {
                                if (libcee::StringContains(t, "AutoStack")) {
                                    break;
                                }
                                tidx += 1;
                            }
                            
                            int ida = libcee::FromString<int>(libcee::StringRemove(tokens1[tidx], "0xAutoStack"));
                            int idb = libcee::FromString<int>(libcee::StringRemove(id, "ID"));

                            if (ida == idb) {
                                try {
                                    std::cout << "Masking: " << dat << std::endl;
                                    ProcessMask(options, tiff_anno, log, dat, image_idx, C);

                    
                                } catch (const std::exception &e) {
                                    std::cout << "An exception occured with" << tiff_anno << " and " <<  tiff_input << std::endl;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!paired){
            std::cout << "Failed to pair " << tiff_anno << std::endl;
        }
    }
    // Version 7 of the library (on Proto) doesnt have this
    // C.disconnect ();

    return EXIT_SUCCESS;

}