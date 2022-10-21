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

typedef struct {
    double a, b, c, d, e, f;
} my_base_data;


// x would be our 6 parameters for the positions (a,b,c ... etc)
double myfunc(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data) {
    
    my_base_data *base = reinterpret_cast<my_base_data*>(my_func_data);

    // Target distances
    double ta = base->a;
    double tb = base->b;
    double tc = base->c;
    double td = base->d;
    double te = base->e;
    double tf = base->f;

    // Candidate positions of the four neurons
    double pa[3] = {0, 0, 0};
    double pb[3] = {x[0], 0, 0}; 
    double pc[3] = {x[1], x[2], 0}; 
    double pd[3] = {x[3], x[4], x[5]}; 

    // Candidate 6 distances
    double da = sqrt(pow(pa[0]- pb[0], 2) + pow(pa[1]- pb[1], 2) + pow(pa[2]- pb[2], 2));
    double db = sqrt(pow(pa[0]- pc[0], 2) + pow(pa[1]- pc[1], 2) + pow(pa[2]- pc[2], 2));
    double dc = sqrt(pow(pa[0]- pd[0], 2) + pow(pa[1]- pd[1], 2) + pow(pa[2]- pd[2], 2));
    double dd = sqrt(pow(pb[0]- pd[0], 2) + pow(pb[1]- pd[1], 2) + pow(pb[2]- pd[2], 2));
    double de = sqrt(pow(pb[0]- pc[0], 2) + pow(pb[1]- pc[1], 2) + pow(pb[2]- pc[2], 2));
    double df = sqrt(pow(pc[0]- pd[0], 2) + pow(pc[1]- pd[1], 2) + pow(pc[2]- pd[2], 2));

    double loss = pow(da - ta, 2) + pow(db - tb, 2) + pow(dc - tc, 2) + pow(dd - td, 2) + pow(de - te, 2) + pow(df - tf, 2);
    std::cout << "Loss: " << loss << std::endl;
    return loss;
}


/**
 * @brief Given 6 distances, minimise the distances between them.
 * Distances should be normalised.
 * 
 * @param dists 
 * @param x - initial positions
 * @param upper_bound - the upper bound on x
 * @param lower_bound - the lower bound on x
 * @return Neurons 
 */
Neurons solve_posititons(NeuronDists dists, std::vector<double> x, double upper_bound, double lower_bound) {
    // Initial positions we will try to minimise
    Neurons positions = {
        glm::vec3(0, 0, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(1, 1, 1)
    };

    my_base_data base = {dists.asi1_asi2, dists.asi1_asj1, dists.asi1_asj2, dists.asi2_asj1, dists.asi2_asj2, dists.asj1_asj2};

    nlopt::opt opt(nlopt::GN_DIRECT, 6);
    // Lower and upper bounds
    std::vector<double> lb = {upper_bound, upper_bound, upper_bound, upper_bound, upper_bound, upper_bound};
    std::vector<double> ub = {lower_bound, lower_bound, lower_bound, lower_bound, lower_bound, lower_bound};
    opt.set_lower_bounds(lb);
    opt.set_upper_bounds(ub);
    opt.set_min_objective(myfunc, &base);
    opt.set_stopval(1e-4);

    // Initial parameters for a, b, c, d, e and f.
    double minf;

    try{
        nlopt::result result = opt.optimize(x, minf);
        std::cout << "found minimum at f(" << x[0] << "," << x[1] << "," << x[2] << ","  << x[3] << ","  << x[4] << ","  << x[5] << ") = "
            << std::setprecision(10) << minf << std::endl;

        positions.asi_2.x =  x[0];
        positions.asj_1.x =  x[1];
        positions.asi_2.y =  x[2];
        positions.asi_2.x =  x[3];
        positions.asi_2.y =  x[4];
        positions.asi_2.z =  x[5];
    
    }
    catch(std::exception &e) {
        std::cout << "nlopt failed: " << e.what() << std::endl;
    }

    return positions;
}


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

    NeuronDists neurons = find_averages();

    std::cout << "Average distances (median): " << neurons.asi1_asi2 << ", " << neurons.asi1_asj1 << ", " <<
        neurons.asi1_asj2 << ", " << neurons.asi2_asj2 << ", " << 
        neurons.asi2_asj1 << ", " << neurons.asj1_asj2 << std::endl;

    
    std::vector<double> xinit = {100, 100, 100, 100, 100, 100};    
    Neurons positions = solve_posititons(neurons, xinit, 600, 0);

    std::cout << "Positions Found - ASI-1: (" << positions.asi_1.x  << ", " << positions.asi_1.y << ", " << positions.asi_1.z << "), " <<
        "ASI-2: (" << positions.asi_2.x  << ", " << positions.asi_2.y << ", " << positions.asi_2.z << "), " << 
        "ASJ-1: (" << positions.asj_1.x  << ", " << positions.asj_1.y << ", " << positions.asj_1.z << "), " <<
        "ASI-2: (" << positions.asi_2.x  << ", " << positions.asi_2.y << ", " << positions.asi_2.z << ") " << std::endl;
}