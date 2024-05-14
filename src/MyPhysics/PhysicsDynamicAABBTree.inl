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


#include "PhysicsDynamicAABBTree.h"

inline void PhysicsDynamicAABBTree::AddToFreeList( int index )
{
	for ( int i = index; i < m_capacity - 1; ++i)
	{
		m_nodes[ i ].next = i + 1;
		m_nodes[ i ].height = Node::Null;
	}

	m_nodes[ m_capacity - 1 ].next = Node::Null;
	m_nodes[ m_capacity - 1 ].height = Node::Null;
	m_freeList = index;
}

//--------------------------------------------------------------------------------------------------
inline void PhysicsDynamicAABBTree::DeallocateNode( int index )
{
	assert( index >= 0 && index < m_capacity );

	m_nodes[ index ].next = m_freeList;
	m_nodes[ index ].height = Node::Null;
	m_freeList = index;

	--m_count;
}

//--------------------------------------------------------------------------------------------------
template <typename T>
inline void PhysicsDynamicAABBTree::Query( T *cb, const PhysicsAABB& aabb ) const
{
	const int k_stackCapacity = 256;
	int stack[ k_stackCapacity ];
	int sp = 1;

	*stack = m_root;

	while ( sp )
	{
		// k_stackCapacity too small
		assert( sp < k_stackCapacity );

		int id = stack[ --sp ];

		const Node *n = m_nodes + id;
		if ( PhysicsAABBtoAABB( aabb, n->aabb ) )
		{
			if ( n->IsLeaf( ) )
			{
				if ( !cb->TreeCallBack( id ) )
					return;
			}
			else
			{
				stack[ sp++ ] = n->left;
				stack[ sp++ ] = n->right;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------
template <typename T>
void PhysicsDynamicAABBTree::Query( T *cb, PhysicsRaycastData& rayCast ) const
{
	const float k_epsilon = float( 1.0e-6 );
	const int k_stackCapacity = 256;
	int stack[ k_stackCapacity ];
	int sp = 1;

	*stack = m_root;

	glm::vec3 p0 = rayCast.start;
	glm::vec3 p1 = p0 + rayCast.dir * rayCast.t;

	while( sp )
	{
		// k_stackCapacity too small
		assert( sp < k_stackCapacity );

		int id = stack[--sp];

		if ( id == Node::Null )
			continue;

		const Node *n = m_nodes + id;

		glm::vec3 e = n->aabb.max - n->aabb.min;
		glm::vec3 d = p1 - p0;
		glm::vec3 m = p0 + p1 - n->aabb.min - n->aabb.max;

		float adx = Abs( d.x );

		if ( Abs( m.x ) > e.x + adx )
			continue;

		float ady = Abs( d.y );

		if ( Abs( m.y ) > e.y + ady )
			continue;

		float adz = Abs( d.z );

		if ( Abs( m.z ) > e.z + adz )
			continue;

		adx += k_epsilon;
		ady += k_epsilon;
		adz += k_epsilon;

		if( Abs( m.y * d.z - m.z * d.y) > e.y * adz + e.z * ady )
			continue;

		if( Abs( m.z * d.x - m.x * d.z) > e.x * adz + e.z * adx )
			continue;

		if ( Abs( m.x * d.y - m.y * d.x) > e.x * ady + e.y * adx )
			continue;

		if ( n->IsLeaf( ) )
		{
			if ( !cb->TreeCallBack( id ) )
				return;
		}

		else
		{
			stack[ sp++ ] = n->left;
			stack[ sp++ ] = n->right;
		}
	}
}
