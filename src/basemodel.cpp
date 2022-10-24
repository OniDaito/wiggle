/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file basemodel.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 21/10/2022
 * @brief Go through all centroids and create the best average
 * 
 * We want good initial placement for our points to ensure
 * HOLLy has the best chance of mapping, so let's construct
 * a system of linear equations and get the positions as
 * close to the average as possible.
 * 
 * This program assumes you have run the DB program to
 * create a postgresql database for all the worm annotation
 * data.
 *
 */


#include "basemodel.hpp"
 
using namespace pqxx;

// Our command line options, held in a struct.
typedef struct {
    std::string image_path = ".";
} Options;


float median(std::vector<float> &v) {
    std::sort(v.begin(), v.end(), std::greater<float>());
    size_t n = v.size() / 2;
    nth_element(v.begin(), v.begin()+n, v.end());
    return v[n];
}

/**
 * @brief Find the average distances between the neurons
 * 
 * @return Neurons 
 */
NeuronDists find_averages() {
    std::vector<glm::vec3> asi_1_positions;
    std::vector<glm::vec3> asi_2_positions;
    std::vector<glm::vec3> asj_1_positions;
    std::vector<glm::vec3> asj_2_positions;

    NeuronDists dists = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    try {
        connection C("dbname = phd user = postgres hostaddr = 127.0.0.1 port = 5432");

        if (C.is_open()) {
            std::cout << "Opened database successfully: " << C.dbname() << std::endl;
            char* sql = "select * from wormz";
            nontransaction N(C);
            result R( N.exec( sql ));

            for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
                glm::vec3 pos (c[4].as<float>(), c[5].as<float>(), c[6].as<float>());

                switch(c[12].as<int>()) {
                    case 1: {
                        asi_1_positions.push_back(pos);
                        break;
                    }
                    case 2: {
                        asi_2_positions.push_back(pos);
                        break;
                    }
                    case 3: {
                        asj_1_positions.push_back(pos);
                        break;
                    }
                    case 4: {
                        asj_2_positions.push_back(pos);
                        break;
                    }
                }
            }

        } else {
            std::cout << "Can't open database" << std::endl;
        }
    } catch (const std::exception &e) {
      std::cerr << e.what() << std::endl;
    }

    // Now we have our positions, lets find the dists.

    std::vector<float> asi1_asi2_v;
    std::vector<float> asi1_asj1_v;
    std::vector<float> asi1_asj2_v;
    std::vector<float> asi2_asj2_v;
    std::vector<float> asi2_asj1_v;
    std::vector<float> asj1_asj2_v;

    for (size_t i = 0; i < asi_1_positions.size(); i++) {
        glm::vec3 asi1 = asi_1_positions[i];
        glm::vec3 asi2 = asi_2_positions[i];
        glm::vec3 asj1 = asj_1_positions[i];
        glm::vec3 asj2 = asj_2_positions[i];

        float asi1_asi2 = glm::distance(asi1, asi2);
        float asi1_asj1 = glm::distance(asi1, asj1);
        float asi1_asj2 = glm::distance(asi1, asj2);
        float asi2_asj2 = glm::distance(asi2, asj2);
        float asi2_asj1 = glm::distance(asi2, asj2);
        float asj2_asj2 = glm::distance(asj2, asi2);

        // We don't want zeros as that indicates there was no neuron found
        if (glm::length(asi1) > 0 && glm::length(asi2) > 0) { asi1_asi2_v.push_back(asi1_asi2); }
        if (glm::length(asi1) > 0 && glm::length(asj1) > 0) { asi1_asj1_v.push_back(asi1_asj1); }
        if (glm::length(asi1) > 0 && glm::length(asj2) > 0) { asi1_asj2_v.push_back(asi1_asj2); }
        if (glm::length(asi2) > 0 && glm::length(asj2) > 0) { asi2_asj2_v.push_back(asi2_asj2); }
        if (glm::length(asi2) > 0 && glm::length(asj1) > 0) { asi2_asj1_v.push_back(asi2_asj2); }
        if (glm::length(asj2) > 0 && glm::length(asj2) > 0) { asj1_asj2_v.push_back(asi2_asj2); }
    }

    dists.asi1_asi2 = median(asi1_asi2_v);
    dists.asi1_asj1 = median(asi1_asj1_v);
    dists.asi1_asj2 = median(asi1_asj2_v);
    dists.asi2_asj2 = median(asi2_asj2_v);
    dists.asi2_asj1 = median(asi2_asj1_v);
    dists.asj1_asj2 = median(asj1_asj2_v);

    return dists;
}

int main (int argc, char ** argv) {
    // Parse command line options
    Options options;
    int c;
    static struct option long_options[] = {
        {NULL, 0, NULL, 0}
    };

    int option_index = 0;
    int image_idx = 0;

    NeuronDists neurons_dists = find_averages();

    std::cout << "Average distances (median): " << neurons_dists.asi1_asi2 << ", " << neurons_dists.asi1_asj1 << ", " <<
        neurons_dists.asi1_asj2 << ", " << neurons_dists.asi2_asj2 << ", " << 
        neurons_dists.asi2_asj1 << ", " << neurons_dists.asj1_asj2 << std::endl;

    std::vector<double> xinit = {50, 50, 50, 50, 50, 50};    
    Neurons positions = solve_posititons(neurons_dists, xinit, -600, 600, 0.001);

    std::cout << "Positions Found - ASI-1: (" << positions.asi_1.x  << ", " << positions.asi_1.y << ", " << positions.asi_1.z << "), " <<
        "ASI-2: (" << positions.asi_2.x  << ", " << positions.asi_2.y << ", " << positions.asi_2.z << "), " << 
        "ASJ-1: (" << positions.asj_1.x  << ", " << positions.asj_1.y << ", " << positions.asj_1.z << "), " <<
        "ASI-2: (" << positions.asi_2.x  << ", " << positions.asi_2.y << ", " << positions.asi_2.z << ") " << std::endl;
}