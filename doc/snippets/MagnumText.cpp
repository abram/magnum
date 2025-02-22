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

/* In order to have the CORRADE_PLUGIN_REGISTER() macro not a no-op. Doesn't
   affect anything else. */
#define CORRADE_STATIC_PLUGIN

#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/StringView.h>
#include <Corrade/Containers/StringStl.h> /** @todo remove once file callbacks are <string>-free */
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Path.h>
#include <Corrade/Utility/Resource.h>

#include "Magnum/FileCallback.h"
#include "Magnum/Math/Color.h"
#include "Magnum/Math/Matrix3.h"
#include "Magnum/Shaders/VectorGL.h"
#include "Magnum/Text/AbstractFont.h"
#include "Magnum/Text/AbstractFontConverter.h"
#include "Magnum/Text/DistanceFieldGlyphCache.h"
#include "Magnum/Text/Renderer.h"

#define DOXYGEN_ELLIPSIS(...) __VA_ARGS__

using namespace Magnum;
using namespace Magnum::Math::Literals;

namespace MyNamespace {

struct MyFont: Text::AbstractFont {
    explicit MyFont(PluginManager::AbstractManager& manager, Containers::StringView plugin): Text::AbstractFont{manager, plugin} {}

    Text::FontFeatures doFeatures() const override { return {}; }
    bool doIsOpened() const override { return false; }
    void doClose() override {}
    UnsignedInt doGlyphId(char32_t) override { return {}; }
    Vector2 doGlyphAdvance(UnsignedInt) override { return {}; }
    Containers::Pointer<Text::AbstractLayouter> doLayout(const Text::AbstractGlyphCache&, Float, const std::string&) override { return {}; }
};
struct MyFontConverter: Text::AbstractFontConverter {
    explicit MyFontConverter(PluginManager::AbstractManager& manager, Containers::StringView plugin): Text::AbstractFontConverter{manager, plugin} {}

    Text::FontConverterFeatures doFeatures() const override { return {}; }
};

}

/* [MAGNUM_TEXT_ABSTRACTFONT_PLUGIN_INTERFACE] */
CORRADE_PLUGIN_REGISTER(MyFont, MyNamespace::MyFont,
    MAGNUM_TEXT_ABSTRACTFONT_PLUGIN_INTERFACE)
/* [MAGNUM_TEXT_ABSTRACTFONT_PLUGIN_INTERFACE] */

/* [MAGNUM_TEXT_ABSTRACTFONTCONVERTER_PLUGIN_INTERFACE] */
CORRADE_PLUGIN_REGISTER(MyFontConverter, MyNamespace::MyFontConverter,
    MAGNUM_TEXT_ABSTRACTFONTCONVERTER_PLUGIN_INTERFACE)
/* [MAGNUM_TEXT_ABSTRACTFONTCONVERTER_PLUGIN_INTERFACE] */

namespace {
    Vector2i windowSize() { return {}; }
    Vector2i framebufferSize() { return {}; }
    Vector2 dpiScaling() { return {}; }
}

int main() {

{
/* [AbstractFont-usage] */
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font =
    manager.loadAndInstantiate("StbTrueTypeFont");
if(!font->openFile("font.ttf", 12.0f))
    Fatal{} << "Can't open font.ttf with StbTrueTypeFont";

Text::GlyphCache cache{Vector2i{128}};
font->fillGlyphCache(cache, "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "0123456789?!:;,. ");
/* [AbstractFont-usage] */
}

{
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font =
    manager.loadAndInstantiate("StbTrueTypeFont");
/* [AbstractFont-usage-data] */
Utility::Resource rs{"data"};
Containers::ArrayView<const char> data = rs.getRaw("font.ttf");
if(!font->openData(data, 12.0f))
    Fatal{} << "Can't open font data with StbTrueTypeFont";
/* [AbstractFont-usage-data] */
}

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font = manager.loadAndInstantiate("SomethingWhatever");
/* [AbstractFont-usage-callbacks] */
struct Data {
    std::unordered_map<std::string, Containers::Optional<
        Containers::Array<const char, Utility::Path::MapDeleter>>> files;
} data;

font->setFileCallback([](const std::string& filename,
    InputFileCallbackPolicy policy, Data& data)
        -> Containers::Optional<Containers::ArrayView<const char>>
    {
        auto found = data.files.find(filename);

        /* Discard the memory mapping, if not needed anymore */
        if(policy == InputFileCallbackPolicy::Close) {
            if(found != data.files.end()) data.files.erase(found);
            return {};
        }

        /* Load if not there yet. If the mapping fails, remember that to not
           attempt to load the same file again next time. */
        if(found == data.files.end()) found = data.files.emplace(
            filename, Utility::Path::mapRead(filename)).first;

        if(!found->second) return {};
        return Containers::arrayView(*found->second);
    }, data);

font->openFile("magnum-font.conf", 13.0f);
/* [AbstractFont-usage-callbacks] */
}
#endif

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font = manager.loadAndInstantiate("SomethingWhatever");
/* [AbstractFont-setFileCallback] */
font->setFileCallback([](const std::string& filename,
    InputFileCallbackPolicy, void*) {
        Utility::Resource rs{"data"};
        return Containers::optional(rs.getRaw(filename));
    });
/* [AbstractFont-setFileCallback] */
}

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
Containers::Pointer<Text::AbstractFont> font = manager.loadAndInstantiate("SomethingWhatever");
/* [AbstractFont-setFileCallback-template] */
const Utility::Resource rs{"data"};
font->setFileCallback([](const std::string& filename,
    InputFileCallbackPolicy, const Utility::Resource& rs) {
        return Containers::optional(rs.getRaw(filename));
    }, rs);
