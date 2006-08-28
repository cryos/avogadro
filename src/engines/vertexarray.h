/***************************************************************************
    copyright            : (C) 2006 by Benoit Jacob
 ***************************************************************************/

/***************************************************************************
 *         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.   *
 *         *
 ***************************************************************************/
#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include <math.h>
#include <QGLWidget>
#include <QPainter>
#include <QImage>
#include <QChar>
#include <QHash>

#include <openbabel/mol.h>
#include <openbabel/obiter.h>

/** USE_FPS_COUNTER: if defined, the GL Widget will show a frames-per-second
 * counter. Use only for testing: this makes the GL Widget constantly
 * redraw, which under normal circumstances is a waste of CPU time.
 */
#define USE_FPS_COUNTER

/** USE_DISPLAY_LISTS: if defined, the whole scene will be stored in
 * an OpenGL display list. The vertex arrays will then be converted into
 * (nested) display lists, in order to avoid replication of geometric data.
 * This option improves performance, especially when rendering complex models,
 * but increases memory usage.
 */
#define USE_DISPLAY_LISTS

namespace Avogadro
{

/**
* This class represents a color in OpenGL float red-green-blue-alpha format.
*
* @author Benoit Jacob
*/
struct Color
{
	///{ The four components of the color, ranging between 0 and 1.
	GLfloat m_red, m_green, m_blue, m_alpha;
	///}

	Color() {}

	/**
	 * This constructor sets the four components of the color
	 * individually. Each one ranges from 0.0 (lowest intensity) to
	 * 1.0 (highest intensity). For the alpha component, 0.0 means fully
	 * transparent and 1.0 (the default) means fully opaque. */
	Color( GLfloat red, GLfloat green, GLfloat blue,
		GLfloat alpha = 1.0 );

	/**
	 * This constructor uses OpenBabel to retrieve the color in which
	 * the atom should be rendered. */
	Color( const OpenBabel::OBAtom *atom );

	Color& operator=( const Color& other );

	/**
	 * Sets this color to be the one used by OpenGL for rendering
	 * when lighting is disabled. */
	inline void apply()
	{
		glColor4fv( reinterpret_cast<GLfloat *>( this ) );
	}

	/**
	* Applies nice OpenGL materials using this color as the
	* diffuse color while using different shades for the ambient and
	* specular colors. This is only useful if lighting is enabled. */
	void applyAsMaterials();
};

/**
* Given a vector U, constructs two unit vectors v and w
* such that (U, v, w) is an orthogonal basis.
* U is not supposed to be normalized.
*
* Returns false if something went wrong.
*/
bool createOrthoBasisGivenFirstVector( const OpenBabel::vector3 &U, OpenBabel::vector3 & v, OpenBabel::vector3 & w );

/*void LinearRegression( const std::list<vector3 *> & points,
	vector3 & ret_plane_base_point, vector3 & ret_plane_normal_vector );
*/

/**
* This is an abstract base class for an OpenGL vertex array, with an option
* (controlled by USE_DISPLAY_LISTS) to compile a display list from it, in which
* case the vertex array is freed and only the display list is kept.
*
* @author Benoit Jacob
*/
class VertexArray
{
	protected:

		/**
		 * This struct represents a vector to be passed to OpenGL as
		 * part of a Vertex Array. Here we don't want to use OpenBabel's
		 * vector3 class, because it uses double-precision coordinates,
		 * which would be a waste of memory here. **/
		struct Vector
		{
			GLfloat x, y, z;
		};

		/** Pointer to the buffer storing the vertex array */
		Vector *m_vertexBuffer;
		/** Pointer to the buffer storing the normal array.
		 * If m_hasSeparateNormalBuffer is false, then this is equal
		 * to m_vertexBuffer. */
		Vector *m_normalBuffer;
		/** Pointer to the buffer storing the indices */
		unsigned short *m_indexBuffer;
		/** The mode in which OpenGL should interpred the vertex arrays
		 * (for example, this could be GL_TRIANGLE_STRIP) */
		GLenum m_mode;
		/** The number of vertices, i.e. the size of m_vertexBuffer
		 * or equivalently m_normalBuffer */
		int m_vertexCount;
		/** The number of indices, i.e. the size of m_indexBuffer */
		int m_indexCount;
		/** The id of the OpenGL display list (used only if this option
		 * is turned on) */
		GLuint m_displayList;
		/** Equals true if there is an index buffer, i.e. if this is an
		 * indexed vertex array */
		bool m_hasIndexBuffer;
		/** If this equals false, then the vertex buffer will also be
		 * used as normal buffer. This allows to divide by 2 the space
		 * taken by a sphere vertex array. For most objects other than
		 * spheres, this should equal true. */
		bool m_hasSeparateNormalBuffer;
		/** Equals true if this is a valid, well-initialized vertex
		 * array */
		bool m_isValid;
		
