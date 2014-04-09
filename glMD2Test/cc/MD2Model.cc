#include <stdio.h>
#include <string.h>
#import <OpenGLES/ES1/gl.h>

#include <stdlib.h>

#include "MD2Model.h"
#include "MD2Format.h"

//#define GL_TRIANGLE_STRIP 0
//#define GL_TRIANGLE_FAN 1

/*static vec3_t _kAnorms[162]= {
#include "anorms.h"
};*/

MD2Model::MD2Model(const char *fileName) {
	init();
	loadFromFile(fileName);
}

void MD2Model::init() {
	_scale = 1.0;

	_skinWidth = 0;
	_skinHeight = 0;
	_frameSize = 0;

	_numSkins = 0;
	_numTexCoords = 0;
	_numTriangles = 0;
	_numFrames = 0;
	_numGLCommands = 0;

	_skins = 0;
	_texCoords = 0;
	_triangles = 0;
	_frames = 0;
	_glCommands = 0;
	
	_vertexBuffer = 0;
	_textureBuffer = 0;
	
	//_textureBytes = 0;
}

void MD2Model::free() {
	size_t i;

	if(_skins) {
		for(i=0;i<_numSkins;i++) if(_skins[i]) delete[] _skins[i];
		delete[] _skins;
	}
	_numSkins = 0;
	_skins = 0;

	if(_texCoords) delete[] _texCoords;
	_numTexCoords = 0;
	_texCoords = 0;

	if(_triangles) delete[] _triangles;
	_numTriangles = 0;
	_triangles = 0;

	if(_glCommands) {
		for(i=0;i<_numGLCommands;i++) if(_glCommands[i].vertices) delete[] _glCommands[i].vertices;
		delete[] _glCommands;
	}
	_numGLCommands = 0;
	_glCommands = 0;

	if(_frames) {
		for(i=0;i<_numFrames;i++) if(_frames[i].vertices) delete[] _frames[i].vertices;
		delete[] _frames;
	}
	_numFrames = 0;
	_frames = 0;

	if(_vertexBuffer) delete[] _vertexBuffer;
	_vertexBuffer = 0;

	if(_textureBuffer) delete[] _textureBuffer;
	_textureBuffer = 0;

	//if(_textureBytes) delete[] _textureBytes;
	//_textureBytes = 0;

	_scale = 1.0;
	_skinWidth = 0;
	_skinHeight = 0;
	_frameSize = 0;
	_numVertices = 0;
}

