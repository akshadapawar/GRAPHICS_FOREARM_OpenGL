#include <GL/glut.h>
#include <stdio.h>


#define Rot_Ang_Inc		0.1

static GLfloat g_rotate = 0;
static GLfloat g_rotInc = Rot_Ang_Inc; 
double rotate_X_axis = 0.5;



GLfloat ctrlpoints[4][4][3] =
{


	{
		{-0.5f,-1.5f,0.0f},
		{-2.0f,-1.5f,1.0f},
		{2.5f,-1.5f,1.0f},
		{-0.5f,-1.5f,0.0f}
	},
	{
		{-0.5f,-0.5f,0.0f},
		{-2.0f,-0.5f,1.0f},
		{2.5f, -0.5f, 1.0f},
		{-0.5f, -0.5,0.0f}
	},
	{
		{-0.1f, 0.5f, 1.0f},
		{-2.5f, 0.5f, 1.0f},
		{0.0f, 0.5f, 1.0f},
		{-0.1f, 0.5f, 1.0f}
	},
	{
		{-1.0f, 1.5f, 1.0f},
		{-2.0f, 1.5f, 1.0f},
		{1.0f, 1.5f, 1.0f},
		{-1.0f, 1.5f, 1.0f}}

};

#define image_Width 1541
#define image_Height 4169
GLubyte checkImage[image_Width][image_Height][3];
GLuint texture[2];
GLint slices=16;
GLint stacks=16;


struct image_Struct {
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
};
typedef struct image_Struct Image;


void makeImage(void)
{
	int width,height, c;
	for (width = 0; width < image_Width; width++) {
		for (height = 0; height < image_Height; height++) {
			c = ((((width&0x8)==0)^((height&0x8)==0)))*255;
			checkImage[width][height][0] = (GLubyte) c;
			checkImage[width][height][1] = (GLubyte) c;
			checkImage[width][height][2] = (GLubyte) c;
		}
	}
}

int ImageLoad(char *filename, Image *image) {
	FILE *file;
	unsigned long size; 						// size of the image in bytes.
	unsigned long i; 						// standard counter.
	unsigned short int plane; 					// number of planes in image

	unsigned short int bpp; 					// number of bits per pixel
	char temp; 							// temporary color storage for
	
	// make sure the file is there.
	if ((file = fopen(filename,"rb"))==NULL)
	{
		printf("File Not Found : %s\n",filename);
		return 0;
	}
	
	// seek through the bmp header, up to the width/height:
	fseek(file, 18, SEEK_CUR);
	// read the width
	if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
		printf("Error reading width from %s.\n", filename);
		return 0;
	}

	// read the height
	if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
		printf("Error reading height from %s.\n", filename);
		return 0;
	}

	//printf("Height of %s: %lu\n", filename, image->sizeY);
	// calculate the size (assuming 24 bits or 3 bytes per pixel).

	size = image->sizeX * image->sizeY * 3;
	
	// read the planes
	if ((fread(&plane, 2, 1, file)) != 1) {
		printf("Error reading planes from %s.\n", filename);
		return 0;
	}
	
	// read the bitsperpixel
	if ((i = fread(&bpp, 2, 1, file)) != 1) {
		printf("Error reading bpp from %s.\n", filename);
		return 0; 
	}
	
	// seek past the rest of the bitmap header.
	fseek(file, 24, SEEK_CUR);
	// read the data.
	image->data = (char *) malloc(size);
	
	if ((i = fread(image->data, size, 1, file)) != 1) {
		printf("Error reading image data from %s.\n", filename);
		return 0;
	}
	for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
		temp = image->data[i];
		image->data[i] = image->data[i+2];
		image->data[i+2] = temp;
	}
	// we're done.
	return 1;
}

Image * loadTexture()
{
	Image *image1;
	// allocate space for texture
	image1 = (Image *) malloc(sizeof(Image));
	if (image1 == NULL) {
		printf("Error allocating space for image");
		exit(0);
	}

	if (!ImageLoad("akshu.bmp", image1)) {
		exit(1);
	}
	return image1;
}
void myinit(void)
{
	glClearColor (0.0, 0.0, 0.0, 1.0);
	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4,0, 1, 12, 4, &ctrlpoints[0][0][0]);
	glEnable(GL_MAP2_VERTEX_3);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glMapGrid2f(20, 0.0, 1.0, 20, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	Image *image1 = loadTexture();
	if(image1 == NULL)
	{
		printf("Image was not returned from loadTexture\n");
		exit(0);
	}
	makeImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);      //To store the pixel That we have read which is not affect to each other.

	// Create Texture
	glGenTextures(2, texture);                  //Return N texture name .That no guarentee to contineous set of integer.

	glBindTexture(GL_TEXTURE_2D, texture[0]);   // bind a named texture to a texturing target

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); //set texture parameters i.e target ,name,set value param
	//scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); //scale linearly when image smalled than texture
	
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0,GL_RGB, GL_UNSIGNED_BYTE, image1->data);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);  //gl_combine_alpha,rgb...
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image_Width,image_Height, 0, GL_RGB, GL_UNSIGNED_BYTE,&checkImage[0][0][0]);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glScalef(rotate_X_axis,rotate_X_axis,1.0); 
	glRotatef(180.0, 1.0, 1.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glEvalMesh2(GL_FILL, 0, 20, 0, 20);
	glPopMatrix();
	glFlush();
	glutSwapBuffers();
}



void myIdleFunc(void) {
	g_rotate += g_rotInc;
	glutPostRedisplay();
}
void Key(unsigned char k, int x, int y)
{
	switch (k) {
		case 'e':
		case 'E':	exit(0);
				break;

		case 'x':
		case 'X':
				glRotatef(10.0,1.0f,0.0f,0.0f);           //Rotation about X-axis
				break;
		case 'y':
		case 'Y':
				glRotatef(10.0,0.0f,1.0f,0.0f);		 //Rotation about Y-axis
				break;

		case 'z':
		case 'Z':
				glRotatef(10.0,0.0f,0.0f,1.0f);		//Rotation about Z-axis
				break;


		default:
				break;
	}
}

void myReshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-4.0, 4.0, -4.0 * (GLfloat) h / (GLfloat) w,4.0 * (GLfloat) h / (GLfloat) w, -4.0, 4.0);
//	glFrustum(-4.0, 4.0, -4.0 * (GLfloat) h / (GLfloat) w,-40.0 * (GLfloat) h / (GLfloat) w, -4.0, 14.0);
	glMatrixMode(GL_MODELVIEW);
	//gluLookAt(0.0,0.0,1.0,0.0,0.0,0.0,0.0,0.1,0.0);

}

void keyBoard(int key,int x,int y)				//Scalling the object .
{
	if(key == GLUT_KEY_UP)
		rotate_X_axis +=0.5;
	if(key == GLUT_KEY_DOWN)
		rotate_X_axis -=0.5;     
}



int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow(argv[0]);
	myinit();
	glutReshapeFunc (myReshape);
	glutIdleFunc(myIdleFunc);
	glutDisplayFunc(display);
	glutKeyboardFunc(Key);
	glutSpecialFunc(keyBoard);
	glutMainLoop();
	return 0;
}
