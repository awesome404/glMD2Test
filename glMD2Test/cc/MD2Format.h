#ifndef __MD2FORMAT_H__
#define __MD2FORMAT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char byte;

struct model_t;
struct triangleVertex_t;
struct frame_t;
struct triangle_t;
struct textureCoordinate_t;
struct glCommandVertex_t;

/**

.md2 File Format Specification

by Daniel E. Schoenblum

http://linux.ucla.edu/~phaethon/q3a/formats/md2-schoenblum.html

 **

INTRO

This page will try and give some sort of technical documentation on the Quake2
model format (.md2).

These specs can be used freely for whatever you want. I only ask that people
send me corrections, suggestions, etc.

Quake2 models are stored in files with the .md2 extension. This is a custom
format used only by Quake2 and (probably) Quake2 mission packs. md2 files can
be generated from various other file formats by tools provided freely by id,
in original and modified form. A single md2 file contains the model's geometry,
frame information, skin filename(s), and texture coordinates. The file is
little-endian (intel byte ordering).

HEADER

The header comes right at the start of the file. The information in the header
is needed to load different parts of the model.

*/

struct model_t { 
	int magic;
	int version;
	int skinWidth;
	int skinHeight;
	int frameSize;
	int numSkins;
	int numVertices;
	int numTexCoords;
	int numTriangles;
	int numGlCommands;
	int numFrames;
	int offsetSkins;
	int offsetTexCoords;
	int offsetTriangles;
	int offsetFrames;
	int offsetGlCommands;
	int offsetEnd;
};

/*

int magic: A "magic number" used to identify the file. The magic number is
           844121161 in decimal (0x32504449 in hexadecimal). The magic number is
		   equal to the int "IDP2" (id polygon 2), which is formed by
		   ('I' + ('D' << 8) + ('P' << 16) + ('2' << 24)).

int version: Version number of the file. Always 8.

int skinWidth: Width of the skin(s) in pixels.

int skinHeight: Height of the skin(s) in pixels.

int frameSize: Size of each frame in bytes.

int numSkins: Number of skins associated with this model.

int numVertices: Number of vertices in each frame.

int numTexCoords: Number of texture coordinates (not necessarily the same as the
                  number of vertices).

int numTriangles: Number of triangles in each frame.

int numGlCommands: Number of dwords (4 bytes) in the gl command list.

int numFrames: Number of frames.

int offsetSkins: Offset, in bytes from the start of the file, to the list of
                 skin names.

int offsetTexCoords: Offset, in bytes from the start of the file, to the list of
                     texture coordinates.

int offsetTriangles: Offset, in bytes from the start of the file, to the list of
                     triangles.

int offsetFrames: Offset, in bytes from the start of the file, to the list of
                  frames.

int offsetGlCommands: Offset, in bytes from the start of the file, to the gl
                      command list.

int offsetEnd: Offset, in bytes from the start of the file, to the end (size of
               the file).

FRAMES

Each frame contains the positions in 3D space for each vertex of each triangle
that makes up the model. Quake 2 (and Quake) models contain only triangles.

*/

struct triangleVertex_t {
	byte vertex[3];
	byte lightNormalIndex;
};

/*

byte vertex[3]: The three bytes represent the x, y, and z coordinates of this
                vertex. This is not the "real" vertex coordinate. This is a
				scaled version of the coordinate, scaled so that each of the
				three numbers fit within one byte. To scale the vertex back to
				the "real" coordinate, you need to first multiply each of the
				bytes by their respective float scale in the frame_t structure,
				and then add the respective float translation< /a>, also in the
				frame_t structure. This will give you the vertex coordinate
				relative to the model's origin, which is at the origin,
				(0, 0, 0).

byte lightNormalIndex: This is an index into a table of normals kept by Quake2.
                       To get the table, you need to download this zip file
					   (1.7 MB), released by id, that has the source code to all
					   of the tools they used for quake2.
*/

struct frame_t {
	float scale[3];
	float translate[3];
	char name[16];
	triangleVertex_t *vertices;
};

/*

frame_t is a variable sized structure, however all frame_t structures within the
same file will have the same size (numVertices in the header)

float scale[3]: This is a scale used by the vertex member of the
                triangleVertex_t structure.

float translate[3]: This is a translation used by the vertex member of the
                    triangleVertex_t structure.

char name[16]: This is a name for the frame.

triangleVertex_t vertices[1]: An array of numVertices triangleVertex_t
                              structures.

TRIANGLES

Quake 2 models are made up of only triangles. At offsetTriangles in the file is
an array of triangle_t structures. The array has numTriangles structures in it.

*/

struct triangle_t {
	short vertexIndices[3];
	short textureIndices[3];
};

/*

short vertexIndices: These three shorts are indices into the array of vertices
                     in each frames. In other words, the number of triangles in
					 a md2 file is fixed, and each triangle is always made of
					 the same three indices into each frame's array of vertices.
					 So, in each frame, the triangles themselves stay intact,
					 their vertices are just moved around.

short textureIndices: These three shorts are indices into the array of texture
					  coordinates.

SKINS

There is an array of numSkins skin names stored at offsetSkins into the file.
Each skin name is a char[64]. The name is really a path to the skin, relative to
the base game directory (baseq2 f or "standard" Quake2). The skin files are
regular pcx files.

*/

struct textureCoordinate_t {
	short s, t;
};

/*

short s, t: These two shorts are used to map a vertex onto a skin. The
            horizontal axis position is given by s, and the vertical axis
			position is given by t. The range for s is greater than or equal to
			0 and less than skinWidth< /a> (0 <= s < skinWidth). The range for t
			is greater than or equal to 0 and less than skinHeight
			(0 <= s < skinHeight). N ote that the ranges are different than in
			the s and t members of the glCommandVertex structure.

GL COMMANDS

At offsetGlCommands bytes into the file, there is the gl command list, which is
made up of a series of numGlCommands int's and float's, organized into groups.
Each group starts with an int. If it is positive, it is followed by that many
glCommandVertex_t structures, which form a triangle strip. If it is negative, it
is followed by -x glCommandVertex_t structures, which form a triangle fan. A 0
indicates the end of the list. The list is an optimized way of issuing commands
when rendering with OpenGl.

*/

struct glCommandVertex_t {
	float s, t;
	int vertexIndex;
};

/*

float s, t: These two floats are used to map a vertex onto a skin. The
horizontal axis position is given by s, and the vertical axis position is given
by t. The range for s and for t is 0.0 to 1.0. Note that the ranges are
different than in the textureCoordinate_t structure. They are stored as floats
here because that's the way Quake2 passes them to OpenGl.

int vertexIndex: Index into the array of vertices stored in each frame.

MAXIMUMS

Quake2 has some pre-defined limits, so that dynamic memory does not need to be
used. You can use these to your advantage to speed up loading if you want.

Triangles: 4096
Vertices: 2048
Texture Coordinates: 2048
Frames: 512
Skins: 32

Quake and Quake2 are trademarks of id Software.
All trademarks used are properties of their respective owners.

*/

#ifdef __cplusplus
}
#endif

#endif
