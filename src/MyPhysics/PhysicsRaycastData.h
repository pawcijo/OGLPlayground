//--------------------------------------------------------------------------------------------------
/**
@file	DropBoxes.h

@author	Randy Gaul
@date	11/25/2014

	Copyright (c) 2014 Randy Gaul http://www.randygaul.net

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
		1. The origin of this software must not be misrepresented; you must not
			 claim that you wrote the original software. If you use this software
			 in a product, an acknowledgment in the product documentation would be
			 appreciated but is not required.
		2. Altered source versions must be plainly marked as such, and must not
			 be misrepresented as being the original software.
		3. This notice may not be removed or altered from any source distribution.
*/
//--------------------------------------------------------------------------------------------------


#pragma once

#include <glm/glm.hpp>

class PhysicsRaycastData
{

public:
    glm::vec3 start; // Beginning point of the ray
    glm::vec3 dir;   // Direction of the ray (normalized)
    float t;         // Time specifying ray endpoint

    float toi;        // Solved time of impact
    glm::vec3 normal; // Surface normal at impact

    void Set(const glm::vec3 &startPoint, const glm::vec3 &direction, float endPointTime);

    // Uses toi, start and dir to compute the point at toi. Should
    // only be called after a raycast has been conducted with a
    // return value of true.
    const glm::vec3 GetImpactPoint() const;
};