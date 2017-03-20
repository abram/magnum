#ifndef Magnum_DebugTools_BufferData_h
#define Magnum_DebugTools_BufferData_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016
              Vladimír Vondruš <mosra@centrum.cz>

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

#ifndef MAGNUM_TARGET_WEBGL
/** @file
 * @brief Function @ref Magnum::DebugTools::bufferData(), @ref Magnum::DebugTools::bufferSubData()
 */
#endif

#include <Corrade/Containers/Array.h>

#include "Magnum/Buffer.h"
#include "Magnum/DebugTools/visibility.h"

#ifndef MAGNUM_TARGET_WEBGL
namespace Magnum { namespace DebugTools {

namespace Implementation {
    MAGNUM_DEBUGTOOLS_EXPORT void bufferSubData(Buffer& buffer, GLintptr offset, GLsizeiptr size, void* output);
}

/**
@brief Buffer subdata

Emulates @ref Buffer::subData() call on platforms that don't support it (such
as OpenGL ES) by using @ref Buffer::map().
@requires_gles30 Extension @extension{EXT,map_buffer_range} in OpenGL ES
    2.0.
@requires_gles Buffer mapping is not available in WebGL.
*/
template<class T> Containers::Array<T> inline bufferSubData(Buffer& buffer, GLintptr offset, GLsizeiptr size) {
    Containers::Array<T> data{std::size_t(size)};
    if(size) Implementation::bufferSubData(buffer, offset, size*sizeof(T), data);
    return data;
}

/**
@brief Buffer data

Emulates @ref Buffer::data() call on platforms that don't support it (such as
OpenGL ES) by using @ref Buffer::map().
@requires_gles30 Extension @extension{EXT,map_buffer_range} in OpenGL ES
    2.0.
@requires_gles Buffer mapping is not available in WebGL.
*/
template<class T = char> Containers::Array<T> inline bufferData(Buffer& buffer) {
    const Int bufferSize = buffer.size();
    CORRADE_ASSERT(bufferSize%sizeof(T) == 0, "Buffer::data(): the buffer size is" << bufferSize << "bytes, which can't be expressed as array of types with size" << sizeof(T), nullptr);
    return bufferSubData<T>(buffer, 0, bufferSize/sizeof(T));
}

}}
#else
#error this header is not available in WebGL build
#endif

#endif
