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

#include <stdio.h>

#include "PhysicsSettings.h"
#include "PhysicsMemory.h"
#include "PhysicsContactManager.h"
#include "PhysicsContact.h"

class PhysicsBody;
struct PhysicsBodyDef;
struct PhysicsContactConstraint;
class PhysicsRender;
struct PhysicsIsland;

class PhysicsContactListener
{
public:
	virtual ~PhysicsContactListener()
	{
	}

	virtual void BeginContact(const PhysicsContactConstraint *contact) = 0;
	virtual void EndContact(const PhysicsContactConstraint *contact) = 0;
};

class PhysicsQueryCallback
{
public:
	virtual ~PhysicsQueryCallback()
	{
	}

	virtual bool ReportShape(PhysicsBox *box) = 0;
};

class PhysicsScene
{
public:
	PhysicsScene(float dt, const glm::vec3 &gravity = glm::vec3(float(0.0), float(-9.8), float(0.0)), int iterations = 20);
	~PhysicsScene();

	// Run the simulation forward in time by dt (fixed timestep). Variable
	// timestep is not supported.
	void Step( float deltaTime);

	// Construct a new rigid body. The BodyDef can be reused at the user's
	// discretion, as no reference to the BodyDef is kept.
	PhysicsBody *CreateBody(const PhysicsBodyDef &def);

	// Frees a body, removes all shapes associated with the body and frees
	// all shapes and contacts associated and attached to this body.
	void RemoveBody(PhysicsBody *body);
	void RemoveAllBodies();

	// Enables or disables rigid body sleeping. Sleeping is an effective CPU
	// optimization where bodies are put to sleep if they don't move much.
	// Sleeping bodies sit in memory without being updated, until the are
	// touched by something that wakes them up. The default is enabled.
	void SetAllowSleep(bool allowSleep);

	// Increasing the iteration count increases the CPU cost of simulating
	// Scene.Step(). Decreasing the iterations makes the simulation less
	// realistic (convergent). A good iteration number range is 5 to 20.
	// Only positive numbers are accepted. Non-positive and negative
	// inputs set the iteration count to 1.
	void SetIterations(int iterations);

	// Friction occurs when two rigid bodies have shapes that slide along one
	// another. The friction force resists this sliding motion.
	void SetEnableFriction(bool enabled);

	// Render the scene with an interpolated time between the last frame and
	// the current simulation step.
	void Render(PhysicsRender *render) const;

	// Gets and sets the global gravity vector used during integration
	const glm::vec3 GetGravity() const;
	void SetGravity(const glm::vec3 &gravity);

	// Removes all bodies from the scene.
	void Shutdown();

	// Sets the listener to report collision start/end. Provides the user
	// with a pointer to an PhysicsContactConstraint. The PhysicsContactConstraint
	// holds pointers to the two shapes involved in a collision, and the
	// two bodies connected to each shape. The PhysicsContactListener will be
	// called very often, so it is recommended for the funciton to be very
	// efficient. Provide a NULL pointer to remove the previously set
	// listener.
	void SetContactListener(PhysicsContactListener *listener);

	// Query the world to find any shapes that can potentially intersect
	// the provided AABB. This works by querying the broadphase with an
	// AAABB -- only *potential* intersections are reported. Perhaps the
	// user might use lmDistance as fine-grained collision detection.
	void QueryAABB(PhysicsQueryCallback *cb, const PhysicsAABB &aabb) const;

	// Query the world to find any shapes intersecting a world space point.
	void QueryPoint(PhysicsQueryCallback *cb, const glm::vec3 &point) const;

	// Query the world to find any shapes intersecting a ray.
	void RayCast(PhysicsQueryCallback *cb, PhysicsRaycastData &rayCast) const;

	// Dump all rigid bodies and shapes into a log file. The log can be
	// used as C++ code to re-create an initial scene setup. Contacts
	// are *not* logged, meaning any cached resolution solutions will
	// not be saved to the log file. This means the log file will be most
	// accurate when dumped upon scene initialization, instead of mid-
	// simulation.
	void Dump(FILE *file) const;

	PhysicsBody *BodyList() { return m_bodyList; }

private:
	PhysicsContactManager m_contactManager;
	PhysicsPagedAllocator m_boxAllocator;

	int m_bodyCount;
	PhysicsBody *m_bodyList;

	PhysicsStack m_stack;
	PhysicsHeap m_heap;

	glm::vec3 m_gravity;
	float m_dt;
	int m_iterations;

	bool m_newBox;
	bool m_allowSleep;
	bool m_enableFriction;

	friend class PhysicsBody;
};
