/*********************************************************************
Matt Marchant 2015
http://trederia.blogspot.com

pseuthe Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <PhysicsWorld.hpp>
#include <Util.hpp>

namespace
{
    const sf::FloatRect bounds(-50.f, 0.f, 2020.f, 1080.f);
}

PhysicsWorld::PhysicsWorld()
{}

PhysicsComponent::Ptr PhysicsWorld::addBody(float radius, MessageBus& m)
{
    auto body = std::make_unique<PhysicsComponent>(radius, m);
    m_bodies.push_back(body.get());
    return std::move(body);
}

void PhysicsWorld::handleMessages(const Message& msg)
{

}

void PhysicsWorld::update(float dt)
{
    //delete any destroyed bodies
    m_bodies.erase(std::remove_if(m_bodies.begin(), m_bodies.end(),
        [](const PhysicsComponent* p)
    {
        return p->destroyed();
    }), m_bodies.end());

    //find collision pairs
    m_collisions.clear();
    for (auto b1 : m_bodies)
    {
        for (auto b2 : m_bodies)
        {
            if (b1 != b2)
            {
                sf::Vector2f pairVector = b2->getPosition() - b1->getPosition();
                float distance = Util::Vector::lengthSquared(pairVector);
                float summedRadius = b1->getRadiusSquared() + b2->getRadiusSquared();
                if (distance < summedRadius)
                {
                    PhysicsComponent::Manifold manifold;
                    manifold.penetration = std::sqrt(summedRadius - distance) / 2.f;
                    manifold.normal = Util::Vector::normalise(pairVector);
                    manifold.transferForce = b1->getTransferForce();

                    b2->setManifold(manifold);
                    manifold.normal = -manifold.normal;
                    manifold.transferForce = b2->getTransferForce();
                    b1->setManifold(manifold);

                    m_collisions.insert(std::minmax(b1, b2));
                }
            }
        }
    }

    //resolve collisions
    for (const auto& c : m_collisions)
    {
        c.first->resolveCollision();
        c.second->resolveCollision();
    }

    //update bodies
    for (auto& b : m_bodies)
    {
        b->physicsUpdate(dt, bounds);
    }
}