		/** This pure virtual method should return the number of
		 * vertices, as computed from certain properties determining
		 * the level of detail */
		virtual int getVertexCount() = 0;
		/** This virtual method returns 0, and should be reimplemented
		 * in child classes to return the number of indices
		 * as computed from certain properties determining
		 * the level of detail */
		virtual int getIndexCount() { return 0; }
		/** This method allocates enough memory for the buffers. It
		 * should only be called once m_vertexCount and m_indexCount
		 * have been set. */
		bool allocateBuffers();
		/** This method frees the buffers. If display list compilation
		 * is enabled, then it is safe to call it once the display list
		 * has been compiled.
		 */
		void freeBuffers();
		/** This pure virtual method should fill the buffers with the
		 * geometric data. It should be called only after
		 * allocateBuffers() */
		virtual void buildBuffers() = 0;
		/** If display list compilation is enabled, then this method
		 * compiles the display list and then calls freeBuffers().
		 * It should only be called after buildBuffers(). */
		void compileDisplayList();
		/** This is a convenient method calling getVertexCount(),
		 * getIndexCount(), allocateBuffers(), buildBuffers() and
		 * compileDisplayList() in that order, thus doing all the
		 * initialization, whether or not display list compilation is
		 * enabled. */
		void initialize();
		/** This function draws the vertex array using OpenGL. */
		void do_draw();

	public:
		/** This constructors only sets the values of the member data to
		 * some pre-initialization state. See the initialize() method
		 * for actual initialization. */
		VertexArray( GLenum mode,
			bool hasIndexBuffer,
			bool hasSeparateNormalBuffer );
		/** This destructor frees the buffers if necessary, and also
		 * deletes the display list if it has been compiled. */
		virtual ~VertexArray();

		/** If display list compilation is enabled, then this function
		 * just calls the display list. Otherwise, it calls do_draw().
		 */
		inline void draw()
		{
#ifdef USE_DISPLAY_LISTS
			if( m_isValid ) glCallList( m_displayList );
#else
			if( m_isValid ) do_draw();
#endif
		}
};

/**
* This class represents and draws a sphere. The sphere is computed as a
* "geosphere", that is, one starts with an icosahedron, which is the regular
* solid with 20 triangular faces, and one then sub-tesselates each face into
* smaller triangles. This is a classical algorithm, known to give very good
* results.
*
* @author Benoit Jacob
*/
class Sphere : public VertexArray
{
	private:
		/** computes the index (position inside the index buffer)
		 * of a vertex given by its position (strip, column, row)
		 * inside a certain flat model of the sub-tesselated
		 * icosahedron */
		inline unsigned short indexOfVertex(
			int strip, int column, int row);
		/** computes the coordinates
		 * of a vertex given by its position (strip, column, row)
		 * inside a certain flat model of the sub-tesselated
		 * icosahedron */
		void computeVertex( int strip, int column, int row );

	protected:
		/** the detail-level of the sphere. Must be at least 1.
		 * This is interpreted as the number of sub-edges into which
		 * each edge of the icosahedron must be split. So the
		 * number of faces of the sphere is simply:
		 * 20 * detail^2. When detail==1, the sphere is just the
		 * icosahedron */
		int m_detail;

		virtual int getVertexCount();
		virtual int getIndexCount();
		virtual void buildBuffers();

	public:
		Sphere();
		virtual ~Sphere() {}

		/** initializes the sphere with given level of detail. If the
		 * sphere was already initialized, any pre-allocated buffers
		 * are freed and then re-allocated.
		@param detail the wanted level of detail. See m_detail member */
		virtual void setup( int detail );