/* [AbstractFont-setFileCallback-template] */
}

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
/* [DistanceFieldGlyphCache-usage] */
Containers::Pointer<Text::AbstractFont> font = DOXYGEN_ELLIPSIS(manager.loadAndInstantiate(""));
font->openFile("font.ttf", 96.0f);

Text::DistanceFieldGlyphCache cache{Vector2i{1024}, Vector2i{128}, 12};
font->fillGlyphCache(cache, "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "0123456789?!:;,. ");
/* [DistanceFieldGlyphCache-usage] */
}

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
/* [GlyphCache-usage] */
Containers::Pointer<Text::AbstractFont> font = DOXYGEN_ELLIPSIS(manager.loadAndInstantiate(""));
font->openFile("font.ttf", 12.0f);

Text::GlyphCache cache{Vector2i{128}};
font->fillGlyphCache(cache, "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "0123456789?!:;,. ");
/* [GlyphCache-usage] */
}

{
/* -Wnonnull in GCC 11+  "helpfully" says "this is null" if I don't initialize
   the font pointer. I don't care, I just want you to check compilation errors,
   not more! */
PluginManager::Manager<Text::AbstractFont> manager;
/* [Renderer-usage1] */
/* Font instance, received from a plugin manager */
Containers::Pointer<Text::AbstractFont> font = DOXYGEN_ELLIPSIS(manager.loadAndInstantiate(""));

/* Open a 12 pt font */
font->openFile("font.ttf", 12.0f);

/* Populate a glyph cache */
Text::GlyphCache cache{Vector2i{128}};
font->fillGlyphCache(cache, "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "0123456789?!:;,. ");

Shaders::VectorGL2D shader;
GL::Buffer vertexBuffer, indexBuffer;
GL::Mesh mesh;

/* Render a 12 pt text, centered */
std::tie(mesh, std::ignore) = Text::Renderer2D::render(*font, cache, 12.0f,
    "Hello World!", vertexBuffer, indexBuffer, GL::BufferUsage::StaticDraw,
    Text::Alignment::LineCenter);

/* Projection matrix is matching application window size to have the size match
   12 pt in other applications, assuming a 96 DPI display and no UI scaling. */
Matrix3 projectionMatrix = Matrix3::projection(Vector2{windowSize()});

/* Draw the text on the screen */
shader
    .setTransformationProjectionMatrix(projectionMatrix)
    .setColor(0xffffff_rgbf)
    .bindVectorTexture(cache.texture())
    .draw(mesh);
/* [Renderer-usage1] */

/* [Renderer-usage2] */
/* Initialize the renderer and reserve memory for enough glyphs */
Text::Renderer2D renderer{*font, cache, 12.0f, Text::Alignment::LineCenter};
renderer.reserve(32, GL::BufferUsage::DynamicDraw, GL::BufferUsage::StaticDraw);

/* Update the text occasionally */
renderer.render("Hello World Countdown: 10");

/* Draw the text on the screen */
shader.setTransformationProjectionMatrix(projectionMatrix)
    .setColor(0xffffff_rgbf)
    .bindVectorTexture(cache.texture())
    .draw(renderer.mesh());
/* [Renderer-usage2] */
}

{
/* [Renderer-dpi-interface-size] */
Vector2 interfaceSize = Vector2{windowSize()}/dpiScaling();
/* [Renderer-dpi-interface-size] */
/* [Renderer-dpi-size-multiplier] */
Float sizeMultiplier =
    (Vector2{framebufferSize()}*dpiScaling()/Vector2{windowSize()}).max();
/* [Renderer-dpi-size-multiplier] */
static_cast<void>(interfaceSize);
static_cast<void>(sizeMultiplier);
}

}
