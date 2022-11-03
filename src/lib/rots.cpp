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


// Returns the graph but in augmented co-ordinates to match the images

void AugmentGraph(std::vector<glm::vec4> const &graph, std::vector<glm::vec4> &rgraph, glm::quat rot, size_t image_dim, size_t final_dim, float zscale) {
    assert(graph.size() == 4);
    assert(rgraph.size() == 0);

    for (int i = 0; i < 4; i++) {
        float fz = static_cast <float>(graph[i].z * zscale);
        float fx = static_cast <float>(graph[i].x);
        float fy = static_cast <float>(graph[i].y);

        fx = (fx / static_cast <float>(image_dim) * 2.0) - 1.0;
        fy = (fy / static_cast <float>(image_dim) * 2.0) - 1.0;
        fz = (fz / static_cast <float>(image_dim) * 2.0) - 1.0;

        glm::mat4 rotmat = glm::toMat4(rot);
        glm::vec4 v = glm::vec4(fx, fy, fz, 1.0);
        v = rotmat * v;

        glm::vec4 tg ( static_cast<float>((v.x + 1.0) / 2.0 * image_dim),
            static_cast<float>((v.y + 1.0) / 2.0 * image_dim),
           static_cast<float>( (v.z + 1.0) / 2.0 * image_dim), 1.0f);

        // Augmenting cuts down the input image a bit so we must adjust 
        // ROI accordingly
        float roi_shift = ( image_dim - final_dim ) / 2.0f;
  
        tg.x = tg.x - roi_shift;
        tg.y = tg.y - roi_shift;
        tg.z = tg.z - roi_shift;

        rgraph.push_back(tg);

        // Note we don't undo Zscale, as augimage above does not flatten the Z back down either.
    }

}
