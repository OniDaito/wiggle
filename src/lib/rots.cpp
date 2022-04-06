/**
 * ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
 * █░███░██▄██░▄▄▄█░▄▄▄█░██░▄▄
 * █▄▀░▀▄██░▄█░█▄▀█░█▄▀█░██░▄▄
 * ██▄█▄██▄▄▄█▄▄▄▄█▄▄▄▄█▄▄█▄▄▄
 * ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀
 * @file rots.cpp
 * @author Benjamin Blundell - k1803390@kcl.ac.uk
 * @date 25/02/2022
 * @brief Useful data finding functions
 * 
 *
 */

#include "rots.hpp"

using namespace imagine;

std::default_random_engine RANDROT_GENERATOR;
std::uniform_real_distribution<float> RANDROT_DISTRIB(0.0f,1.0f);

glm::quat RandRot() {
    float u1 = RANDROT_DISTRIB(RANDROT_GENERATOR);
    float u2 = RANDROT_DISTRIB(RANDROT_GENERATOR);
    float u3 = RANDROT_DISTRIB(RANDROT_GENERATOR);

    glm::quat q = glm::quat(
            static_cast <float>(sqrt(1.0 - u1) * sin(2.0 * M_PI * u2)),
            static_cast <float>(sqrt(1.0 - u1) * cos(2.0 * M_PI * u2)),
            static_cast <float>(sqrt(u1) * sin(2.0 * M_PI * u3)),
            static_cast <float>(sqrt(u1) * cos(2.0 * M_PI * u3))
    );
    q = glm::normalize(q);
    return q;
}

