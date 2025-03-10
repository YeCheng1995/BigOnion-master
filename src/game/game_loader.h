#pragma once

#include "../engine/BOEngine.h"
#include "../FileSystem.h"
#include "../engine/input/InputHandler.h"

class GameLoader
{
	private:
		BOEngine* engine = nullptr;

		void processInput(GLFWwindow* window);
		

	public:
		GameLoader();
		//GameWorld* gworld = nullptr;
		InputHandler input;

		void createGame();
		void setEngine(BOEngine&);
		void reload();
		void loadGameScene();
		void startGame();
		void exitGame();
};
