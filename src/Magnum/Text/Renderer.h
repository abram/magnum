#ifndef Magnum_Text_Renderer_h
#define Magnum_Text_Renderer_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file Text/Renderer.h
 * @brief Class @ref Magnum::Text::AbstractRenderer, @ref Magnum::Text::Renderer, typedef @ref Magnum::Text::Renderer2D, @ref Magnum::Text::Renderer3D
 */

#include "Magnum/configure.h"

#ifdef MAGNUM_TARGET_GL
#include <string>
#include <tuple>
#include <vector>

#include "Magnum/DimensionTraits.h"
#include "Magnum/Math/Range.h"
#include "Magnum/GL/Buffer.h"
#include "Magnum/GL/Mesh.h"
#include "Magnum/Text/Text.h"
#include "Magnum/Text/Alignment.h"
#include "Magnum/Text/visibility.h"

#ifdef CORRADE_TARGET_EMSCRIPTEN
#include <Corrade/Containers/Array.h>
#endif

namespace Magnum { namespace Text {

/**
@brief Base for text renderers

Not meant to be used directly, see the @ref Renderer class for more
information.
@see @ref Renderer2D, @ref Renderer3D
*/
class MAGNUM_TEXT_EXPORT AbstractRenderer {
    public:
        /**
         * @brief Render text
         * @param font          Font
         * @param cache         Glyph cache
         * @param size          Font size in points
         * @param text          Text to render
         * @param alignment     Text alignment
         *
         * Returns tuple with vertex positions, texture coordinates, indices
         * and rectangle spanning the rendered text.
         */
        static std::tuple<std::vector<Vector2>, std::vector<Vector2>, std::vector<UnsignedInt>, Range2D> render(AbstractFont& font, const GlyphCache& cache, Float size, const std::string& text, Alignment alignment = Alignment::LineLeft);

        /**
         * @brief Capacity for rendered glyphs
         *
         * @see @ref reserve()
         */
        UnsignedInt capacity() const { return _capacity; }

        /**
         * @brief Font size in points
         * @m_since_latest
         */
        Float fontSize() const { return _fontSize; }

        /** @brief Rectangle spanning the rendered text */
        Range2D rectangle() const { return _rectangle; }

        /** @brief Vertex buffer */
        GL::Buffer& vertexBuffer() { return _vertexBuffer; }

        /** @brief Index buffer */
        GL::Buffer& indexBuffer() { return _indexBuffer; }

        /** @brief Mesh */
        GL::Mesh& mesh() { return _mesh; }

        /**
         * @brief Reserve capacity for rendered glyphs
         *
         * Reallocates memory in buffers to hold @p glyphCount glyphs and
         * prefills index buffer. Consider using appropriate @p vertexBufferUsage
         * if the text will be changed frequently. Index buffer is changed
         * only by calling this function, thus @p indexBufferUsage generally
         * doesn't need to be so dynamic if the capacity won't be changed much.
         *
         * Initially zero capacity is reserved.
         * @see @ref capacity()
         */
        void reserve(UnsignedInt glyphCount, GL::BufferUsage vertexBufferUsage, GL::BufferUsage indexBufferUsage);

        /**
         * @brief Render text
         *
         * Renders the text to vertex buffer, reusing index buffer already
         * filled with @ref reserve(). Rectangle spanning the rendered text is
         * available through @ref rectangle().
         *
         * Initially no text is rendered.
         * @attention The capacity must be large enough to contain all glyphs,
         *      see @ref reserve() for more information.
         */
        void render(const std::string& text);

    #ifndef DOXYGEN_GENERATING_OUTPUT
    protected:
    #else
    private:
    #endif
        explicit MAGNUM_TEXT_LOCAL AbstractRenderer(AbstractFont& font, const GlyphCache& cache, Float size, Alignment alignment);

        ~AbstractRenderer();

        GL::Mesh _mesh;
        GL::Buffer _vertexBuffer, _indexBuffer;
        #ifdef CORRADE_TARGET_EMSCRIPTEN
        Containers::Array<UnsignedByte> _vertexBufferData, _indexBufferData;
        #endif

