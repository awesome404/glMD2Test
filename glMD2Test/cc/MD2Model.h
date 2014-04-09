#ifndef __MD2MODEL_H__
#define __MD2MODEL_H__

#include "MD2Format.h"

typedef float vec3_t[3];

struct glCommand_t {
	int type;
	int count;
	glCommandVertex_t *vertices;
};

class MD2Model
{
	private:
		float _scale;

		int _skinWidth;
		int _skinHeight;
		int _frameSize;

		int _numVertices;
		int _numSkins;
		int _numTexCoords;
		int _numTriangles;
		int _numGLCommands;
		int _numFrames;

		char **_skins;
		textureCoordinate_t *_texCoords;
		triangle_t *_triangles;
		frame_t *_frames;
		glCommand_t *_glCommands;
		
		float *_vertexBuffer;
		float *_textureBuffer;

		unsigned int _textureID;

		/*byte *_textureBytes;
		unsigned int _textureHandle;*/

	protected:
		void init();
		void free();

	public:
		MD2Model() {init();};
		MD2Model(const char *fileName);
		~MD2Model() {free();};

		bool loadFromFile(const char *fileName);
		//bool loadTextureFromFile(char *fileName);
		//void setTextureFromFile(char *fileName);
		bool setTextureBytes(const char *targa);

		//void renderFrame(const int frame);
		//void iterpFrame(const int framea,const int frameb,const float interp);

		void glRenderFrame(const int frame);
		void glIterpFrame(const int framea,const int frameb,const float interp);

		float scale(const float &scale) {return _scale=scale;};
		float scale() {return _scale;};
};

#endif