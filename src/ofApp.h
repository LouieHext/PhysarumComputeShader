#pragma once

#include "ofMain.h"
#include "ofxGui.h"

//globals for simulation and image size
#define W 1920
#define H 1080

class ofApp : public ofBaseApp{

	public:

		//main functions
		void setup();
		void update();
		void draw();

		void setupShaders();
		void setupParams();

		void updateAgents();
		void updatePheromones();

		void showFPS();
		void saveFrame();
		//interations
		void keyPressed(int key);
		
		
		//shader objects
		ofShader simulation,diffusion;
		
		//particle struct
		struct Particle {
			glm::vec3 pos;
			float heading;
		};

		//container for particles
		vector<Particle> particles, particles2;

		//buffer objects and textures
		ofBufferObject particlesBuffer, particlesBuffer2, particlesBufferClear, particlesBufferClear2;
		ofBufferObject	pheremones, pheremones2, pheremonesBack,pheremonesBack2,pheremonesClear, pheremonesClear2;
		ofTexture pheromoneIntensityTexture;

		//CPU side array for buffer decloration
		float pheremonesCPU[W*H], pheremonesCPU2[W*H];

		//GUI and params
		ofxPanel gui;
		ofParameter<float> maxSpeed,turningSpeed;
		ofParameter<float> sensorAngle;
		ofParameter<int> sensorDistance, sensorSize;
		ofParameter<int> numAgents;
		ofParameter<float> decayWeight, diffusionWeight;
		ofParameter<int> densitySpeed,colouring,multiSpecies;
		ofParameter<float> baseMulti, densityMulti;

		ofParameterGroup agentSettings, pheromoneSettings;

		//saving
		bool saving;
		ofPixels pixels;
		ofImage image;
		int c = 1;

};