bool MD2Model::loadFromFile(const char *fileName) {

	free();
	size_t i; // generic index
	model_t header;

	FILE *pFile=0;
	if(!(pFile=fopen(fileName,"rb"))) {
		throw "unable to open md2 file";
		return false;
	}

	if(fread(&header,sizeof(model_t),1,pFile)!=1) {
		throw "unable to read header";
		return false;
	}

	if(header.magic!=0x32504449) { // "IDP2"
		throw "no md2 magic";
		return false;
	}

	if(header.version!=0x8) {
		throw "md2 version not 8";
		return false;
	}
	
	_skinWidth = header.skinWidth;
	_skinHeight = header.skinHeight;
	_frameSize = header.frameSize;
	_numVertices = header.numVertices;

	if((_numSkins = header.numSkins)) {
		size_t len;
		char buffer[64];

		_skins = new char*[_numSkins];
		memset(_skins,0,sizeof(char*)*_numSkins); // incase of an error

		if(fseek(pFile,header.offsetSkins,SEEK_SET)!=0) {
			throw "seek error at skins";
			return false;
		}
		for(i=0;i<_numSkins;i++) {
			if(fread(buffer,sizeof(char),64,pFile) != 64) {
				throw "could not read a skin name";
				return false;
			}
			buffer[63] = 0; // safety (who farted?)
			_skins[i] = new char[(len=strlen(buffer)+1)];
			strncpy(_skins[i],buffer,len);
		}
	}

	if((_numTexCoords = header.numTexCoords)) {
		_texCoords = new textureCoordinate_t[_numTexCoords];
		if(fseek(pFile,header.offsetTexCoords,SEEK_SET)!=0) {
			throw "seek error at texture coordinates";
			return false;
		}
		if(fread(_texCoords,sizeof(textureCoordinate_t),_numTexCoords,pFile) != _numTexCoords) {
			throw "could not read texture coordinates";
			return false;
		}
	}

	if((_numTriangles = header.numTriangles)) {
		_triangles = new triangle_t[_numTriangles];
		if(fseek(pFile,header.offsetTriangles,SEEK_SET)!=0) {
			throw "seek error at triangles";
			return false;
		}
		if(fread(_triangles,sizeof(triangle_t),_numTriangles,pFile) != _numTriangles) {
			throw "could not read triangles";
			return false;
		}
	}

	if((_numGLCommands = header.numGlCommands)) {
		_glCommands = new glCommand_t[_numGLCommands];
		memset(_glCommands,0,sizeof(glCommand_t)*_numGLCommands);

		if(fseek(pFile,header.offsetGlCommands,SEEK_SET)!=0) {
			throw "seek error at gl commands";
			return false;
		}

		size_t x,bufferSize=0;
		for(i=0;i<_numGLCommands;i++) {
			if(fread(&(_glCommands[i].count),sizeof(int),1,pFile) != 1) {
				throw "could not read a gl command";
				return false;
			}
			if(_glCommands[i].count > 0) {
				_glCommands[i].type = /*GL_LINE_STRIP;*/ GL_TRIANGLE_STRIP;
			} else if(_glCommands[i].count < 0) {
				_glCommands[i].type = /*GL_LINE_LOOP;*/ GL_TRIANGLE_FAN;
				_glCommands[i].count = -_glCommands[i].count;
			} else break; // end of list

			_glCommands[i].vertices = new glCommandVertex_t[_glCommands[i].count];

			if(fread(_glCommands[i].vertices,sizeof(glCommandVertex_t),_glCommands[i].count,pFile) != _glCommands[i].count) {
				throw "could not read gl command vertices";
				return false;
			}
			
			for(x=0;x<_glCommands[i].count;x++) _glCommands[i].vertices[x].t = (1.0 - _glCommands[i].vertices[x].t);
			
			if(_glCommands[i].count > bufferSize) bufferSize = _glCommands[i].count;
		}
		if(bufferSize) {
			_vertexBuffer = new float[bufferSize*3];
			_textureBuffer = new float[bufferSize*2];
		}
	}

	if((_numFrames = header.numFrames)) {
		_frames = new frame_t[_numFrames];
		memset(_frames,0,sizeof(frame_t)*_numFrames);

		if(fseek(pFile,header.offsetFrames,SEEK_SET)!=0) {
			throw "seek error at frames";
			return false;
		}
		for(i=0;i<_numFrames;i++) {
			if(fread(&_frames[i],sizeof(frame_t),1,pFile) != 1) {
				throw "could not read a frame";
				return false;
			}
			_frames[i].vertices = new triangleVertex_t[_numVertices];

			fseek(pFile,-sizeof(triangleVertex_t*),SEEK_CUR); // step back - should never fail
			if(fread(_frames[i].vertices,sizeof(triangleVertex_t),_numVertices,pFile) != _numVertices) {
				throw "could not read a frame's vertices";
				return false;
			}
		}
	}
	
	return true;
}

bool MD2Model::setTextureBytes(const char *targa) {

	FILE *pFile=fopen(targa,"rb");
	if(!pFile) return false;
	
	size_t size=(256*256*3);
	byte *bytes=new byte[size];

	fread(bytes,sizeof(byte),14,pFile);
	fseek(pFile,bytes[0],SEEK_CUR);
	fread(bytes,sizeof(byte),size,pFile);

	// swap green and blue - stupid tga
	byte temp;
	for(size_t i=0;i<size;i+=3) {
		temp=bytes[i+1];
		bytes[i+1]=bytes[i+2];
		bytes[i+2]=temp;
	}

	glGenTextures(1,&_textureID); // create one texture and get it's ID
	glBindTexture(GL_TEXTURE_2D,_textureID); // make it the current texture

	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,256,256,0,GL_RGB,GL_UNSIGNED_BYTE,bytes);

	delete [] bytes;
	return true;
	/*_textureBytes=bytes;
	
	_textureHandle = 0;
	glGenTextures (1, &_textureHandle);
	glBindTexture (GL_TEXTURE_2D, _textureHandle);

	// Setup texture filters
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Build the texture and generate mipmaps

	// Hardware mipmap generation
	glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glHint (GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);

      glTexImage2D (GL_TEXTURE_2D, 0, getInternalFormat (3),
		    256, 256, 0, img->format (),
		    GL_UNSIGNED_BYTE, _textureBytes);*/
}

