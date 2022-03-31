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

glm::quat RandRot() {
    // Multiply by 0.25 to keep the rotations small
    srand(time(NULL));
    float u1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float u2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float u3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    glm::quat q = glm::quat(
            static_cast <float>(sqrt(1.0 - u1) * sin(2.0 * M_PI * u2)),
            static_cast <float>(sqrt(1.0 - u1) * cos(2.0 * M_PI * u2)),
            static_cast <float>(sqrt(u1) * sin(2.0 * M_PI * u3)),
            static_cast <float>(sqrt(u1) * cos(2.0 * M_PI * u3))
    );
    q = glm::normalize(q);
    return q;
}

