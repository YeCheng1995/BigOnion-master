#include "RaceGameComponent.h"
#include "../../engine/ECS/GameObject.h"

const std::string RaceGameComponent::typeID = "RaceGameComponent";


RaceGameComponent::RaceGameComponent()
{
	reachCheckPoint = 0;
}

void RaceGameComponent::update(float deltaTime)
{
	if (reachCheckPoint == checkpoints.size()) 
		return; // game finished

	glm::vec3 pos = gameObject->transform.position.getGlmVec3();
	float distanct = glm::distance(checkpoints[reachCheckPoint], pos);
	if (distanct < passDistance) {
		reachCheckPoint++; // next point
	}
	// printf("position=(%f, %f, %f), dist=%f , percentage=%f \n", pos.x, pos.y, pos.z, distanct, GetPercentage());
}

void RaceGameComponent::resetPercentage() {
	reachCheckPoint = 0;
}

float RaceGameComponent::GetPercentage() {
	return (float)reachCheckPoint / checkpoints.size();
}