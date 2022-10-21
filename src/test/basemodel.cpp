#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "test/doctest.h"
#include "basemodel.hpp"


TEST_CASE("Testing Basemodel solver") {
    // Lets use a 3 x 4 x 5 triangle
    NeuronDists dists = {3.0, 4.0, 5.0, 4.0, 5.0, 3.0};

    std::vector<double> xinit = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};    
    Neurons positions = solve_posititons(dists, xinit, -10.0, 10.0, 1e-9);

    float da = glm::distance(positions.asi_1, positions.asi_2);
    float db = glm::distance(positions.asi_1, positions.asj_1);
    float dc = glm::distance(positions.asi_1, positions.asj_2);
    float dd = glm::distance(positions.asi_2, positions.asj_2);
    float de = glm::distance(positions.asi_2, positions.asj_1);
    float df = glm::distance(positions.asj_1, positions.asj_2);

    CHECK(abs(da - dists.asi1_asi2) <= 0.3);
    CHECK(abs(db - dists.asi1_asj1) <= 0.3);
    CHECK(abs(dc - dists.asi1_asj2) <= 0.3);
    CHECK(abs(dd - dists.asi2_asj2) <= 0.3);
    CHECK(abs(de - dists.asi2_asj1) <= 0.3);
    CHECK(abs(df - dists.asj1_asj2) <= 0.3);

}