
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


#include "PhysicsContactManager.h"
#include "PhysicsBox.h"
#include "PhysicsBody.h"
#include "PhysicsContact.h"
#include "PhysicsScene.h"
#include "PhysicsRender.h"

//--------------------------------------------------------------------------------------------------
// PhysicsContactManager
//--------------------------------------------------------------------------------------------------
PhysicsContactManager::PhysicsContactManager( PhysicsStack* stack )
	: m_stack( stack )
	, m_allocator( sizeof( PhysicsContactConstraint ), 256 )
	, m_broadphase( this )
{
	m_contactList = NULL;
	m_contactCount = 0;
	m_contactListener = NULL;
}

//--------------------------------------------------------------------------------------------------
void PhysicsContactManager::AddContact( PhysicsBox *A, PhysicsBox *B )
{
	PhysicsBody *bodyA = A->body;
	PhysicsBody *bodyB = B->body;
	if ( !bodyA->CanCollide( bodyB ) )
		return;

	// Search for existing matching contact
	// Return if found duplicate to avoid duplicate constraints
	// Mark pre-existing duplicates as active
	PhysicsContactEdge* edge = A->body->m_contactList;
	while ( edge )
	{
		if ( edge->other == bodyB )
		{
			PhysicsBox *shapeA = edge->constraint->A;
			PhysicsBox *shapeB = edge->constraint->B;

			// @TODO: Verify this against Box2D; not sure if this is all we need here
			if( (A == shapeA) && (B == shapeB) )
				return;
		}

		edge = edge->next;
	}

	// Create new contact
	PhysicsContactConstraint *contact = (PhysicsContactConstraint*)m_allocator.Allocate( );
	contact->A = A;
	contact->B = B;
	contact->bodyA = A->body;
	contact->bodyB = B->body;
	contact->manifold.SetPair( A, B );
	contact->m_flags = 0;
	contact->friction = PhysicsMixFriction( A, B );
	contact->restitution = PhysicsMixRestitution( A, B );
	contact->manifold.contactCount = 0;

	for ( int i = 0; i < 8; ++i )
		contact->manifold.contacts[ i ].warmStarted = 0;

	contact->prev = NULL;
	contact->next = m_contactList;
	if ( m_contactList )
		m_contactList->prev = contact;
	m_contactList = contact;

	// Connect A
	contact->edgeA.constraint = contact;
	contact->edgeA.other = bodyB;

	contact->edgeA.prev = NULL;
	contact->edgeA.next = bodyA->m_contactList;
	if ( bodyA->m_contactList )
		bodyA->m_contactList->prev = &contact->edgeA;
	bodyA->m_contactList = &contact->edgeA;

	// Connect B
	contact->edgeB.constraint = contact;
	contact->edgeB.other = bodyA;

	contact->edgeB.prev = NULL;
	contact->edgeB.next = bodyB->m_contactList;
	if ( bodyB->m_contactList )
		bodyB->m_contactList->prev = &contact->edgeB;
	bodyB->m_contactList = &contact->edgeB;

	bodyA->SetToAwake( );
	bodyB->SetToAwake( );

	++m_contactCount;
}

//--------------------------------------------------------------------------------------------------
void PhysicsContactManager::FindNewContacts( )
{
	m_broadphase.UpdatePairs( );
}

//--------------------------------------------------------------------------------------------------
void PhysicsContactManager::RemoveContact( PhysicsContactConstraint *contact )
{
	PhysicsBody *A = contact->bodyA;
	PhysicsBody *B = contact->bodyB;

	// Remove from A
	if ( contact->edgeA.prev )
		contact->edgeA.prev->next = contact->edgeA.next;

	if ( contact->edgeA.next )
		contact->edgeA.next->prev = contact->edgeA.prev;

	if ( &contact->edgeA == A->m_contactList )
		A->m_contactList = contact->edgeA.next;

	// Remove from B
	if ( contact->edgeB.prev )
		contact->edgeB.prev->next = contact->edgeB.next;

	if ( contact->edgeB.next )
		contact->edgeB.next->prev = contact->edgeB.prev;

	if ( &contact->edgeB == B->m_contactList )
		B->m_contactList = contact->edgeB.next;

	A->SetToAwake( );
	B->SetToAwake( );

	// Remove contact from the manager
	if ( contact->prev )
		contact->prev->next = contact->next;

	if ( contact->next )
		contact->next->prev = contact->prev;

	if ( contact == m_contactList )
		m_contactList = contact->next;

	--m_contactCount;

	m_allocator.Free( contact );
}

//--------------------------------------------------------------------------------------------------
void PhysicsContactManager::RemoveContactsFromBody( PhysicsBody *body )
{
	PhysicsContactEdge* edge = body->m_contactList;

	while( edge )
	{
		PhysicsContactEdge* next = edge->next;
		RemoveContact( edge->constraint );
		edge = next;
	}
}

