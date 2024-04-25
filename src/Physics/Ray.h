#pragma once

#include <glm/glm.hpp>

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;

    inline Ray() : direction(0.0f, 0.0f, 1.0f),origin{0,0,0} {}

    inline Ray(const glm::vec3 &o, const glm::vec3 &d) : origin(o), direction(d)
    {
        NormalizeDirection();
    }
    inline glm::vec3 NormalizeDirection()
    {
        return glm::normalize(direction);
    }
};