#include "ofApp.h"

//Created 04/2022 - Louie Hext
//Explorations of stigmergy via a Physarum Polycephalum slime mould simulation
//this code  uses a compute shader to simulate the stigmergy properties of physarum slimes
//effectivly motion driven by pheremones

//this code simulates a large number of agents that move in a periodically bound system
//as an agent moves it leaves a pheromone trail behind it.
//imporantly an agents heading is dictated by pheomone intensities in its vicinity.
//this feedback loop causes the agents to follow each other in twisting arcing paths.

//the patterns that these paths make are influenced by the agents global settings.
//exploring these gives a wealth of different structures, some are "physical" in the sense that
//the paths are connected and naturally organic
//other parts of the paramter space act like rain drops, particles, worms, lattices

//for slime like settings try
//speed < distance , turning speed <1.5, angle <1.5

//if speed is > distance you will get streaking particles that wiggle around
//adjust other settings in this part of the param space to explore these

//if turning speed >2 and angle >1.5 (ish) you will get closed circular particles
//within this regime if speed > distance these aparticles move
//if these particles collide the more massive one will "absorb" the smaller

//CONTROLS
//pressing "s" toggles the image save function
//pressing "r" restarts the simulation

//enjoy!

//--------------------------------------------------------------
void ofApp::setup(){
	setupShaders(); //setting up shaders, 
	setupParams();  //setting up simulation params
	saving = false; //setting up saving conditional
}

//--------------------------------------------------------------
void ofApp::update() {
	updateAgents();		//dispatching simulation shader
	updatePheromones();	//dispatching diffusion shader
}

//--------------------------------------------------------------
void ofApp::draw(){
	showFPS();  //performance
	pheromoneIntensityTexture.draw(0, 0);  //drawing our texture buffer
	gui.draw(); //drawing GUI
	if (saving && ofGetFrameNum()%20==0) { 
		saveFrame(); //saving images
	}
}

//SETUP FUNCTIONS
//--------------------------------------------------------------
void ofApp::setupShaders() {
	//linking shaders
	simulation.setupShaderFromFile(GL_COMPUTE_SHADER, "simulation.glsl"); //simulations the stigmergy Agents
	simulation.linkProgram();
	diffusion.setupShaderFromFile(GL_COMPUTE_SHADER, "Diffusion.glsl");  //diffuses and decays pheremones
	diffusion.linkProgram();

	//initialising particle vector with particles
	particles.resize(1024*16*1024); //try and keep power of 2
	for (auto & particle : particles) {
		particle.pos = glm::vec3(ofGetWidth()*0.5, ofGetHeight()*0.5,0);
		particle.heading = ofRandom(0, 2 * PI);
	}
	//initialising pheremone array with zero values
	for (int x = 0; x < W; x++) {
		for (int y = 0; y < H; y++) {
			int idx = x + y * W; //2D->1D
			pheremonesCPU[idx] = 0.0;
		}
	}

	//allocating buffer objects
	auto arraySize = W * H * sizeof(float);
	pheremones.allocate(arraySize, pheremonesCPU, GL_STATIC_DRAW);		//storing pheremone intensities
	pheremonesBack.allocate(arraySize, pheremonesCPU, GL_STATIC_DRAW);  //to allow for diffusion
	pheremonesClear.allocate(arraySize, pheremonesCPU, GL_STATIC_DRAW); //for resetting 
	particlesBuffer.allocate(particles, GL_DYNAMIC_DRAW);				//storing particle info
	particlesBufferClear.allocate(particles, GL_STATIC_DRAW);			//for restarting

	//binding buffers so GPU knows whats what
	//note we dont bind the "Clear" buffers as they are not referenced in the shader code
	particlesBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	pheremones.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	pheremonesBack.bindBase(GL_SHADER_STORAGE_BUFFER, 2);

	//allocating and binding display texture which we use to visualise GPU results
	pheromoneIntensityTexture.allocate(W, H, GL_RGBA8);
	pheromoneIntensityTexture.bindAsImage(3, GL_WRITE_ONLY);

}

void ofApp::setupParams() {
	//setting up GUI and simulation params
	gui.setup();
	//setting up agent params
	agentSettings.setName("Agent params");
	agentSettings.add(maxSpeed.set("maxSpeed", 2.7, 0, 20)); 			//step size of particles
	agentSettings.add(turningSpeed.set("turningSpeed", 1.0, 0, 3.141)); //angular step size
	agentSettings.add(sensorAngle.set("sensorAngle", 0.3, 0, 3.141));   //angle at which particles checks phermones
	agentSettings.add(sensorDistance.set("sensorDistance", 10, 1, 25)); //distance at when particle checks phermones
	agentSettings.add(sensorSize.set("sensorSize", 1, 0, 5));			//kernel size of pheremone check
	//setting up pheromone params
	pheromoneSettings.setName("Pheromone params");
	pheromoneSettings.add(decayWeight.set("decayWeight", 0.5, 0, 1));		  //value at which all pheromones decay
	pheromoneSettings.add(diffusionWeight.set("diffusionWeight", 0.1, 0, 1)); //value at which all pheromones diffuse
	//adding to GUI
	gui.add(agentSettings);
	gui.add(pheromoneSettings);

}

//UPDATE FUNCTIONS
//-------------------------------------------------------------
void ofApp::updateAgents() {
	//starting shader
	simulation.begin();
	//passing uniforms
	simulation.setUniforms(agentSettings);
	simulation.setUniform1i("W", W);
	simulation.setUniform1i("H", H);
	simulation.setUniform1i("numParticles", particles.size());
	//dispatching
	simulation.dispatchCompute(512, 1, 1); //splitting particles into 512 work groups (much faser than 1024 for some reason)
	//ending shader
	simulation.end();
}

void ofApp::updatePheromones() {
	//starting shader
	diffusion.begin();
	//passing uniforms
	diffusion.setUniforms(pheromoneSettings);
	diffusion.setUniform1i("W", W);
	diffusion.setUniform1i("H", H);
	//dispatching
	diffusion.dispatchCompute(W / 20, H / 20, 1); //splitting diffusion into work groups of size 20*20
	//ending shader
	diffusion.end();
	//diffusion shader sets diffused values to pheremoneBack buffer
	//copying those to main pheremone buffer for next frame
	pheremonesBack.copyTo(pheremones);
}

//Helper Functions
//--------------------------------------------------------------
void ofApp::showFPS() {
	ofSetColor(255);					 //setting white
	std::stringstream strm;
	strm << "fps: " << ofGetFrameRate(); //frame rate to sting stream
	ofSetWindowTitle(strm.str());
}

void ofApp::saveFrame() {
	//reading texture data into an oF image for saving
	pheromoneIntensityTexture.readToPixels(pixels);
	image.setFromPixels(pixels);
	image.save(ofToString(c) + ".png"); //saving with sequential names
	c++;
}

void ofApp::keyPressed(int key){
	//reseting the sim by copying the stored default values to the main buffers
	if (key == 'r') {
		particlesBufferClear.copyTo(particlesBuffer);
		pheremonesClear.copyTo(pheremones);
		pheremonesClear.copyTo(pheremonesBack);
	}
	//changing saving conditional
	if (key == 's') {
		saving = !saving;
	}
}