//--------------------------------------------------------------------------------------------------
void PhysicsContactManager::RemoveFromBroadphase( PhysicsBody *body )
{
	PhysicsBox* box = body->m_boxes;

	while ( box )
	{
		m_broadphase.RemoveBox( box );
		box = box->next;
	}
}

//--------------------------------------------------------------------------------------------------
void PhysicsContactManager::TestCollisions( void )
{
	PhysicsContactConstraint* constraint = m_contactList;

	while( constraint )
	{
		PhysicsBox *A = constraint->A;
		PhysicsBox *B = constraint->B;
		PhysicsBody *bodyA = A->body;
		PhysicsBody *bodyB = B->body;

		constraint->m_flags &= ~PhysicsContactConstraint::eIsland;

		if( !bodyA->IsAwake( ) && !bodyB->IsAwake( ) )
		{
			constraint = constraint->next;
			continue;
		}

		if ( !bodyA->CanCollide( bodyB ) )
		{
			PhysicsContactConstraint* next = constraint->next;
			RemoveContact( constraint );
			constraint = next;
			continue;
		}

		// Check if contact should persist
		if ( !m_broadphase.TestOverlap( A->broadPhaseIndex, B->broadPhaseIndex ) )
		{
			PhysicsContactConstraint* next = constraint->next;
			RemoveContact( constraint );
			constraint = next;
			continue;
		}
		PhysicsManifold* manifold = &constraint->manifold;
		PhysicsManifold oldManifold = constraint->manifold;
		glm::vec3 ot0 = oldManifold.tangentVectors[ 0 ];
		glm::vec3 ot1 = oldManifold.tangentVectors[ 1 ];
		constraint->SolveCollision( );
		PhysicsComputeBasis( manifold->normal, manifold->tangentVectors, manifold->tangentVectors + 1 );

		for ( int i = 0; i < manifold->contactCount; ++i )
		{
			PhysicsContact *c = manifold->contacts + i;
			c->tangentImpulse[ 0 ] = c->tangentImpulse[ 1 ] = c->normalImpulse = float( 0.0 );
			unsigned char oldWarmStart = c->warmStarted;
			c->warmStarted = 0;

			for ( int j = 0; j < oldManifold.contactCount; ++j )
			{
				PhysicsContact *oc = oldManifold.contacts + j;
				if ( c->fp.key == oc->fp.key )
				{
					c->normalImpulse = oc->normalImpulse;

					// Attempt to re-project old friction solutions
					glm::vec3 friction = ot0 * oc->tangentImpulse[ 0 ] + ot1 * oc->tangentImpulse[ 1 ];
					c->tangentImpulse[ 0 ] = glm::dot( friction, manifold->tangentVectors[ 0 ] );
					c->tangentImpulse[ 1 ] = glm::dot( friction, manifold->tangentVectors[ 1 ] );
					c->warmStarted = glm::max( (int)oldWarmStart, (int)(oldWarmStart + 1) );
					break;
				}
			}
		}

		if ( m_contactListener )
		{
			int now_colliding = constraint->m_flags & PhysicsContactConstraint::eColliding;
			int was_colliding = constraint->m_flags & PhysicsContactConstraint::eWasColliding;

			if ( now_colliding && !was_colliding )
				m_contactListener->BeginContact( constraint );

			else if ( !now_colliding && was_colliding )
				m_contactListener->EndContact( constraint );
		}

		constraint = constraint->next;
	}
}

//--------------------------------------------------------------------------------------------------
void PhysicsContactManager::RenderContacts( PhysicsRender* render ) const
{
	const PhysicsContactConstraint *contact = m_contactList;

	while ( contact )
	{
		const PhysicsManifold *m = &contact->manifold;

		if ( !(contact->m_flags & PhysicsContactConstraint::eColliding) )
		{
			contact = contact->next;
			continue;
		}

		for ( int j = 0; j < m->contactCount; ++j)
		{
			const PhysicsContact *c = m->contacts + j;
			float blue = (float)(255 - c->warmStarted) / 255.0f;
			float red = 1.0f - blue;
			render->SetScale( 10.0f, 10.0f, 10.0f );
			render->SetPenColor( red, blue, blue );
			render->SetPenPosition( c->position.x, c->position.y, c->position.z );
			render->Point( );

			if ( m->A->body->IsAwake( ) )
				render->SetPenColor( 1.0f, 1.0f, 1.0f );
			else
				render->SetPenColor( 0.2f, 0.2f, 0.2f );

			render->SetPenPosition( c->position.x, c->position.y, c->position.z );
			render->Line(
				c->position.x + m->normal.x * 0.5f,
				c->position.y + m->normal.y * 0.5f,
				c->position.z + m->normal.z * 0.5f
				);
		}

		contact = contact->next;
	}

	render->SetScale( 1.0f, 1.0f, 1.0f );
}
