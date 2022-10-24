#ifndef __BASEMODEL_H__
#define __BASEMODEL_H__

/**
 * @file basemodel.hpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 21/10/2022
 * @brief basemodel executable header
 *
 */

#include <libcee/string.hpp>
#include <libcee/file.hpp>
#include <libcee/threadpool.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <getopt.h>
#include <cstdlib>
#include <nlopt.hpp>
#include <pqxx/pqxx>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

typedef struct {
    float asi1_asi2;
    float asi1_asj1;
    float asi1_asj2;
    float asi2_asj2;
    float asi2_asj1;
    float asj1_asj2;
} NeuronDists;

typedef struct {
    glm::vec3 asi_1;
    glm::vec3 asi_2;
    glm::vec3 asj_1;
    glm::vec3 asj_2;
} Neurons;

Neurons solve_posititons(NeuronDists dists, std::vector<double> x, double upper_bound, double lower_bound, double term);
NeuronDists find_averages();
float median(std::vector<float> &v);


#endif