    private:
        AbstractFont& font;
        const GlyphCache& cache;
        Float _fontSize;
        Alignment _alignment;
        UnsignedInt _capacity;
        Range2D _rectangle;

        #if defined(MAGNUM_TARGET_GLES2) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        typedef void*(*BufferMapImplementation)(GL::Buffer&, GLsizeiptr);
        static MAGNUM_TEXT_LOCAL void* bufferMapImplementationFull(GL::Buffer& buffer, GLsizeiptr length);
        static MAGNUM_TEXT_LOCAL void* bufferMapImplementationRange(GL::Buffer& buffer, GLsizeiptr length);
        static BufferMapImplementation bufferMapImplementation;
        #else
        #ifndef CORRADE_TARGET_EMSCRIPTEN
        static
        #else
        MAGNUM_TEXT_LOCAL
        #endif
        void* bufferMapImplementation(GL::Buffer& buffer, GLsizeiptr length);
        #endif

        #if defined(MAGNUM_TARGET_GLES2) && !defined(CORRADE_TARGET_EMSCRIPTEN)
        typedef void(*BufferUnmapImplementation)(GL::Buffer&);
        static MAGNUM_TEXT_LOCAL void bufferUnmapImplementationDefault(GL::Buffer& buffer);
        static MAGNUM_TEXT_LOCAL BufferUnmapImplementation bufferUnmapImplementation;
        #else
        #ifndef CORRADE_TARGET_EMSCRIPTEN
        static
        #else
        MAGNUM_TEXT_LOCAL
        #endif
        void bufferUnmapImplementation(GL::Buffer& buffer);
        #endif
};

/**
@brief Text renderer

Lays out the text into mesh using given font. Use of ligatures, kerning etc.
depends on features supported by particular font and its layouter.

@section Text-Renderer-usage Usage

Immutable text (e.g. menu items, credits) can be simply rendered using static
methods, returning result either as data arrays or as fully configured mesh.
The text can be then drawn as usual by configuring the shader and drawing the
mesh:

@snippet MagnumText.cpp Renderer-usage1

See @ref render(AbstractFont&, const GlyphCache&, Float, const std::string&, Alignment)
and @ref render(AbstractFont&, const GlyphCache&, Float, const std::string&, GL::Buffer&, GL::Buffer&, GL::BufferUsage, Alignment)
for more information.

While this method is sufficient for one-shot rendering of static texts, for
mutable texts (e.g. FPS counters, chat messages) there is another approach
that doesn't recreate everything on each text change:

@snippet MagnumText.cpp Renderer-usage2

@subsection Text-Renderer-usage-font-size Font size

As mentioned in @ref Text-AbstractFont-usage-font-size "AbstractFont class documentation",
the size at which the font is loaded is decoupled from the size at which a
concrete text is rendered. In particular, with a concrete projection matrix,
the size you pass to either @ref render() or to the @ref Renderer() constructor
will always result in the same size of the rendered text, independently of the
size the font was loaded in. Size of the loaded font is the size at which the
glyphs get prerendered into the glyph cache, affecting visual quality.

When rendering the text, there are two common approaches --- either setting up
the size to match a global user interface scale, or having the text size
proportional to the window size. The first approach results in e.g. a 12 pt
font matching a 12 pt font in other applications, and is what's shown in the
above snippets. The most straightforward way to achieve that is to set up the
projection matrix size to match actual window pixels, such as @ref Platform::Sdl2Application::windowSize() "Platform::*Application::windowSize()".
If using the regular @ref GlyphCache, for best visual quality it should be
created with the @ref AbstractFont loaded at the same size as the text to be
rendered, although often a double supersampling achieves a crisper result.
I.e., loading the font with 24 pt, but rendering with 12 pt. See below for
@ref Text-Renderer-usage-font-size-dpi "additional considerations for proper DPI awareness".

The second approach, with text size being relative to the window size, is for
cases where the text is meant to match surrounding art, such as in a game menu.
In this case the projection size is usually something arbitrary that doesn't
match window pixels, and the text point size then has to be relative to that.
For this use case a @ref DistanceFieldGlyphCache is the better match, as it can
provide text at different sizes with minimal quality loss. See its
documentation for details about picking the right font size and other
parameters for best results.

@subsection Text-Renderer-usage-font-size-dpi DPI awareness

To achieve crisp rendering and/or text size matching other applications on
HiDPI displays, additional steps need to be taken. There are two separate
concepts for DPI-aware rendering:

-   Interface size --- size at which the interface elements are positioned on
    the screen. Often, for simplicity, the interface is using some "virtual
    units", so a 12 pt font is still a 12 pt font independently of how the
    interface is scaled compared to actual display properties (for example by
    setting a global 150% scale in the desktop environment, or by zooming a
    browser window). The size used by the @ref Renderer should match these
    virtual units.
-   Framebuffer size --- how many pixels is actually there. If a 192 DPI
    display has a 200% interface scale, a 12 pt font would be 32 pixels. But if
    it only has a 150% scale, all interface elements will be smaller, and a 12
    pt font would be only 24 pixels. The size used by the @ref AbstractFont and
    @ref GlyphCache should be chosen with respect to the actual physical
    pixels.

When using for example @ref Platform::Sdl2Application or other `*Application`
implementations, you usually have three values at your disposal ---
@ref Platform::Sdl2Application::windowSize() "windowSize()",
@ref Platform::Sdl2Application::framebufferSize() "framebufferSize()" and
@ref Platform::Sdl2Application::dpiScaling() "dpiScaling()". Their relation is
documented thoroughly in @ref Platform-Sdl2Application-dpi, for this particular
case a scaled interface size, used instead of window size for the projection,
would be calculated like this:

@snippet MagnumText.cpp Renderer-dpi-interface-size

And a multiplier for the @ref AbstractFont and @ref GlyphCache font size like
this. The @ref Renderer keeps using the size without this multiplier.

@snippet MagnumText.cpp Renderer-dpi-size-multiplier

@section Text-Renderer-required-opengl-functionality Required OpenGL functionality

Mutable text rendering requires @gl_extension{ARB,map_buffer_range} on desktop
OpenGL (also part of OpenGL ES 3.0). If @gl_extension{EXT,map_buffer_range} is not
available in ES 2.0, at least @gl_extension{OES,mapbuffer} must be supported for
asynchronous buffer updates. There is no similar extension in WebGL, thus plain
(and slow) buffer updates are used there.

@see @ref Renderer2D, @ref Renderer3D, @ref AbstractFont,
    @ref Shaders::VectorGL, @ref Shaders::DistanceFieldVectorGL
*/
template<UnsignedInt dimensions> class MAGNUM_TEXT_EXPORT Renderer: public AbstractRenderer {
    public:
        /**
         * @brief Render text
         * @param font          Font
         * @param cache         Glyph cache
         * @param size          Font size
         * @param text          Text to render
         * @param vertexBuffer  Buffer where to store vertices
         * @param indexBuffer   Buffer where to store indices
         * @param usage         Usage of vertex and index buffer
         * @param alignment     Text alignment
         *
         * Returns mesh prepared for use with @ref Shaders::VectorGL or
         * @ref Shaders::DistanceFieldVectorGL and rectangle spanning the
         * rendered text.
         */
        static std::tuple<GL::Mesh, Range2D> render(AbstractFont& font, const GlyphCache& cache, Float size, const std::string& text, GL::Buffer& vertexBuffer, GL::Buffer& indexBuffer, GL::BufferUsage usage, Alignment alignment = Alignment::LineLeft);

        /**
         * @brief Constructor
         * @param font          Font
         * @param cache         Glyph cache
         * @param size          Font size
         * @param alignment     Text alignment
         */
        explicit Renderer(AbstractFont& font, const GlyphCache& cache, Float size, Alignment alignment = Alignment::LineLeft);
        Renderer(AbstractFont&, GlyphCache&&, Float, Alignment alignment = Alignment::LineLeft) = delete; /**< @overload */

        #ifndef DOXYGEN_GENERATING_OUTPUT
        using AbstractRenderer::render;
        #endif
};

/** @brief Two-dimensional text renderer */
typedef Renderer<2> Renderer2D;

/** @brief Three-dimensional text renderer */
typedef Renderer<3> Renderer3D;

}}
#else
#error this header is available only in the OpenGL build
#endif

#endif
