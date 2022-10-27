#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "test/doctest.h"
#include "basemodel.hpp"


TEST_CASE("Testing Basemodel solver") {
    // Lets use a 3 x 4 x 5 triangle
    NeuronDists dists = {4.0, 4.0, 8.0, 8.0};

    std::vector<double> xinit = {1.0, 5.0, 4.0, 3.0, 5.0, 3.0};    
    Neurons positions = solve_posititons(dists, xinit, -50.0, 50.0, 1e-30);

    float da = glm::distance(positions.asi_1, positions.asi_2);
    float db = glm::distance(positions.asj_1, positions.asj_2);
    float dc = glm::distance(positions.asi_1, positions.asj_1) + glm::distance(positions.asi_1, positions.asj_2);
    float dd = glm::distance(positions.asi_2, positions.asj_1) + glm::distance(positions.asi_2, positions.asj_2);
  
    CHECK(abs(da - dists.asi1_asi2) <= 0.3);
    CHECK(abs(db - dists.asj1_asj2) <= 0.3);
    CHECK(abs(dc - dists.asi1_asj12) <= 0.3);
    CHECK(abs(dd - dists.asi2_asj12) <= 0.3);

}