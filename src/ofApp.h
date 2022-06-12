#pragma once

#include "ofMain.h"
#include "ofxGui.h"

//globals for simulation and image size
#define W 1080 
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
		vector<Particle> particles;

		//buffer objects and textures
		ofBufferObject particlesBuffer, particlesBufferClear, pheremones, pheremonesBack,pheremonesClear;
		ofTexture pheromoneIntensityTexture;

		//CPU side array for buffer decloration
		float pheremonesCPU[W*H];

		//GUI and params
		ofxPanel gui;
		ofParameter<float> maxSpeed,turningSpeed;
		ofParameter<float> sensorAngle;
		ofParameter<int> sensorDistance, sensorSize;
		ofParameter<int> numAgents;
		ofParameter<float> decayWeight, diffusionWeight;
		ofParameter<int> densitySpeed;
		ofParameter<float> baseMulti, densityMulti;

		ofParameterGroup agentSettings, pheromoneSettings;

		//saving
		bool saving;
		ofPixels pixels;
		ofImage image;
		int c = 1;

};
