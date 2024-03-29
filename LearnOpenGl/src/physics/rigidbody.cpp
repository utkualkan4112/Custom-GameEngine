#include "rigidbody.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

// test for equivalence of two rigid bodies
bool RigidBody::operator==(RigidBody rb)
{
	return instanceId == rb.instanceId;
}

// test for equivalence of two rigid bodies
bool RigidBody::operator==(std::string id)
{
	return instanceId == id;
}

/*
	constructor
*/

// construct with parameters and default
RigidBody::RigidBody(std::string modelId, glm::vec3 size, float mass, glm::vec3 pos, glm::vec3 rot)
	: modelId(modelId), size(size), mass(mass), pos(pos), velocity(0.0f), acceleration(0.0f), state(0), rot(rot),
	lastCollision(COLLISION_THRESHOLD), lastCollisionID("") {
	update(0.0f);
}

/*
	transformation functions
*/

// update position with velocity and acceleration
void RigidBody::update(float dt, bool gun)
{
	if (velocity != glm::vec3(0.0f) && acceleration != glm::vec3(0.0f)) {
		pos += velocity * dt + 0.5f * acceleration * (dt * dt);
	}


	velocity += acceleration * dt;

	// rotation
	if (!gun) {
		rotationMatrix = glm::toMat4(glm::quat(rot));
	}

	// model = translation * rotation * scale
	model = glm::translate(glm::mat4(1.0f), pos);
	model = model * rotationMatrix;
	model = glm::scale(model, size);

	normalModel = glm::transpose(glm::inverse(glm::mat3(model)));

	lastCollision += dt;
}

// apply a force
void RigidBody::applyForce(glm::vec3 force)
{
	acceleration += force / mass;
}

// apply a force
void RigidBody::applyForce(glm::vec3 direction, float magnitude)
{
	applyForce(direction * magnitude);
}

// apply an acceleration (remove redundancy of dividing by mass)
void RigidBody::applyAcceleration(glm::vec3 a)
{
	acceleration += a;
}

// apply an acceleration (remove redundancy of dividing by mass)
void RigidBody::applyAcceleration(glm::vec3 direction, float magnitude)
{
	applyAcceleration(direction * magnitude);
}

// apply force over time
void RigidBody::applyImpulse(glm::vec3 force, float dt)
{
	velocity += force / mass * dt;
}

// apply force over time
void RigidBody::applyImpulse(glm::vec3 direction, float magnitude, float dt)
{
	applyImpulse(direction * magnitude, dt);
}

// transfer potential or kinetic energy from another object
void RigidBody::transferEnergy(float joules, glm::vec3 direction)
{
	if (joules == 0) {
		return;
	}

	// comes from formula: KE = 1 / 2 * m * v^2

	glm::vec3 deltaV = sqrt(2 * abs(joules) / mass) * direction;

	velocity += joules > 0 ? deltaV : -deltaV;
}

/*
	collisions
*/
void RigidBody::handleCollision(RigidBody* inst, glm::vec3 norm)
{
	if (lastCollision >= COLLISION_THRESHOLD || lastCollisionID != inst->instanceId) {
		if (this->modelId == "sphere") {
			if (glm::any(glm::isnan(this->velocity)) && glm::length(inst->velocity) > 0.1f) {
				// Get the mass and velocity of the moving sphere
				float movingMass = inst->mass;
				glm::vec3 movingVelocity = inst->velocity;

				// Get the mass of the stationary sphere
				float stationaryMass = this->mass;

				// Calculate the resulting velocity of the stationary sphere using conservation of momentum
				glm::vec3 resultingVelocity = (2.0f * movingMass * movingVelocity) / (movingMass + stationaryMass);

				// Apply the resulting velocity to the stationary sphere
				this->velocity = resultingVelocity;

				applyAcceleration(Environment::gravitationalAcceleration);
			}
			else {
				// Reflect the velocity using the collision normal
				this->velocity = glm::reflect(this->velocity, glm::normalize(norm));

				// Apply friction to the velocity
				float frictionCoefficient = 0.2f; // Adjust this value to control friction
				glm::vec3 frictionForce = -frictionCoefficient * glm::length(this->velocity) * glm::normalize(this->velocity);
				this->velocity += frictionForce;

				// Reset counter
				lastCollision = 0.0f;

				// Check if the hit surface is perpendicular to gravity (assuming -y is the gravity direction)
				float gravityDotProduct = glm::dot(glm::vec3(0.0f, -1.0f, 0.0f), glm::normalize(norm));
				if (std::abs(gravityDotProduct) > 0.9f) { // Adjust the threshold as needed


					// Check if the velocity magnitude is below a threshold
					float velocityThreshold = 0.5f; // Adjust this value to control when the object stops
					if (glm::length(velocity) < velocityThreshold) {
						// Set the vertical velocity to zero to prevent the object from sinking into the ground
						acceleration.y += 9.81f;

						// Set the velocity to zero to make the object stay still
						velocity = glm::vec3(0.0f);
					}
				}
			}
			
		}
	}
	lastCollisionID = inst->instanceId;
}

void RigidBody::apllyAirFriction(float dt)
{
	applyImpulse(glm::vec3(-velocity.x, 0, -velocity.z), 0.5, dt);
	
}