		/** draws the sphere at specifiec position and with
		 * specified radius */
		virtual void draw( const OpenBabel::vector3 &center, double radius );
};

/**
* This class represents and draws a cylinder
*
* @author Benoit Jacob
*/
class Cylinder : public VertexArray
{
	protected:
		/** the number of faces of the cylinder. This only
		 * includes the lateral faces, as the base and top faces (the
		 * two discs) are not rendered. */
		int m_faces;

		virtual int getVertexCount();
		virtual void buildBuffers();

	public:
		Cylinder();
		virtual ~Cylinder() {}
		/** initializes the cylinder with given number of faces. If the
		 * cylinder was already initialized, any pre-allocated buffers
		 * are freed and then re-allocated */
		virtual void setup( int faces );
		/**
		 * draws the cylinder at specified position, with specified
		 * radius. the order and shift arguments allow to render
		 * multiple cylinders at once. If you only want to render one
		 * cylinder, leave order and shift at their default values.
		 @param end1 the position of the first end of the cylinder.
			that is, the center of the first disc-shaped face.
		 @param end2 the position of the second end of the cylinder.
			that is, the center of the second disc-shaped face.
		 @param radius the radius of the cylinder
		 @param order to render only one cylinder, leave this set to
			the default value, which is 1. If order>1, then order
			parallel cylinders are drawn around the axis
			(end1 - end2).
		@param order this is only meaningful of order>1, otherwise
			just let this set to the default value. When order>1,
			this is interpreted as the displacement of the axis
			of the drawn cylinders from the axis (end1 - end2).
		 */
		virtual void draw( const OpenBabel::vector3 &end1, const OpenBabel::vector3 &end2,
			double radius, int order = 1, double shift = 0.0 );
};

/** BEEP BEEP BEEP this will likely be removed as Qt 4.2 has something better
*
* This is a helper class for TextRenderer, and should probably never be
* used directly. See TextRenderer.
*
* The CharRenderer class represents a character stored as OpenGL rendering
* data : a texture object and a display list mapping it on a quad and then
* translating to the right of it.
*
* See the m_charTable member of TextRenderer for an example of use of
* this class.
*/
class CharRenderer
{
	protected:
		/**
		 * The OpenGL texture object
		 */
		GLuint m_texture;

		/**
		 * The OpenGL display list
		 */
		GLuint m_displayList;

		/**
		 * Width and height in pixels of the rendered character
		 */
		int m_width, m_height;