/*void MD2Model::renderFrame(const int frame) {

	// Compute max frame index
	//int maxFrame = _header.num_frames - 1;
	
	// Check if the frame index is valid
	if((frame<0) || (frame>(_numFrames-1))) return;
	
	// Bind to model's texture
	//if(_tex) _tex->bind ();

	vec3_t v;

	frame_t *pFrame=&_frames[frame];
	glCommandVertex_t *pGLCmdVertex;
	triangleVertex_t *pVert;
	
	//float s,t;
	
	//glColor4f(0, 1, 0, 1);
	//glBegin (GL_TRIANGLES);
    // Draw each triangle
	int i,j;
	
	float buffer[9];

    for (i=0; i < _numTriangles; ++i) {
		// Draw each vertex of this triangle
		for (j = 0; j < 3; ++j) {
			//Md2Vertex_t *pVert = &pFrame->verts[_triangles[i].vertex[j]];
			//Md2TexCoord_t *pTexCoords = &_texCoords[_triangles[i].st[j]];

			//pGLCmdVertex = &pGLCommand->vertices[ii];
			pVert = &pFrame->vertices[_triangles[i].vertexIndices[j]];

			// Compute final texture coords.
			//s = (pTexCoords->s) / _skinWidth;
			//t = (pTexCoords->t) / _skinHeight;
			
			//glTexCoord2f (s, 1.0f - t);
			
			// Send normal vector to OpenGL
			//glNormal3fv (_kAnorms[pVert->normalIndex]);
			
			// Uncompress vertex position and scale it

			v[0] = (pFrame->scale[0] * pVert->vertex[0] + pFrame->translate[0]) * _scale;
			v[1] = (pFrame->scale[1] * pVert->vertex[1] + pFrame->translate[1]) * _scale;
			v[2] = (pFrame->scale[2] * pVert->vertex[2] + pFrame->translate[2]) * _scale;
			
			//glVertex3fv (v);

		}
		
		glEnableClientState(GL_VERTEX_ARRAY);
		//glEnableClientState(GL_TEXTURE_COORD_ARRAY); 

		glVertexPointer(3, GL_FLOAT, 0, buffer);
		//glTexCoordPointer(2, GL_FLOAT,0,_textureBuffer); 

		glDrawArrays(GL_LINES, 0, 3);
	}
	//glEnd();
}*/

void MD2Model::glRenderFrame(const int frame) {
	if((frame<0) || (frame>=_numFrames)) return;

	size_t i,ii;
	float *t,*v;
	const frame_t *pFrame=&_frames[frame];
	glCommand_t *pGLCommand;
	glCommandVertex_t *pGLCmdVertex;
	triangleVertex_t *pVert;
	
	//glColor4f(0, 0, 0, 1);

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY); 

	// Bind to model's texture
	// if (_tex) _tex->bind ();
	glBindTexture(GL_TEXTURE_2D,_textureID);

	for(i=0;i<_numGLCommands;i++) {

		pGLCommand = &_glCommands[i];

		t=_textureBuffer;
		v=_vertexBuffer;

		for(ii=0;ii<pGLCommand->count;ii++) {

			pGLCmdVertex = &pGLCommand->vertices[ii];
			pVert = &pFrame->vertices[pGLCmdVertex->vertexIndex];

			// Send texture coords. to OpenGL
			*t++ = pGLCmdVertex->s;
			*t++ = pGLCmdVertex->t;//(1.0f - pGLCmdVertex->t);
			
			// Send normal vector to OpenGL
			// glNormal3fv (_kAnorms[pVert->lightNormalIndex]);

			// Uncompress vertex position and scale it
			*v++ = (pFrame->scale[0] * pVert->vertex[0] + pFrame->translate[0]);// * _scale;
			*v++ = (pFrame->scale[1] * pVert->vertex[1] + pFrame->translate[1]);// * _scale;
			*v++ = (pFrame->scale[2] * pVert->vertex[2] + pFrame->translate[2]);// * _scale;
		}

		glVertexPointer(3, GL_FLOAT, 0, _vertexBuffer);
		glTexCoordPointer(2, GL_FLOAT,0,_textureBuffer); 

		glDrawArrays(pGLCommand->type, 0, pGLCommand->count);
    }
}
