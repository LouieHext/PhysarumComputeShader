#version 440


//loading in buffers and defining parallelisation method
//--------------------------------------------------------------

//declaring agent Struct to read in agent buffer
struct Agent{
	vec3 pos;
	float heading;
};
//agent buffer
layout(std140, binding=0) buffer agentBuffer{
    Agent p[];
};
layout(std140, binding=4) buffer agentBuffer2{
    Agent p2[];
};
//main pheremone buffer
layout(binding = 1) buffer pheremonesBuffer { 
	float pheremones [  ];
};
layout(binding = 5) buffer pheremonesBuffer2 { 
	float pheremones2 [  ];
};
//local work group size of 1024
layout(local_size_x = 512, local_size_y = 1, local_size_z = 1) in;

//loading in simulation param uniforms
//--------------------------------------------------------------

//agent uniforms
uniform float maxSpeed;
uniform float turningSpeed;
uniform float sensorAngle;
uniform int sensorDistance;
uniform int sensorSize;
uniform int densitySpeed;
uniform int multiSpecies;
uniform float baseMulti;
uniform float densityMulti;
//width and height uniforms
uniform int W;
uniform int H;
uniform int numParticles;

;

//Simulation Functions
//--------------------------------------------------------------

//calulates the index of the sensed region of a Agent
//depends on sensor angle and distance
//returns a vec2 of the sensed position
vec2 getSensedRegion(Agent agent,float theta) {
    return floor(vec2(agent.pos.x,agent.pos.y) + sensorDistance * vec2(cos(agent.heading + theta),
									                  sin(agent.heading + theta )));
 }
  
//returns the value of phermones at the given coords.
//if a kernel size>0 is used the values in the kernel are summed
//larger values are "attractive" to agents
 float getSensedValue(vec2 pos, bool secondSpecies) {
	
    float value = 0; //init
    for (int i = -sensorSize; i <= sensorSize; i++) {				 //kernel loop
      for (int j = -sensorSize; j <= sensorSize; j++) {				 //to sum values
        if (!secondSpecies){
			value += pheremones[int(pos.x) + i + (int(pos.y) + j) * W]-pheremones2[int(pos.x) + i + (int(pos.y) + j) * W]*0.8;  //2D -> 1D
		}
		else if (secondSpecies){
			value += pheremones2[int(pos.x) + i + (int(pos.y) + j) * W]-pheremones[int(pos.x) + i + (int(pos.y) + j) * W]*0.8; 
		}
      }
    }
    return value;
	
  }


//updates the agents heading based on the infomation from the phermones
//the agents turn in the direction of higher pheremone intensity
float updateHeading(Agent agent,bool secondSpecies) {

    //checking left, right and forward regions.
	//gets the sensed region, based on sensor Angle and distance
	//which is then fed into the getSensedValue function to 
	//return the pheremone value
    float senseLeft = getSensedValue(getSensedRegion(agent,sensorAngle),secondSpecies);
    float senseForward = getSensedValue(getSensedRegion(agent,0.0),secondSpecies);
    float senseRight = getSensedValue(getSensedRegion(agent,-sensorAngle),secondSpecies);
    
    //moving right
    if (senseRight > senseLeft && senseRight > senseForward) {
      return agent.heading-turningSpeed;
    }
    //moving left
    else if (senseLeft > senseRight && senseLeft > senseForward) {
      return agent.heading + turningSpeed;
    }
	else {
	  return agent.heading;
	}
  }

  //standard position update function
  //moves at a step size of speed in direction of agent heading
vec2 updatePos(Agent agent,bool secondSpecies) {
	
	
	float speed = maxSpeed;

	if (densitySpeed>0){
		float value = 0;
		int i = int(agent.pos.x);
		int j = int(agent.pos.y);
		for(int ii=-6;ii<=6;ii++){
			for(int jj=-6;jj<=6;jj++){
				if (!secondSpecies){
					value+=pheremones[i + ii + ( j + jj ) * W];
				}
				 else if (secondSpecies){
					value+=pheremones2[i + ii + ( j + jj ) * W];
				}
			}
		}
		speed = maxSpeed*baseMulti + min(value*value*densityMulti*0.01,maxSpeed);
	}
	
	return vec2(agent.pos.x,agent.pos.y) + speed*vec2(cos(agent.heading),sin(agent.heading));

  }

  //applies periodic boundary conditions
  //this captures all possiblities without the need for branches
vec2 boundaryCheck(vec2 pos) {
    return vec2(mod(pos.x + W,W),mod(pos.y + H,  H));
  }


void main(){
	//loading agent
	

	
	Agent agent = p[gl_GlobalInvocationID.x];
	agent.heading=updateHeading(agent,false);						//updating heading
    vec2 tempPos=updatePos(agent,false);							//getting new position
    agent.pos=vec3(boundaryCheck(tempPos),0.0);				//applying BC on new position
	p[gl_GlobalInvocationID.x] =  agent;					//updating agent buffer
	pheremones[int(agent.pos.x) + int(agent.pos.y) * W]+=1; //dropping phermones
	
	if (multiSpecies>0){
		Agent agent2 = p2[gl_GlobalInvocationID.x];
		agent2.heading=updateHeading(agent2,true);						//updating heading
		vec2 tempPos=updatePos(agent2,true);							//getting new position
		agent2.pos=vec3(boundaryCheck(tempPos),0.0);				//applying BC on new position
		p2[gl_GlobalInvocationID.x] =  agent2;					//updating agent buffer
		pheremones2[int(agent2.pos.x) + int(agent2.pos.y) * W]+=1; //dropping phermones


	}

}
