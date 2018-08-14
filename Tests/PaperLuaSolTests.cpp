#include <Stick/Test.hpp>
#include <PaperLuaSol/PaperLuaSol.hpp>
// #include <CrunchLua/CrunchLua.hpp>

using namespace stick;
using namespace paperLua;

const Suite spec[] =
{
    SUITE("Namespacing Tests")
    {
    }
    // SUITE("Namespacing Tests")
    // {
    //     lua_State * state = createLuaState();
    //     {
    //         openStandardLibraries(state);
    //         initialize(state);
    //         crunchLua::registerCrunch(state);
    //         registerPaper(state, "paper");

    //         EXPECT(lua_gettop(state) == 0);

    //         //very basic test to see if the namespacing works
    //         String basicTest =
    //         "print(type(paper.Document))\n"
    //         "local doc = paper.Document(\"Doc\")\n"
    //         "assert(doc:name() == \"Doc\")\n"
    //         "local path = doc:createPath()\n"
    //         "path:addPoint({100, 100})\n"
    //         "path:addPoint({200, 300})\n"
    //         "path:addPoint({300, 500})\n"
    //         "assert(path:segmentCount() == 3)\n"
    //         "assert(path:segment(0):position() == Vec2(100, 100))\n"
    //         "assert(path:segment(1):position() == Vec2(200, 300))\n"
    //         "assert(path:segment(2):position() == Vec2(300, 500))\n"
    //         "for seg in path:segments() do print('SEG POSISISI:', seg:position()) end\n"
    //         "for i=0,path:segmentCount() - 1,1 do print('SEG POS', path:segment(i):position()) end\n"
    //         "local grad = paper.createLinearGradient({0, 0}, {100, 200})\n"
    //         "grad:addStop(ColorRGBA(1.0, 0.5, 0.3, 1.0), 0.0)\n"
    //         "grad:addStop(ColorRGBA(1.0, 0.5, 1.0, 1.0), 1.0)\n"
    //         "assert(grad:origin() == Vec2(0, 0))\n"
    //         "assert(grad:destination() == Vec2(100, 200))\n"
    //         "for stop in grad:stops() do print(stop.color.r, stop.color.g, stop.color.b, stop.color.a, stop.offset) end\n"
    //         "path:setFillLinearGradient(grad)\n"
    //         "print(doc:exportSVG())\n";

    //         auto err = luanatic::execute(state, basicTest);
    //         if (err)
    //             printf("%s\n", err.message().cString());
    //         EXPECT(!err);

    //         EXPECT(lua_gettop(state) == 0);
    //     }
    //     lua_close(state);
    // },
    // SUITE("Path Tests")
    // {
    //     // lua_State * state = createLuaState();
    //     // {
    //     //     openStandardLibraries(state);
    //     //     initialize(state);
    //     //     registerPaper(state);
    //     //     crunchLua::registerCrunch(state);

    //     //     EXPECT(lua_gettop(state) == 0);

    //     //     String basicTest =
    //     //         "local doc = Document(\"Doc\")\n"
    //     //         "assert(doc:name() == \"Doc\")\n"
    //     //         "local renderer = GLRenderer.new(doc)\n"
    //     //         "renderer:setViewport(100, 100)\n"
    //     //         "local p = doc:createCircle(Vec2.fromNumbers(100, 100), 10)\n"
    //     //         "local val = p:normalAt(p:length() * 0.5)\n"
    //     //         "p:setFill(ColorRGBA.fromNumbers(1.0, 0.0, 0.0, 1.0))\n";

    //     //     auto err = luanatic::execute(state, basicTest);
    //     //     if (err)
    //     //         printf("%s\n", err.message().cString());

    //     //     EXPECT(lua_gettop(state) == 0);

    //     //     EXPECT(!err);
    //     // }
    //     // lua_close(state);
    // }
};

int main(int _argc, const char * _args[])
{
    return runTests(spec, _argc, _args);
}