	public:
		CharRenderer();
		~CharRenderer();
		bool initialize( QChar c, const QFont &font,
			GLenum textureTarget );
		inline void draw()
		{
			glCallList( m_displayList );
		}
};


/** BEEP BEEP BEEP this will likely be removed as Qt 4.2 has something better
*
* This class renders text inside a QGLWidget. It replaces the functionality
* of QGLWidget::renderText(). The advantages over renderText() include:
*  - supports any font, any character encoding supported by Qt
*    (renderText is 8-bit-only and can only use "OpenGL-compatible" fonts)
*  - does not use any library outside Qt (renderText uses FreeType on X11)
*  - renders characters as textured quads instead of calling glDrawPixels,
*    which does not make much of a difference on MesaGL, but can be a lot
*    faster and safer with other (buggy) OpenGL implementations. It will also
*    allow to add more graphical effects in the future, like rotation,
*    if we ever need that.
*  - the characters are stored as 8bpp Alpha, which takes 4 times less
*    memory than the 32bpp RGBA used by renderText.
*  - the characters are rendered on-the-fly on the first time they appear
*    in a QString being printed. This is achieved using a QHash to test whether
*    a character has already been rendered.
*
* Recommended usage:
* The TextRender class is meant to be used from inside a child class of
* QGLWidget, say MyGLWidget.
*
* In the declaration of MyGLWidget, please declare a TextRenderer member:
* 
* @code
class MyGLWidget : public QGLWidget
{
	...
	TextRenderer m_textRenderer;
	...
};
* @endcode
* 
* Now, in the constructor of MyGLWidget, please call setup() along these lines:
*
* @code
	QFont f;
	f.setStyleHint( QFont::SansSerif, QFont::PreferAntialias );
	m_textRenderer.setup( this, f );
* @endcode 
*
* The setup() method should be called only once, which means you have to choose
* a font once and for all, in the lifetime of your TextRenderer. Any QFont can
* be used, the above is just an example. Now, to actually render text, in
* the MyGLWidget::paintGL() method, you can call

* @code
	m_textRenderer.print( x, y, string );
* @endcode

* where x,y are ints and string is any QString. If you want to choose a color,
* please call glColor3f or glColor4f before calling print(). Of course you can
* also call qglColor or Color::apply. You can achieve semitransparent text at
* no additional cost by choosing a semitransparent color.
*
* If you wish to do several calls to print(), it will improve performance
* to enclose them between a call to begin() and a call to end(), like that:
* 
* @code
	m_textRenderer.begin();
	m_textRenderer.print( x1, y1, string1 );
	m_textRenderer.print( x2, y2, string2 );
	m_textRenderer.print( x3, y2, string3 );
	m_textRenderer.end();
* @endcode
* 
* Please make sure, though, that no relevant OpenGL state change occurs between
* begin() and end(), except the state changes performed by the TextRenderer
* itself. In other words, please avoid calling glSomething() between begin() and
* end(), except if you are sure that this call won't perform a relevant state
* change.
*
* The print() method when called alone, or the begin()-print()-end() group,
* do restore the OpenGL state as they found it, including the matrices.
*
* If you experience rendering problems, you can try the following:
* - disable some OpenGL state bits. For instance, TextRenderer automatically
*   disables fog and lighting during rendering, because it doesn't work
*   correctly with them enabled. There probably are other OpenGL state bits
*   that have to be disabled, so if your program enables some of them, you
*   might have to disable them before rendering text.
* - if you experience poor font quality, please consider using an antialiased
*   font.
*
* @author Benoit Jacob
*/
class TextRenderer
{
	protected:
		/**
		 * The font used for rendering the chars. This is set
		 * once and for all by setup(). Note that it is stored
		 * by value, so the caller doesn't have to keep it alive.
		 */
		QFont m_font;
		
		/**
		 * This hash gives the correspondence table between QChars
		 * (the keys) and the corresponding CharRenderers (the values).
		 * Every time a QChar is being met, either it is found in this
		 * table, in which case it can be directly rendered, or it is
		 * not found, in which case a new CharRenderer is created for
		 * it and added to this table.
		 */
		QHash<QChar, CharRenderer*> m_charTable;

		/**
		 * The QGLWidget in which to render. This is set
		 * once and for all by setup().
		 */
		const QGLWidget *m_glwidget;

		/**
		 * This equals true if begin() has been called, but end() hasn't
		 * since.
		 */
		GLboolean m_isBetweenBeginAndEnd;

		///{ Members used to remember the OpenGL state in order to be able to restore it after rendering. See do_end().
		GLboolean m_wasEnabled_LIGHTING;
		GLboolean m_wasEnabled_textureTarget;
		GLboolean m_wasEnabled_FOG;
		GLboolean m_wasEnabled_BLEND;
		GLboolean m_wasEnabled_DEPTH_TEST;
		///}

		GLenum m_textureTarget;

		/**
		 * Stores the relevant part of the OpenGL state, and prepares
		 * for rendering
		 */
		void do_begin();

		/**
		 * Restores the OpenGL state
		 */
		void do_end();

	public:
		TextRenderer();
		~TextRenderer();
		
		/**
		 * This should be called only once, before any printing occurs.
		 * @param glwidget The QGLWidget in which to render.
		 * See m_glwidget member.
		 * @param font The QFont to use. See m_font member.
		 */
		void setup( const QGLWidget *glwidget, const QFont &font );

		/**
		 * Prints text at the position (x,y) in window coordinates
		 * (0,0) is the bottom left corner
		 * @param x the x-coordinate
		 * @param y the y-coordinate
		 * @param string the QString to print
		 */
		void print( int x, int y, const QString &string);

		/**
		 * Call this before doing multiple calls to print(). This is
		 * not necessary, but will improve performance. Don't forget,
		 * then, to call end() after.
		 */
		void begin();

		/**
		 * Call this after having called begin() and print().
		 */
		void end();
};

} // namespace Avogadro

#endif // VERTEXARRAY_H

