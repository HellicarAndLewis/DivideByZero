#include "ofTextureAdv.h"

//----------------------------------------------------------
ofTextureAdv::ofTextureAdv(){
	texData.textureID = 0;
	grid = new ofPoint[GRID_X * GRID_Y];
	coor = new ofPoint[GRID_X * GRID_Y];
	compL = new float[GRID_Y];
	compR = new float[GRID_Y];

	memset(compL, 0, GRID_Y * sizeof(float));
	memset(compR, 0, GRID_Y * sizeof(float));
}


//----------------------------------------------------------
ofTextureAdv::~ofTextureAdv(){
	delete[] grid;
	delete[] coor;
	delete[] compL;
	delete[] compR;
}

//----------------------------------------------------------
void ofTextureAdv::clear(){
	width = 0;
	height = 0;
}

//----------------------------------------------------------
void ofTextureAdv::setPoint(int which, float x, float y){

	if( which>4 || which < 0){
		printf("ofTextureAdv:: please choose point 0-3\n");
		return;
	}

	quad[which].set(x, y, 0);

	updatePoints();
}

//----------------------------------------------------------
void ofTextureAdv::setPoints(ofPoint outputPts[4]){
	for (int i = 0; i < 4; i++){
	    quad[i].set(outputPts[i].x, outputPts[i].y, 0);
	}
	updatePoints();
}

//----------------------------------------------------------
void ofTextureAdv::setPoints(ofPoint inputPts[4], ofPoint outputPts[4]){
	for (int i = 0; i < 4; i++){
	    quad[i].set(outputPts[i].x, outputPts[i].y, 0);
	    utQuad[i].set(inputPts[i].x, inputPts[i].y, 0);
	}
	updatePoints();
}

//----------------------------------------------------------
void ofTextureAdv::updatePoints(){

	int gridSizeX = GRID_X;
	int gridSizeY = GRID_Y;

	float xRes = 1.0/(gridSizeX-1);
	float yRes = 1.0/(gridSizeY-1);

	//GLfloat offsetw = 1.0f/(tex_w);
	//GLfloat offseth = 1.0f/(tex_h);

	//this is because we want pct to go from offsetw to tex_t - offsetw
	GLfloat texTAdj = texData.tex_t;
	GLfloat texUAdj = texData.tex_u;
	//printf("%f %f \n", tex_t, tex_u);

	float xPct = 0.0;

	for(int y = 0; y < gridSizeY; y++){
		for(int x = 0; x < gridSizeX; x++){

            int index = y*gridSizeX + x;


			float pctx  = (float)x * xRes;
			float pcty  = (float)y * yRes;

			float pctyL = pcty + yRes*compL[y];
            float pctyR = pcty + yRes*compR[y];

			float linePt0x  = (1-pctyL)*quad[0].x + pctyL * quad[3].x;
			float linePt0y  = (1-pctyL)*quad[0].y + pctyL * quad[3].y;
			float linePt1x  = (1-pctyR)*quad[1].x + pctyR * quad[2].x;
			float linePt1y  = (1-pctyR)*quad[1].y + pctyR * quad[2].y;

            float ptx 	    = (1-pctx) * linePt0x + pctx * linePt1x;
			float pty 	    = (1-pctx) * linePt0y + pctx * linePt1y;

            float utPt0x    = (1-pcty)*utQuad[0].x + pcty * utQuad[3].x;
			float utPt0y    = (1-pcty)*utQuad[0].y + pcty * utQuad[3].y;
			float utPt1x    = (1-pcty)*utQuad[1].x + pcty * utQuad[2].x;
			float utPt1y    = (1-pcty)*utQuad[1].y + pcty * utQuad[2].y;
            float tt 	    = (1-pctx) * utPt0x + pctx * utPt1x;
			float uu 	    = (1-pctx) * utPt0y + pctx * utPt1y;



			grid[index].set(ptx, pty, 0);
			coor[index].set( (tt * texData.tex_t), texData.bFlipTexture ? texData.tex_u - (uu * texData.tex_u) : (uu * texData.tex_u), 0);
		}
	}

}

void ofTextureAdv::draw(bool showGrid){

	int gridSizeX = GRID_X;
	int gridSizeY = GRID_Y;

	ofSetColor(255, 255, 255);

	glEnable(texData.textureTarget);
		//glBindTexture( texData.textureTarget, (GLuint)texData.textureName[0] );
	glBindTexture( texData.textureTarget, texData.textureID );//JGL EDIT

	for(int y = 0; y < gridSizeY-1; y++){
		for(int x = 0; x < gridSizeX-1; x++){

			glBegin(GL_QUADS);

			int pt0 = x + y*gridSizeX;
			int pt1 = (x+1) + y*gridSizeX;
			int pt2 = (x+1) + (y+1)*gridSizeX;
			int pt3 = x + (y+1)*gridSizeX;

			glTexCoord2f(coor[pt0].x, coor[pt0].y);
			glVertex2f(  grid[pt0].x, grid[pt0].y);

			glTexCoord2f(coor[pt1].x, coor[pt1].y);
			glVertex2f(  grid[pt1].x, grid[pt1].y);

			glTexCoord2f(coor[pt2].x, coor[pt2].y);
			glVertex2f(  grid[pt2].x, grid[pt2].y);

			glTexCoord2f(coor[pt3].x, coor[pt3].y);
			glVertex2f(  grid[pt3].x, grid[pt3].y);

			glEnd();

		}
	}
	glDisable(texData.textureTarget);

    if(showGrid){
        for(int y = 0; y < gridSizeY-1; y++){
            for(int x = 0; x < gridSizeX-1; x++){

                glBegin(GL_LINE_LOOP);

                int pt0 = x + y*gridSizeX;
                int pt1 = (x+1) + y*gridSizeX;
                int pt2 = (x+1) + (y+1)*gridSizeX;
                int pt3 = x + (y+1)*gridSizeX;

                glVertex2f(  grid[pt0].x, grid[pt0].y);
                glVertex2f(  grid[pt1].x, grid[pt1].y);
                glVertex2f(  grid[pt2].x, grid[pt2].y);
                glVertex2f(  grid[pt3].x, grid[pt3].y);

                glEnd();

            }
        }
    }

}
