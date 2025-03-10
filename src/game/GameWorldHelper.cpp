#include "GameWorldHelper.h"
#include "../engine/BOEngine.h"
#include "../engine/ECS/ComponentManager.h"
#include "../src/game/components/AudioPlayerComponent.h"
#include "components/TypeTestComponent.h"
#include "components/CarControlComponent.h"
#include "../../Settings.h"
#include "components/RaceGameComponent.h"

#include "../../Settings.h"

#include "yaml-cpp/yaml.h"
#include "../object_loader.h"
#include <vector>
#include <thread>

auto shaders = std::unordered_map<std::string, Shader*>();

/*
	The main Menu Scene which cantains audio only.
*/
void GameWorldHelper::initMenuScene(BOEngine* engine)
{
	delete engine->gameWorld;
	engine->gameWorld = nullptr;

	std::string strategy[] = { CarControlComponent::typeID, RigidBodyComponent::typeID, RenderComponent::typeID, RaceGameComponent::typeID };
	engine->gameWorld = new GameWorld(strategy, 4, 1.0 / 60.0);

	// std::cout << "Game init" << std::endl;
	AudioEngine audio;
	Camera camera;

	//background music
	audio.Shutdown();
	audio.Init();

	//engine sound
	GameObject* engine_sound = new  GameObject("EngineSound");
	engine_sound->transform.position = { camera.Position.x, camera.Position.y, camera.Position.z };
	engine->gameWorld->addGameObject(engine_sound);
	engine_sound->addComponent(new AudioPlayerComponent(audio, "game/assets/sounds/startup.wav", 1, true, false, false));
	engine_sound->getComponent<AudioPlayerComponent>()->onAddToGameWorld();
	engine_sound->getComponent<AudioPlayerComponent>()->volume(1);

	engine_sound->addComponent(new AudioPlayerComponent(audio, "game/assets/sounds/idle.wav", 1, true, true, false));
	engine_sound->getComponent<AudioPlayerComponent>()->onAddToGameWorld();
	engine_sound->getComponent<AudioPlayerComponent>()->volume(0.3);
	engine_sound->getComponent<AudioPlayerComponent>()->setSpeed(0);

	//skid sound
	GameObject* skid_sound = new  GameObject("SkidSound");
	skid_sound->transform.position = { camera.Position.x, camera.Position.y, camera.Position.z };
	engine->gameWorld->addGameObject(skid_sound);
	skid_sound->addComponent(new AudioPlayerComponent(audio, "game/assets/sounds/car_break.mp3", 0.5, true, true, false));
	skid_sound->getComponent<AudioPlayerComponent>()->onAddToGameWorld();

	//impact sound
	GameObject* impact_sound_small = new  GameObject("SmallImpact");
	impact_sound_small->transform.position = { camera.Position.x, camera.Position.y, camera.Position.z };
	engine->gameWorld->addGameObject(impact_sound_small);
	impact_sound_small->addComponent(new AudioPlayerComponent(audio, "game/assets/sounds/small_impact.mp3", 30, true, true, false));
	impact_sound_small->getComponent<AudioPlayerComponent>()->onAddToGameWorld();

	GameObject* impact_sound_big = new  GameObject("BigImpact");
	impact_sound_big->transform.position = { camera.Position.x, camera.Position.y, camera.Position.z };
	engine->gameWorld->addGameObject(impact_sound_big);
	impact_sound_big->addComponent(new AudioPlayerComponent(audio, "game/assets/sounds/big_impact.mp3", 20, true, true, false));
	impact_sound_big->getComponent<AudioPlayerComponent>()->onAddToGameWorld();

	//background music
	GameObject* background_music = new  GameObject("BackgroundMusic");
	engine->gameWorld->addGameObject(background_music);
	background_music->addComponent(new AudioPlayerComponent(audio, "game/assets/sounds/start.mp3", 1, false, true, false));
	background_music->getComponent<AudioPlayerComponent>()->onAddToGameWorld();
	background_music->getComponent<AudioPlayerComponent>()->volume(0.3, true);

	//win lose music
	GameObject* win1_music = new  GameObject("WinMusic1");
	engine->gameWorld->addGameObject(win1_music);
	win1_music->addComponent(new AudioPlayerComponent(audio, "game/assets/sounds/win1.mp3", 1, false, true, false));
	win1_music->getComponent<AudioPlayerComponent>()->onAddToGameWorld();

	GameObject* win2_music = new  GameObject("WinMusic2");
	engine->gameWorld->addGameObject(win2_music);
	win2_music->addComponent(new AudioPlayerComponent(audio, "game/assets/sounds/win2.mp3", 1, false, true, false));
	win2_music->getComponent<AudioPlayerComponent>()->onAddToGameWorld();

	GameObject* lose1_music = new  GameObject("LoseMusic1");
	engine->gameWorld->addGameObject(lose1_music);
	lose1_music->addComponent(new AudioPlayerComponent(audio, "game/assets/sounds/lose1.mp3", 1, false, true, false));
	lose1_music->getComponent<AudioPlayerComponent>()->onAddToGameWorld();

	GameObject* lose2_music = new  GameObject("LoseMusic2");
	engine->gameWorld->addGameObject(lose2_music);
	lose2_music->addComponent(new AudioPlayerComponent(audio, "game/assets/sounds/lose2.mp3", 1, false, true, false));
	lose2_music->getComponent<AudioPlayerComponent>()->onAddToGameWorld();
}
/*
	Loads a test scene into the given BOEngine.
*/
void GameWorldHelper::initTestScene(BOEngine* engine)
{
	auto start = std::chrono::steady_clock::now();

	delete engine->gameWorld;
	engine->gameWorld = nullptr;

	// std::cout << "Game init" << std::endl;
	AudioEngine audio;
	Camera camera;

	Shader* shader;
	if (shaders.count("model") > 0)
	{
		shader = shaders["model"];
	}
	else
	{
		shader = shaders["model"] = new Shader("engine/graphic/shader/model_loading.vs", "engine/graphic/shader/model_loading.fs");
	}

	Shader* lightshader;
	if (shaders.count("light") > 0)
	{
		lightshader = shaders["light"];
	}
	else
	{
		lightshader = shaders["light"] = new Shader("engine/graphic/shader/model_loading.vs", "engine/graphic/shader/light.fs.glsl");
	}

	audio.Shutdown();
	audio.Init();

	engine->skybox.load(Skybox_Load("game/assets/objects/sky_ground.yaml"));

	shader->use();
	shader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f); //obj to light
	shader->setVec3("dirLight.ambient", 0.35f, 0.35f, 0.35f);
	shader->setVec3("dirLight.diffuse", 1.0f, 1.0f, 1.0f);
	shader->setVec3("dirLight.specular", 0.8f, 0.8f, 0.8f);

	glm::vec3 pointLightPositions[] = {
		glm::vec3(16.0f,  5.0f,  0.0f),
		glm::vec3(12.3f, 5.3f, 5.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	// point light 1
	shader->setVec3("pointLights[0].position", pointLightPositions[0]);
	shader->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	shader->setVec3("pointLights[0].diffuse", 1.0f, 1.0f, 1.0f);
	shader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	shader->setFloat("pointLights[0].constant", 1.0f);
	shader->setFloat("pointLights[0].linear", 0.09);
	shader->setFloat("pointLights[0].quadratic", 0.032);

	// point light 2
	shader->setVec3("pointLights[1].position", pointLightPositions[1]);
	shader->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	shader->setVec3("pointLights[1].diffuse", 1.0f, 1.0f, 1.0f);
	shader->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	shader->setFloat("pointLights[1].constant", 1.0f);
	shader->setFloat("pointLights[1].linear", 0.09);
	shader->setFloat("pointLights[1].quadratic", 0.032);

	std::string strategy[] = {CarControlComponent::typeID, RigidBodyComponent::typeID, RenderComponent::typeID, RaceGameComponent::typeID };
	engine->gameWorld = new GameWorld(strategy, 4, 1.0 / 60.0);

	std::thread audioThread = std::thread([](BOEngine* engine, AudioEngine* audio, Camera* camera)
		{
			//background music
	
			//engine sound
			GameObject* engine_sound = new  GameObject("EngineSound");
					engine_sound->transform.position = { camera->Position.x, camera->Position.y, camera->Position.z };
			engine->gameWorld->addGameObject(engine_sound);
					engine_sound->addComponent(new AudioPlayerComponent(*audio, "game/assets/sounds/startup.wav", 1, true, false, false));
			engine_sound->getComponent<AudioPlayerComponent>()->onAddToGameWorld();
			engine_sound->getComponent<AudioPlayerComponent>()->volume(1);
			engine_sound->getComponent<AudioPlayerComponent>()->play();
					engine_sound->addComponent(new AudioPlayerComponent(*audio, "game/assets/sounds/idle.wav", 1, true, true, false));
			engine_sound->getComponent<AudioPlayerComponent>()->onAddToGameWorld();
			engine_sound->getComponent<AudioPlayerComponent>()->volume(0.3);
			engine_sound->getComponent<AudioPlayerComponent>()->setSpeed(0);
			engine_sound->getComponent<AudioPlayerComponent>()->play();

			//skid sound
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/sound/skidsound.yaml", engine, NULL, camera, audio));

			//impact sounds
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/sound/smallimpact.yaml", engine, NULL, camera, audio));
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/sound/bigimpact.yaml", engine, NULL, camera, audio));

			//background music
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/music/background.yaml", engine, NULL, camera, audio));

			//win lose music
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/music/winmusic1.yaml", engine, NULL, camera, audio));
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/music/winmusic2.yaml", engine, NULL, camera, audio));
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/music/losemusic1.yaml", engine, NULL, camera, audio));
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/music/losemusic2.yaml", engine, NULL, camera, audio));
		}, engine, &audio, &camera);

	Vector3f carPos = Vector3f(0.0f, -3.0f, -43.0f);
	GameObject* player_car = new  GameObject("PlayerCar");
	player_car->transform.position = carPos;
	player_car->transform.rotation = Vector3f(0.0f, 0.0f, 0.0f);
	player_car->transform.scale = 1;
	player_car->addComponent(new RenderComponent(engine, "game/assets/avent/Avent_red_notires.obj", shader)); // no tires
	// player_car->addComponent(new RenderComponent(engine, "game/assets/avent/Avent_red.obj", shader));
	player_car->addComponent(RigidBodyComponent::createWithCube(1.0, 0.4, 1.0, 1.0, 1.0, 0)); //note id is set to 0, DO NOT CHANGE unless you change the id in the isHit()
	CarControlComponent* carControl = new CarControlComponent(10, 15, 2.5);
	player_car->addComponent(carControl);

	RaceGameComponent* race = new RaceGameComponent();
	race->checkpoints.push_back(glm::vec3(-45.0f, -3.0f, 0.0f));
	race->checkpoints.push_back(glm::vec3(0.0f, -3.0f, 45.0f));
	race->checkpoints.push_back(glm::vec3(45.0f, -3.0f, 0.0f));
	race->checkpoints.push_back(glm::vec3(0.0f, -3.0f, -45.0f));
	player_car->addComponent(race);
	engine->gameWorld->addGameObject(player_car);


	std::vector<GameObject*> tires(4);
	for (int i = 0; i < 4; i++) {
		tires[i] = new GameObject("PlayerCarTire" + i);
		tires[i]->parent = player_car;
		tires[i]->transform.position = Vector3f(1.55 * (i > 1 ? 1 : -1), 0.43, 1.02 * (i % 2 ? 1 : -1)); // relative local position, (back, up, left)
		if (!i % 2) { // right tires
			tires[i]->transform.rotation = Vector3f(0.0f, 3.1416, 0.0f);
		}
		tires[i]->transform.scale = 1;
		tires[i]->addComponent(new RenderComponent(engine, "game/assets/avent/Tires.obj", shader));
		((RenderComponent*)tires[i]->getComponent<RenderComponent>())->isSelfRotation = true;
		engine->gameWorld->addGameObject(tires[i]);
	}
	carControl->tires = tires;

	// create race track walls
	engine->gameWorld->addGameObject(Object_Load("game/assets/objects/track_walls1.yaml", engine, shader));
	//engine->gameWorld->addGameObject(Object_Load("game/assets/objects/track_walls2.yaml", engine, shader));

	// create race track
	engine->gameWorld->addGameObject(Object_Load("game/assets/objects/track1.yaml", engine, shader));
	//engine->gameWorld->addGameObject(Object_Load("game/assets/objects/track2.yaml", engine, shader));

	// Light
	GameObject* light = new  GameObject("Light");
	light->transform.position = Vector3f(pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
	light->transform.scale = 1.0; // has to be double because dimensions of 1.0 entered above refer to distance from origin to edge
	light->addComponent(new RenderComponent(engine, "game/assets/box/cube.obj", lightshader));
	// box->addComponent(RigidBodyComponent::createWithCube(1.0, 1.0, 1.0, 1.0));
	engine->gameWorld->addGameObject(light);

	// Light
	GameObject* light2 = new  GameObject("Light1");
	light2->transform.position = Vector3f(pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
	light2->transform.scale = 1.0; // has to be double because dimensions of 1.0 entered above refer to distance from origin to edge
	light2->addComponent(new RenderComponent(engine, "game/assets/box/cube.obj", lightshader));
	// box->addComponent(RigidBodyComponent::createWithCube(1.0, 1.0, 1.0, 1.0));
	engine->gameWorld->addGameObject(light2);

	// create ground
	engine->gameWorld->addGameObject(Object_Load("game/assets/objects/ground.yaml", engine, shader));


	engine->gameWorld->addGameObject(Object_Load("game/assets/objects/onion.yaml", engine, shader));
	engine->gameWorld->addGameObject(Object_Load("game/assets/objects/box.yaml", engine, shader));
	engine->gameWorld->addGameObject(Object_Load("game/assets/objects/box2.yaml", engine, shader));

	audioThread.join();

	// create box
	/*GameObject* box4 = new  GameObject("Box4");
	box4->transform.position = Vector3f(5.0, 25.0, 0);
	box4->transform.scale = 2.0; // has to be double because dimensions of 1.0 entered above refer to distance from origin to edge
	box4->addComponent(new RenderComponent(engine, "game/assets/box/cube.obj", shader));
	box4->addComponent(RigidBodyComponent::createWithCube(1.0, 1.0, 1.0, 1.0));
	box4->addComponent(new TypeTestComponent("This is a test message!"));
	engine->gameWorld->addGameObject(box4);*/

	//box4->getComponent<TypeTestComponent>()->talk();

	auto end = std::chrono::steady_clock::now();

	std::chrono::duration<float> duration = end - start;

	std::cout << "Loading complete! Took " << duration.count() << " seconds." << std::endl;
}

void GameWorldHelper::initTestScene2(BOEngine* engine)
{
	delete engine->gameWorld;
	engine->gameWorld = nullptr;

	// std::cout << "Game init" << std::endl;
	AudioEngine audio;
	Camera camera;

	Shader* shader;
	if (shaders.count("model") > 0)
	{
		shader = shaders["model"];
	}
	else
	{
		shader = shaders["model"] = new Shader("engine/graphic/shader/model_loading.vs", "engine/graphic/shader/model_loading.fs");
	}

	Shader* lightshader;
	if (shaders.count("light") > 0)
	{
		lightshader = shaders["light"];
	}
	else
	{
		lightshader = shaders["light"] = new Shader("engine/graphic/shader/model_loading.vs", "engine/graphic/shader/light.fs.glsl");
	}

	audio.Shutdown();
	audio.Init();
	engine->skybox.load(Skybox_Load("game/assets/objects/sky_water.yaml"));

	shader->use();
	shader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f); //obj to light
	shader->setVec3("dirLight.ambient", 0.45f, 0.45f, 0.35f);
	shader->setVec3("dirLight.diffuse", 1.0f, 1.0f, 1.0f);
	shader->setVec3("dirLight.specular", 0.8f, 0.8f, 0.8f);

	glm::vec3 pointLightPositions[] = {
		glm::vec3(16.0f,  5.0f,  0.0f),
		glm::vec3(12.3f, 5.3f, 5.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	// point light 1
	shader->setVec3("pointLights[0].position", pointLightPositions[0]);
	shader->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	shader->setVec3("pointLights[0].diffuse", 1.0f, 1.0f, 1.0f);
	shader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
	shader->setFloat("pointLights[0].constant", 1.0f);
	shader->setFloat("pointLights[0].linear", 0.09);
	shader->setFloat("pointLights[0].quadratic", 0.032);

	// point light 2
	shader->setVec3("pointLights[1].position", pointLightPositions[1]);
	shader->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	shader->setVec3("pointLights[1].diffuse", 1.0f, 1.0f, 1.0f);
	shader->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
	shader->setFloat("pointLights[1].constant", 1.0f);
	shader->setFloat("pointLights[1].linear", 0.09);
	shader->setFloat("pointLights[1].quadratic", 0.032);

	std::string strategy[] = { CarControlComponent::typeID, RigidBodyComponent::typeID, RenderComponent::typeID, RaceGameComponent::typeID };
	engine->gameWorld = new GameWorld(strategy, 4, 1.0 / 60.0);

	std::thread audioThread = std::thread([](BOEngine* engine, AudioEngine* audio, Camera* camera)
		{
			//background music

			//engine sound
			GameObject* engine_sound = new  GameObject("EngineSound");
					engine_sound->transform.position = { camera->Position.x, camera->Position.y, camera->Position.z };
			engine->gameWorld->addGameObject(engine_sound);
					engine_sound->addComponent(new AudioPlayerComponent(*audio, "game/assets/sounds/startup.wav", 1, true, false, false));
			engine_sound->getComponent<AudioPlayerComponent>()->onAddToGameWorld();
			engine_sound->getComponent<AudioPlayerComponent>()->volume(1);
			engine_sound->getComponent<AudioPlayerComponent>()->play();
					engine_sound->addComponent(new AudioPlayerComponent(*audio, "game/assets/sounds/idle.wav", 1, true, true, false));
			engine_sound->getComponent<AudioPlayerComponent>()->onAddToGameWorld();
			engine_sound->getComponent<AudioPlayerComponent>()->volume(0.3);
			engine_sound->getComponent<AudioPlayerComponent>()->setSpeed(0);
			engine_sound->getComponent<AudioPlayerComponent>()->play();

			//skid sound
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/sound/skidsound.yaml", engine, NULL, camera, audio));

			//impact sounds
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/sound/smallimpact.yaml", engine, NULL, camera, audio));
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/sound/bigimpact.yaml", engine, NULL, camera, audio));

			//background music
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/music/background.yaml", engine, NULL, camera, audio));

			//win lose music
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/music/winmusic1.yaml", engine, NULL, camera, audio));
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/music/winmusic2.yaml", engine, NULL, camera, audio));
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/music/losemusic1.yaml", engine, NULL, camera, audio));
			engine->gameWorld->addGameObject(Audio_Load("game/assets/objects/music/losemusic2.yaml", engine, NULL, camera, audio));
		}, engine, &audio, &camera);

	Vector3f carPos = Vector3f(0.0f, -3.0f, -43.0f);
	GameObject* player_car = new  GameObject("PlayerCar");
	player_car->transform.position = carPos;
	player_car->transform.rotation = Vector3f(0.0f, 0.0f, 0.0f);
	player_car->transform.scale = 1;
	player_car->addComponent(new RenderComponent(engine, "game/assets/avent/Avent_red_notires.obj", shader)); // no tires
	// player_car->addComponent(new RenderComponent(engine, "game/assets/avent/Avent_red.obj", shader));
	player_car->addComponent(RigidBodyComponent::createWithCube(1.0, 0.4, 1.0, 1.0, 1.0, 0)); //note id is set to 0, DO NOT CHANGE unless you change the id in the isHit()
	CarControlComponent* carControl = new CarControlComponent(10, 15, 2.5);
	player_car->addComponent(carControl);

	RaceGameComponent* race = new RaceGameComponent();
	race->checkpoints.push_back(glm::vec3(-145.0f, -3.0f, 0.0f));
	race->checkpoints.push_back(glm::vec3(0.0f, -3.0f, 45.0f));
	race->checkpoints.push_back(glm::vec3(45.0f, -3.0f, 0.0f));
	race->checkpoints.push_back(glm::vec3(0.0f, -3.0f, -45.0f));
	player_car->addComponent(race);
	engine->gameWorld->addGameObject(player_car);


	std::vector<GameObject*> tires(4);
	for (int i = 0; i < 4; i++) {
		tires[i] = new GameObject("PlayerCarTire" + i);
		tires[i]->parent = player_car;
		tires[i]->transform.position = Vector3f(1.55 * (i > 1 ? 1 : -1), 0.43, 1.02 * (i % 2 ? 1 : -1)); // relative local position, (back, up, left)
		if (!i % 2) { // right tires
			tires[i]->transform.rotation = Vector3f(0.0f, 3.1416, 0.0f);
		}
		tires[i]->transform.scale = 1;
		tires[i]->addComponent(new RenderComponent(engine, "game/assets/avent/Tires.obj", shader));
		((RenderComponent*)tires[i]->getComponent<RenderComponent>())->isSelfRotation = true;
		engine->gameWorld->addGameObject(tires[i]);
	}
	carControl->tires = tires;

	// create race track walls
	engine->gameWorld->addGameObject(Object_Load("game/assets/objects/track_walls3.yaml", engine, shader));

	// create race track
	engine->gameWorld->addGameObject(Object_Load("game/assets/objects/track3.yaml", engine, shader));

	// Light
	GameObject* light = new  GameObject("Light");
	light->transform.position = Vector3f(pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
	light->transform.scale = 1.0; // has to be double because dimensions of 1.0 entered above refer to distance from origin to edge
	light->addComponent(new RenderComponent(engine, "game/assets/box/cube.obj", lightshader));
	// box->addComponent(RigidBodyComponent::createWithCube(1.0, 1.0, 1.0, 1.0));
	engine->gameWorld->addGameObject(light);

	// Light
	GameObject* light2 = new  GameObject("Light1");
	light2->transform.position = Vector3f(pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
	light2->transform.scale = 1.0; // has to be double because dimensions of 1.0 entered above refer to distance from origin to edge
	light2->addComponent(new RenderComponent(engine, "game/assets/box/cube.obj", lightshader));
	// box->addComponent(RigidBodyComponent::createWithCube(1.0, 1.0, 1.0, 1.0));
	engine->gameWorld->addGameObject(light2);

	// create ground
	engine->gameWorld->addGameObject(Object_Load("game/assets/objects/ground_water.yaml", engine, shader));

	engine->gameWorld->addGameObject(Object_Load("game/assets/objects/onion.yaml", engine, shader));

	audioThread.join();

}
