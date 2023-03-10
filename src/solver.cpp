/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file solver.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 21/10/2022
 * @brief Go through all centroids and create the best average
 * 
 * We want good initial placement for our points to ensure
 * HOLLy has the best chance of mapping, so let's construct
 * a system of nonlinear equations and get the positions as
 * close to the average as possible.
 * 
 * Based on nlopt library: https://nlopt.readthedocs.io/en/latest/
 *
 */


#include "basemodel.hpp"

typedef struct {
    double a, b, c, d;
} my_base_data;


// x would be our 6 parameters for the positions (a,b,c ... etc)
double myfunc(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data) {
    
    my_base_data *base = reinterpret_cast<my_base_data*>(my_func_data);

    // Target distances
    double ta = base->a;
    double tb = base->b;
    double tc = base->c;
    double td = base->d;

    // Candidate positions of the four neurons
    double pa[3] = {0, 0, 0};
    double pb[3] = {x[0], 0 , 0}; 
    double pc[3] = {x[1], x[2], 0}; 
    double pd[3] = {x[3], x[4], x[5]};

    // Candidate 6 distances
    double da = sqrt(pow(pa[0]- pb[0], 2) + pow(pa[1]- pb[1], 2) + pow(pa[2]- pb[2], 2));
    double db = sqrt(pow(pc[0]- pd[0], 2) + pow(pc[1]- pd[1], 2) + pow(pc[2]- pd[2], 2));
    double dc = sqrt(pow(pa[0]- pc[0], 2) + pow(pa[1]- pc[1], 2) + pow(pa[2]- pc[2], 2)) + sqrt(pow(pa[0]- pd[0], 2) + pow(pa[1]- pd[1], 2) + pow(pa[2]- pd[2], 2));
    double dd = sqrt(pow(pb[0]- pc[0], 2) + pow(pb[1]- pc[1], 2) + pow(pb[2]- pc[2], 2)) + sqrt(pow(pb[0]- pd[0], 2) + pow(pb[1]- pd[1], 2) + pow(pb[2]- pd[2], 2));

    double loss = pow(da - ta, 2) + pow(db - tb, 2) + pow(dc - tc, 2) + pow(dd - td, 2);
    //std::cout << "Loss: " << loss << std::endl;
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
 * @param term - terminiating loss condition
 * @return Neurons 
 */
Neurons solve_posititons(NeuronDists dists, std::vector<double> x, double upper_bound, double lower_bound, double term) {
    // Initial positions we will try to minimise
    Neurons positions = {
        glm::vec3(0, 0, 0),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 0, 0)
    };

    // We are using 9 parameters. We could use 6, with x, xy, and xyz for the last 3 neurons but for now, sticking with 9.
    my_base_data base = {dists.asi1_asi2, dists.asj1_asj2, dists.asi1_asj12, dists.asi2_asj12};

    nlopt::opt opt(nlopt::GN_DIRECT_L_RAND, 6);
    // Lower and upper bounds
    std::vector<double> lb = {upper_bound, upper_bound, upper_bound, upper_bound, upper_bound, upper_bound};
    std::vector<double> ub = {lower_bound, lower_bound, lower_bound, lower_bound, lower_bound, lower_bound};
    opt.set_lower_bounds(lb);
    opt.set_upper_bounds(ub);
    opt.set_min_objective(myfunc, &base);
    opt.set_stopval(term);

    double minf;

    try{
        nlopt::result result = opt.optimize(x, minf);
        std::cout << "found minimum at f(" << x[0] << "," << x[1] << "," << x[2] << ","  << x[3] << ","  << x[4] << ","  << x[5] << ") = "
            << std::setprecision(10) << minf << std::endl;

        positions.asi_2.x = x[0];
 
        positions.asj_1.x = x[1];
        positions.asj_1.y = x[2];
 
        positions.asj_2.x = x[3];
        positions.asj_2.y = x[4];
        positions.asj_2.z = x[5];
    
    }
    catch(std::exception &e) {
        std::cout << "nlopt failed: " << e.what() << std::endl;
    }

    return positions;
}
