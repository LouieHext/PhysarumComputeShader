#version 440


//loading in textures and buffers
layout(binding = 1) buffer pheremonesBuffer { 
	float pheremones [  ];
};
layout(binding = 2) buffer pheremonesBackBuffer { 
	float pheremonesBack [  ];
};
layout(rgba8,binding=3)  uniform writeonly image2D pheremoneDisplay; //output to texture

layout(local_size_x= 20, local_size_y= 20, local_size_z = 1) in;    //reciving segment for parallelisation

//diffusion uniforms
uniform float decayWeight;
uniform float diffusionWeight;
uniform int colouring;

//width and height uniforms
uniform int W;
uniform int H;



//diffusion kernel
float weights[9] = float[](
   1/16.0, 1/8.0, 1/16.0, 
    1/8.0, 1/4.0, 1/8.0, 
    1/16.0, 1/8.0, 1/16.0);

void main(){
    //loading pheremone data
	int i = int(gl_GlobalInvocationID.x); 
    int j = int(gl_GlobalInvocationID.y);
	int idx = i+j*W;

	//applying diffusion kernel (3x3)
	//diffusing  surroundings to one square has the same impact as diffusing one square to surroundings 
	//but this allows for it to be independent of the order in which it is done

	float value=0;
	int c=0;
	for(int ii=-1;ii<=1;ii++){
		for(int jj=-1;jj<=1;jj++){
			value+=pheremones[i + ii + ( j + jj ) * W]*weights[c];
			c++;
		}
	}

	//mixing in diffused and decayed values into back buffer
	float origValue=pheremones[idx]; //current value in primary 
	pheremonesBack[idx]=origValue * (1.0-diffusionWeight) + value * (diffusionWeight); //diffusion
	pheremonesBack[idx]*=(1.0-decayWeight); //decay

	//setting current colour based on pheremone intensity
	vec4 col;

	if (colouring>0){
		col = vec4(0.8*vec3(pheremonesBack[idx],pheremonesBack[idx]*0.5,pheremonesBack[idx]*0.1),1.0);

		if (pheremonesBack[idx]<0.5){
			col = vec4(1.5*vec3(pheremonesBack[idx]*0.1,pheremonesBack[idx]*0.5,pheremonesBack[idx]),1.0);
		}
	}
	else {
		col = vec4(vec3(pheremonesBack[idx]),1.0);
	}

    imageStore(pheremoneDisplay,ivec2(gl_GlobalInvocationID.xy),col); //storing in texture

}