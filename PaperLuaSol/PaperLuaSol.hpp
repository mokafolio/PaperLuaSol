#ifndef PAPERLUASOL_PAPERLUASOL_HPP
#define PAPERLUASOL_PAPERLUASOL_HPP

#include <CrunchLuaSol/CrunchLuaSol.hpp>

#include <Paper2/Document.hpp>
#include <Paper2/Group.hpp>
#include <Paper2/Path.hpp>
#include <Paper2/Symbol.hpp>
#include <Paper2/Tarp/TarpRenderer.hpp>

namespace paperLuaSol
{

STICK_API void registerPaper(sol::state_view _lua, const stick::String & _namespace = "");
STICK_API void registerPaper(sol::state_view _lua, sol::table _tbl);

namespace detail
{
template <class T>
struct ContainerViewPairHelper
{
    using ViewType = T;
    using Iter = typename ViewType::Iter;

    struct IterState
    {
        IterState(ViewType & _view) : it(_view.begin()), view(&_view)
        {
        }

        Iter it;
        ViewType * view;
    };

    static std::tuple<sol::object, sol::object> next(sol::user<IterState &> _iterState,
                                                     sol::this_state _lua)
    {
        IterState & iState = _iterState;
        if (iState.it == iState.view->end())
        {
            return std::make_tuple(sol::object(sol::lua_nil), sol::object(sol::lua_nil));
        }

        auto val = *(iState.it);
        return std::make_tuple(
            sol::object(_lua, sol::in_place, std::distance(iState.view->begin(), iState.it++)),
            sol::object(_lua, sol::in_place, val));
    }

    static auto pairs(ViewType & _view)
    {
        IterState iState(_view);
        return std::make_tuple(&next, sol::user<IterState>(std::move(iState)), sol::lua_nil);
    }
};

// Dynamic Property Helpers
//@TODO: This stuff should most likely be housed somewhere else. Maybe StickLuaSol?

// a simple unique default identifier to index luas registry (Callable).
struct UniquePropertyTableIdentifier
{
    void * operator()()
    {
        static int s_var;
        return (void *)&s_var;
    }
};

// if your new_index metamethod needs to do additional things, you can use this function directly
// inside your new_index implementation to set a property.
template <class T, class Identifier = UniquePropertyTableIdentifier>
void setDynamicProperty(T * _self,
                        sol::stack_object _key,
                        sol::stack_object _value,
                        sol::this_state _s,
                        Identifier _ident = UniquePropertyTableIdentifier())
{
    sol::state_view lua(_s);
    sol::table reg = lua.registry();

    // ensure that the table that maps objects to property tables exists
    sol::optional<sol::table> oir = reg[_ident];
    if (!oir)
    {
        oir = lua.create_table();
        reg[_ident] = oir.value();
    }

    // ensure that the property table for this _self exists
    sol::optional<sol::table> oit = oir.value()[(void *)_self];
    if (!oit)
    {
        oit = lua.create_table();
        oir.value()[(void *)_self] = oit.value();
    }

    oit.value()[_key] = _value;
}

// if your index metamethod needs to do additional things, you can use this function directly
// inside your index implementation to get a property.
template <class T, class Identifier = UniquePropertyTableIdentifier>
sol::object getDynamicProperty(T * _self,
                               sol::stack_object _key,
                               sol::this_state _s,
                               Identifier _ident = UniquePropertyTableIdentifier())
{
    sol::state_view lua(_s);
    sol::table reg = lua.registry();

    sol::optional<sol::table> oir = reg[_ident];
    if (!oir)
        return sol::make_object(lua, sol::lua_nil);

    sol::optional<sol::table> oit = oir.value()[(void *)_self];
    if (!oit)
        return sol::make_object(lua, sol::lua_nil);

    return sol::make_object(lua, oit.value()[_key]);
}

// if your gc metamethod needs to do additional things, or your destroy your objects in a different
// way, you can call this function directly to remove all entries for an item from the registry.
template <class T, class Identifier = UniquePropertyTableIdentifier>
void removeDynamicProperties(T * _self,
                             sol::state_view _s,
                             Identifier _ident = UniquePropertyTableIdentifier())
{
    sol::table reg = _s.registry();

    // remove the entry for _self from the registry table if needed
    sol::optional<sol::table> oir = reg[_ident];
    if (oir)
    {
        sol::optional<sol::table> oit = oir.value()[(void *)_self];
        if (oit)
        {
            oir.value()[(void *)_self] = sol::lua_nil;
        }
    }
}
// just a little helper to work around sol::constructor not working with sol::this_state
template <class T, class Identifier = UniquePropertyTableIdentifier>
void removeDynamicProperties(T * _self,
                             sol::this_state _s,
                             Identifier _ident = UniquePropertyTableIdentifier())
{
    removeDynamicProperties(_self, sol::state_view(_s), _ident);
}

// bind this to sol::meta_function::new_index
template <class T, class IdentFunctor = UniquePropertyTableIdentifier>
void dynamicPropertyNewIndex(T * _self,
                             sol::stack_object _key,
                             sol::stack_object _value,
                             sol::this_state _s)
{
    setDynamicProperty(_self, _key, _value, _s, IdentFunctor()());
}

// bind this to sol::meta_function::index
template <class T, class IdentFunctor = UniquePropertyTableIdentifier>
sol::object dynamicPropertyIndex(T * _self, sol::stack_object _key, sol::this_state _s)
{
    return getDynamicProperty(_self, _key, _s, IdentFunctor()());
}

// bind this to sol::meta_function::garbage_collect
//@NOTE: This wont work right now as sol::destructor does not work with sol::this_state :/
// template <class T, class IdentFunctor = UniquePropertyTableIdentifier>
// void dynamicPropertyGC(T * _self, sol::this_state _s)
// {
//     removeDynamicProperties(_self, _s, IdentFunctor()());
// }

} // namespace detail

#ifdef PAPERLUASOL_IMPLEMENTATION
STICK_API void registerPaper(sol::state_view _lua, const stick::String & _namespace)
{
    registerPaper(_lua, stickLuaSol::ensureNamespaceTable(_lua, _lua.globals(), _namespace));
}

template <class T>
inline void registerCurveType(sol::table _tbl, const char * _name)
{
    _tbl.new_usertype<T>(_name,
                         "new",
                         sol::no_constructor,
                         "path",
                         &T::path,
                         "setPositionOne",
                         &T::setPositionOne,
                         "setHandleOne",
                         &T::setHandleOne,
                         "setPositionTwo",
                         &T::setPositionTwo,
                         "setHandleTwo",
                         &T::setHandleTwo,
                         "positionOne",
                         &T::positionOne,
                         "positionTwo",
                         &T::positionTwo,
                         "handleOne",
                         &T::handleOne,
                         "handleOneAbsolute",
                         &T::handleOneAbsolute,
                         "handleTwo",
                         &T::handleTwo,
                         "handleTwoAbsolute",
                         &T::handleTwoAbsolute,
                         "positionAt",
                         &T::positionAt,
                         "normalAt",
                         &T::normalAt,
                         "tangentAt",
                         &T::tangentAt,
                         "curvatureAt",
                         &T::curvatureAt,
                         "angleAt",
                         &T::angleAt,
                         "positionAtParameter",
                         &T::positionAtParameter,
                         "normalAtParameter",
                         &T::normalAtParameter,
                         "tangentAtParameter",
                         &T::tangentAtParameter,
                         "curvatureAtParameter",
                         &T::curvatureAtParameter,
                         "angleAtParameter",
                         &T::angleAtParameter,
                         "parameterAtOffset",
                         &T::parameterAtOffset,
                         "closestParameter",
                         (Float(T::*)(const Vec2f &) const) & T::closestParameter,
                         "lengthBetween",
                         &T::lengthBetween,
                         "pathOffset",
                         &T::pathOffset,
                         "closestCurveLocation",
                         &T::closestCurveLocation,
                         "curveLocationAt",
                         &T::curveLocationAt,
                         "curveLocationAtParameter",
                         &T::curveLocationAtParameter,
                         "isLinear",
                         &T::isLinear,
                         "isStraight",
                         &T::isStraight,
                         "isArc",
                         &T::isArc,
                         "isOrthogonal",
                         &T::isOrthogonal,
                         "isCollinear",
                         &T::isCollinear,
                         "length",
                         &T::length,
                         "area",
                         &T::area,
                         "divideAt",
                         &T::divideAt,
                         "divideAtParameter",
                         &T::divideAtParameter,
                         "bounds",
                         (const Rect & (T::*)() const) & T::bounds,
                         "boundsWithPadding",
                         (Rect(T::*)(Float) const) & T::bounds,
                         "index",
                         &T::index);
}

STICK_API void registerPaper(sol::state_view _lua, sol::table _tbl)
{
    using namespace paper;
    using namespace stick;

    sol::table tbl = _tbl;
    tbl.new_enum("StrokeJoin", //
                 "Miter",
                 StrokeJoin::Miter, //
                 "Round",
                 StrokeJoin::Round, //
                 "Bevel",
                 StrokeJoin::Bevel);

    tbl.new_enum("StrokeCap", //
                 "Round",
                 StrokeCap::Round, //
                 "Square",
                 StrokeCap::Square, //
                 "Butt",
                 StrokeCap::Butt);

    tbl.new_enum("WindingRule", //
                 "EvenOdd",
                 WindingRule::EvenOdd, //
                 "NonZero",
                 WindingRule::NonZero);

    tbl.new_enum("Smoothing", //
                 "Continuous",
                 Smoothing::Continuous, //
                 "Asymmetric",
                 Smoothing::Asymmetric, //
                 "CatmullRom",
                 Smoothing::CatmullRom, //
                 "Geometric",
                 Smoothing::Geometric);

    tbl.new_enum("ItemType", //
                 "Document",
                 ItemType::Document, //
                 "Group",
                 ItemType::Group, //
                 "Path",
                 ItemType::Path, //
                 "Symbol",
                 ItemType::Symbol, //
                 "Unknown",
                 ItemType::Unknown);

    tbl.new_enum("HitTestMode", //
                 "Fill",
                 HitTestFill, //
                 "Curves",
                 HitTestCurves);

    tbl.new_usertype<Segment>("Segment",
                              "new",
                              sol::no_constructor,
                              "setPosition",
                              &Segment::setPosition,
                              "setHandleIn",
                              &Segment::setHandleIn,
                              "setHandleOut",
                              &Segment::setHandleOut,
                              "position",
                              &Segment::position,
                              "positionAbsolute",
                              &Segment::positionAbsolute,
                              "handleIn",
                              &Segment::handleIn,
                              "handleOut",
                              &Segment::handleOut,
                              "handleInAbsolute",
                              &Segment::handleInAbsolute,
                              "handleOutAbsolute",
                              &Segment::handleOutAbsolute,
                              "isLinear",
                              &Segment::isLinear,
                              "remove",
                              &Segment::remove,
                              "index",
                              &Segment::index);

    tbl.new_usertype<CurveLocation>("CurveLocation",
                                    "new",
                                    sol::no_constructor,
                                    "position",
                                    &CurveLocation::position,
                                    "positionAbsolute",
                                    &CurveLocation::positionAbsolute,
                                    "normal",
                                    &CurveLocation::normal,
                                    "tangent",
                                    &CurveLocation::tangent,
                                    "curvature",
                                    &CurveLocation::curvature,
                                    "angle",
                                    &CurveLocation::angle,
                                    "parameter",
                                    &CurveLocation::parameter,
                                    "offset",
                                    &CurveLocation::offset,
                                    "isValid",
                                    &CurveLocation::isValid,
                                    "curve",
                                    &CurveLocation::curve);

    registerCurveType<Curve>(tbl, "Curve");
    registerCurveType<ConstCurve>(tbl, "ConstCurve");

    // tbl.new_usertype<Curve>("Curve",
    // "new",
    // sol::no_constructor,
    // "path",
    // &Curve::path,
    // "setPositionOne",
    // &Curve::setPositionOne,
    // "setHandleOne",
    // &Curve::setHandleOne,
    // "setPositionTwo",
    // &Curve::setPositionTwo,
    // "setHandleTwo",
    // &Curve::setHandleTwo,
    // "positionOne",
    // &Curve::positionOne,
    // "positionTwo",
    // &Curve::positionTwo,
    // "handleOne",
    // &Curve::handleOne,
    // "handleOneAbsolute",
    // &Curve::handleOneAbsolute,
    // "handleTwo",
    // &Curve::handleTwo,
    // "handleTwoAbsolute",
    // &Curve::handleTwoAbsolute,
    // "positionAt",
    // &Curve::positionAt,
    // "normalAt",
    // &Curve::normalAt,
    // "tangentAt",
    // &Curve::tangentAt,
    // "curvatureAt",
    // &Curve::curvatureAt,
    // "angleAt",
    // &Curve::angleAt,
    // "positionAtParameter",
    // &Curve::positionAtParameter,
    // "normalAtParameter",
    // &Curve::normalAtParameter,
    // "tangentAtParameter",
    // &Curve::tangentAtParameter,
    // "curvatureAtParameter",
    // &Curve::curvatureAtParameter,
    // "angleAtParameter",
    // &Curve::angleAtParameter,
    // "parameterAtOffset",
    // &Curve::parameterAtOffset,
    // "closestParameter",
    // (Float(Curve::*)(const Vec2f &) const) & Curve::closestParameter,
    // "lengthBetween",
    // &Curve::lengthBetween,
    // "pathOffset",
    // &Curve::pathOffset,
    // "closestCurveLocation",
    // &Curve::closestCurveLocation,
    // "curveLocationAt",
    // &Curve::curveLocationAt,
    // "curveLocationAtParameter",
    // &Curve::curveLocationAtParameter,
    // "isLinear",
    // &Curve::isLinear,
    // "isStraight",
    // &Curve::isStraight,
    // "isArc",
    // &Curve::isArc,
    // "isOrthogonal",
    // &Curve::isOrthogonal,
    // "isCollinear",
    // &Curve::isCollinear,
    // "length",
    // &Curve::length,
    // "area",
    // &Curve::area,
    // "divideAt",
    // &Curve::divideAt,
    // "divideAtParameter",
    // &Curve::divideAtParameter,
    // "bounds",
    // (const Rect & (Curve::*)() const) & Curve::bounds,
    // "boundsWithPadding",
    // (Rect(Curve::*)(Float) const) & Curve::bounds,
    // "index",
    // &Curve::index);

    tbl.new_usertype<NoPaint>("NoPaint", sol::call_constructor, sol::constructors<NoPaint()>());

    tbl.new_usertype<BaseGradient>("BaseGradient",
                                   "new",
                                   sol::no_constructor,
                                   "setOrigin",
                                   &BaseGradient::setOrigin,
                                   "setDestination",
                                   &BaseGradient::setDestination,
                                   "addStop",
                                   &BaseGradient::addStop,
                                   "origin",
                                   &BaseGradient::origin,
                                   "destination",
                                   &BaseGradient::destination,
                                   "stops",
                                   &BaseGradient::stops);

    tbl.new_usertype<LinearGradient>("LinearGradient",
                                     sol::base_classes,
                                     sol::bases<BaseGradient>(),
                                     "new",
                                     sol::no_constructor);

    tbl.new_usertype<RadialGradient>("RadialGradient",
                                     sol::base_classes,
                                     sol::bases<BaseGradient>(),
                                     "new",
                                     sol::no_constructor,
                                     "setFocalPointOffset",
                                     &RadialGradient::setFocalPointOffset,
                                     "setRatio",
                                     &RadialGradient::setRatio,
                                     "focalPointOffset",
                                     &RadialGradient::focalPointOffset,
                                     "ratio",
                                     &RadialGradient::ratio);

    // void setStrokeJoin(StrokeJoin _join);
    // void setStrokeCap(StrokeCap _cap);
    // void setMiterLimit(Float _limit);
    // void setStrokeWidth(Float _width);
    // void setStroke(const stick::String & _svgName);
    // void setStroke(const Paint & _paint);
    // void setDashArray(const DashArray & _arr);
    // void setDashOffset(Float _f);
    // void setScaleStroke(bool _b);
    // void setFill(const stick::String & _svgName);
    // void setFill(const Paint & _paint);
    // void setWindingRule(WindingRule _rule);

    // StrokeJoin strokeJoin() const;
    // StrokeCap strokeCap() const;
    // Float miterLimit() const;
    // Float strokeWidth() const;
    // const DashArray & dashArray() const;
    // Float dashOffset() const;
    // WindingRule windingRule() const;
    // bool scaleStroke() const;
    // Paint fill() const;
    // Paint stroke() const;
    // const StyleData & data() const;

    // StylePtr clone(Item * _item = nullptr) const;

    tbl.new_usertype<Style>(
        "Style",
        "new",
        sol::no_constructor,
        "setStrokeJoin",
        &Style::setStrokeJoin,
        "setStrokeCap",
        &Style::setStrokeCap,
        "setMiterLimit",
        &Style::setMiterLimit,
        "setStrokeWidth",
        &Style::setStrokeWidth,
        "setStroke",
        sol::overload((void (Style::*)(const String &)) & Style::setStroke,
                      (void (Style::*)(const Paint &)) & Style::setStroke),
        "setDashArray",
        [](Style * _self, sol::table _tbl) {
            Size s = _tbl.size();
            stick::DynamicArray<Float> dashes(s);
            for (Size i = 1; i <= _tbl.size(); ++i)
            {
                dashes[i - 1] = _tbl[i];
            }
            _self->setDashArray(dashes);
        },
        "setDashOffset",
        &Style::setDashOffset,
        "setScaleStroke",
        &Style::setScaleStroke,
        "setFill",
        sol::overload((void (Style::*)(const String &)) & Style::setFill,
                      (void (Style::*)(const Paint &)) & Style::setFill),
        "setWindingRule",
        &Style::setWindingRule,
        "strokeJoin",
        &Style::strokeJoin,
        "strokeCap",
        &Style::strokeCap,
        "miterLimit",
        &Style::miterLimit,
        "strokeWidth",
        &Style::strokeWidth,
        "dashArray",
        &Style::dashArray,
        "dashOffset",
        &Style::dashOffset,
        "windingRule",
        &Style::windingRule,
        "scaleStroke",
        &Style::scaleStroke,
        "fill",
        &Style::fill,
        "stroke",
        &Style::stroke);

    //@TODO: Bind StyleData or make a table converter for easily setting a style from a table

    // tbl.new_usertype<ResolvedStyle>("ResolvedStyle",
    //                                 "new",
    //                                 sol::no_constructor,
    //                                 "fill",
    //                                 &ResolvedStyle::fill,
    //                                 "stroke",
    //                                 &ResolvedStyle::stroke,
    //                                 "strokeWidth",
    //                                 &ResolvedStyle::strokeWidth,
    //                                 "strokeJoin",
    //                                 &ResolvedStyle::strokeJoin,
    //                                 "strokeCap",
    //                                 &ResolvedStyle::strokeCap,
    //                                 "scaleStroke",
    //                                 &ResolvedStyle::scaleStroke,
    //                                 "miterLimit",
    //                                 &ResolvedStyle::miterLimit,
    //                                 "dashArray",
    //                                 &ResolvedStyle::dashArray,
    //                                 "dashOffset",
    //                                 &ResolvedStyle::dashOffset,
    //                                 "windingRule",
    //                                 &ResolvedStyle::windingRule);

    tbl.new_usertype<Item>(
        "Item",
        "new",
        sol::no_constructor,
        sol::meta_function::new_index,
        detail::dynamicPropertyNewIndex<Item>,
        sol::meta_function::index,
        detail::dynamicPropertyIndex<Item>,
        "addChild",
        &Item::addChild,
        "insertAbove",
        &Item::insertAbove,
        "insertBelow",
        &Item::insertBelow,
        "sendToFront",
        &Item::sendToFront,
        "sendToBack",
        &Item::sendToBack,
        "reverseChildren",
        &Item::reverseChildren,
        "remove",
        [](Item * _self, sol::this_state _s) {
            detail::removeDynamicProperties(_self, _s);
            _self->remove();
        },
        "removeFromParent",
        &Item::removeFromParent,
        "removeChildren",
        &Item::removeChildren,
        "findChild",
        &Item::findChild,
        "child",
        &Item::child,
        "name",
        &Item::name,
        "parent",
        &Item::parent,
        "nextSibling",
        &Item::nextSibling,
        "previousSibling",
        &Item::previousSibling,
        "isAncestor",
        &Item::isAncestor,
        "isDescendant",
        &Item::isDescendant,
        "setPosition",
        &Item::setPosition,
        "setPivot",
        &Item::setPivot,
        "removePivot",
        &Item::removePivot,
        "setVisible",
        &Item::setVisible,
        "setName",
        &Item::setName,
        "setTransform",
        &Item::setTransform,
        "translateTransform",
        (void (Item::*)(const Vec2f &)) & Item::translateTransform,
        "scaleTransform",
        sol::overload((void (Item::*)(Float)) & Item::scaleTransform,
                      (void (Item::*)(const Vec2f &)) & Item::scaleTransform,
                      (void (Item::*)(const Vec2f &, const Vec2f &)) & Item::scaleTransform),
        "rotateTransform",
        sol::overload((void (Item::*)(Float)) & Item::rotateTransform,
                      (void (Item::*)(Float, const Vec2f &)) & Item::rotateTransform),
        "skewTransform",
        sol::overload((void (Item::*)(const Vec2f &)) & Item::skewTransform,
                      (void (Item::*)(const Vec2f &, const Vec2f &)) & Item::skewTransform),
        "transformItem",
        (void (Item::*)(const Mat32f &)) & Item::transform,
        "translate",
        (void (Item::*)(const Vec2f &)) & Item::translate,
        "scale",
        sol::overload((void (Item::*)(Float)) & Item::scale,
                      (void (Item::*)(const Vec2f &)) & Item::scale,
                      (void (Item::*)(const Vec2f &, const Vec2f &)) & Item::scale),
        "rotate",
        sol::overload((void (Item::*)(Float)) & Item::rotate,
                      (void (Item::*)(Float, const Vec2f &)) & Item::rotate),
        "skew",
        sol::overload((void (Item::*)(const Vec2f &)) & Item::skew,
                      (void (Item::*)(const Vec2f &, const Vec2f &)) & Item::skew),
        "applyTransform",
        &Item::applyTransform,
        "transform",
        (const Mat32f & (Item::*)() const) & Item::transform,
        "rotation",
        &Item::rotation,
        "translation",
        &Item::translation,
        "scaling",
        &Item::scaling,
        "absoluteRotation",
        &Item::absoluteRotation,
        "absoluteTranslation",
        &Item::absoluteTranslation,
        "absoluteScaling",
        &Item::absoluteScaling,
        "bounds",
        &Item::bounds,
        "handleBounds",
        &Item::handleBounds,
        "strokeBounds",
        &Item::strokeBounds,
        "position",
        &Item::position,
        "pivot",
        &Item::pivot,
        "isVisible",
        &Item::isVisible,
        "setStrokeJoin",
        &Item::setStrokeJoin,
        "setStrokeCap",
        &Item::setStrokeCap,
        "setMiterLimit",
        &Item::setMiterLimit,
        "setStrokeWidth",
        &Item::setStrokeWidth,
        "setDashArray",
        [](Item * _self, sol::table _tbl) {
            Size s = _tbl.size();
            stick::DynamicArray<Float> dashes(s);
            for (Size i = 1; i <= _tbl.size(); ++i)
            {
                dashes[i - 1] = _tbl[i];
            }
            _self->setDashArray(dashes);
        },
        "setDashOffset",
        &Item::setDashOffset,
        "setScaleStroke",
        &Item::setScaleStroke,
        "setStroke",
        sol::overload((void (Item::*)(const String &)) & Item::setStroke,
                      (void (Item::*)(const Paint &)) & Item::setStroke),
        "removeStroke",
        &Item::removeStroke,
        "setFill",
        sol::overload((void (Item::*)(const String &)) & Item::setFill,
                      (void (Item::*)(const Paint &)) & Item::setFill),
        "removeFill",
        &Item::removeFill,
        "setWindingRule",
        &Item::setWindingRule,
        "strokeJoin",
        &Item::strokeJoin,
        "strokeCap",
        &Item::strokeCap,
        "miterLimit",
        &Item::miterLimit,
        "strokeWidth",
        &Item::strokeWidth,
        "dashArray",
        &Item::dashArray,
        "dashOffset",
        &Item::dashOffset,
        "windingRule",
        &Item::windingRule,
        "scaleStroke",
        &Item::scaleStroke,
        "fill",
        &Item::fill,
        "stroke",
        &Item::stroke,
        "clone",
        &Item::clone,
        "document",
        &Item::document,
        "itemType",
        &Item::itemType,
        "children",
        &Item::children,
        "exportSVG",
        &Item::exportSVG,
        "exportBinary",
        [](const Item * _self, sol::this_state _s) {
            sol::state_view lua(_s);
            auto res = _self->exportBinary();
            if (res)
                return sol::make_object<const char *>(
                    lua, reinterpret_cast<const char *>(&res.get()[0]), res.get().count());
            return sol::make_object(lua, res.error());
        },
        "saveSVG",
        &Item::saveSVG,
        "setFillPaintTransform",
        &Item::setFillPaintTransform,
        "removeFillPaintTransform",
        &Item::removeFillPaintTransform,
        "setStrokePaintTransform",
        &Item::setStrokePaintTransform,
        "removeStrokePaintTransform",
        &Item::removeStrokePaintTransform,
        "removeTransform",
        &Item::removeTransform,
        "hitTest",
        sol::overload(&Item::hitTest,
                      [](Item * _self, const Vec2f & _pos) { return _self->hitTest(_pos); }),
        "hitTestAll",
        sol::overload(&Item::hitTestAll,
                      [](Item * _self, const Vec2f & _pos) { return _self->hitTestAll(_pos); }),
        "selectChildren",
        &Item::selectChildren,
        "hasTransform",
        &Item::hasTransform,
        "isTransformed",
        &Item::isTransformed,
        "canAddChild",
        &Item::canAddChild,
        "style",
        &Item::style,
        "stylePtr",
        &Item::stylePtr);

    tbl.new_usertype<Group>("Group",
                            sol::base_classes,
                            sol::bases<Item>(),
                            "new",
                            sol::no_constructor,
                            sol::meta_function::new_index,
                            detail::dynamicPropertyNewIndex<Group>,
                            sol::meta_function::index,
                            detail::dynamicPropertyIndex<Group>,
                            "setClipped",
                            &Group::setClipped,
                            "isClipped",
                            &Group::isClipped);

    tbl.new_usertype<SegmentView>("__SegmentView",
                                  sol::meta_function::pairs,
                                  &detail::ContainerViewPairHelper<SegmentView>::pairs,
                                  sol::meta_function::ipairs,
                                  &detail::ContainerViewPairHelper<SegmentView>::pairs);

    tbl.new_usertype<CurveView>("__CurveView",
                                sol::meta_function::pairs,
                                &detail::ContainerViewPairHelper<CurveView>::pairs,
                                sol::meta_function::ipairs,
                                &detail::ContainerViewPairHelper<CurveView>::pairs);

    tbl.new_usertype<Path>(
        "Path",
        sol::base_classes,
        sol::bases<Item>(),
        "new",
        sol::no_constructor,
        sol::meta_function::new_index,
        detail::dynamicPropertyNewIndex<Path>,
        sol::meta_function::index,
        detail::dynamicPropertyIndex<Path>,
        "addPoint",
        &Path::addPoint,
        "cubicCurveTo",
        &Path::cubicCurveTo,
        "quadraticCurveTo",
        &Path::quadraticCurveTo,
        "curveTo",
        &Path::curveTo,
        "arcThrough",
        (Error(Path::*)(const Vec2f &, const Vec2f &)) & Path::arcTo,
        "arcTo",
        sol::overload((Error(Path::*)(const Vec2f &, bool)) & Path::arcTo,
                      (Error(Path::*)(const Vec2f &, const Vec2f &)) & Path::arcTo),
        "cubicCurveBy",
        &Path::cubicCurveBy,
        "quadraticCurveBy",
        &Path::quadraticCurveBy,
        "curveBy",
        &Path::curveBy,
        "closePath",
        &Path::closePath,
        "makeEllipse",
        &Path::makeEllipse,
        "makeCircle",
        &Path::makeCircle,
        "makeRectangle",
        &Path::makeRectangle,
        "makeRoundedRectangle",
        &Path::makeRoundedRectangle,
        "smooth",
        sol::overload([](Path & _self) { _self.smooth(); },
                      (void (Path::*)(Smoothing, bool)) & Path::smooth,
                      (void (Path::*)(Int64, Int64, Smoothing)) & Path::smooth),
        "simplify",
        &Path::simplify,
        "addSegment",
        &Path::addSegment,
        "removeSegment",
        &Path::removeSegment,
        "removeSegments",
        sol::overload((void (Path::*)(Size)) & Path::removeSegments,
                      (void (Path::*)(Size, Size)) & Path::removeSegments,
                      (void (Path::*)()) & Path::removeSegments),
        "positionAt",
        &Path::positionAt,
        "normalAt",
        &Path::normalAt,
        "tangentAt",
        &Path::tangentAt,
        "curvatureAt",
        &Path::curvatureAt,
        "angleAt",
        &Path::angleAt,
        "reverse",
        &Path::reverse,
        "setClockwise",
        &Path::setClockwise,
        "flatten",
        &Path::flatten,
        "flattenRegular",
        &Path::flattenRegular,
        "regularOffset",
        &Path::regularOffset,
        "closestCurveLocation",
        sol::overload(
            [](Path & _self, const Vec2f & _point) {
                Float outDist;
                CurveLocation loc = _self.closestCurveLocation(_point, outDist);
                return std::make_tuple(loc, outDist);
            },
            [](Path & _self, const Mat32f & _transform, const Vec2f & _point) {
                Float outDist;
                CurveLocation loc = _self.closestCurveLocation(_point, _transform, outDist);
                return std::make_tuple(loc, outDist);
            }),
        "closestCurveLocationLocal",
        [](Path & _self, const Vec2f & _point) {
            Float outDist;
            CurveLocation loc = _self.closestCurveLocationLocal(_point, outDist);
            return std::make_tuple(loc, outDist);
        },
        "curveLocationAt",
        &Path::curveLocationAt,
        "length",
        &Path::length,
        "area",
        &Path::area,
        "isClosed",
        &Path::isClosed,
        "isClockwise",
        &Path::isClockwise,
        "contains",
        sol::overload((bool (Path::*)(const Vec2f &) const) & Path::contains,
                      (bool (Path::*)(const Vec2f &, const Mat32f &) const) & Path::contains),
        "segment",
        (Segment(Path::*)(stick::Size)) & Path::segment,
        "curve",
        (Curve(Path::*)(stick::Size)) & Path::curve,
        "segmentCount",
        &Path::segmentCount,
        "curveCount",
        &Path::curveCount,
        "intersections",
        sol::overload(
            (IntersectionArray(Path::*)() const) & Path::intersections,
            (IntersectionArray(Path::*)(const Mat32f &) const) & Path::intersections,
            (IntersectionArray(Path::*)(const Path *) const) & Path::intersections,
            (IntersectionArray(Path::*)(const Path *, const Mat32f &) const) & Path::intersections,
            (IntersectionArray(Path::*)(const Path *, const Mat32f &, const Mat32f &) const) &
                Path::intersections),
        "intersectionsLocal",
        &Path::intersectionsLocal,
        "extrema",
        [](Path * _self) {
            stick::DynamicArray<CurveLocation> ret;
            _self->extrema(ret);
            return ret;
        },
        "slice",
        sol::overload((Path * (Path::*)(CurveLocation, CurveLocation) const) & Path::slice,
                      (Path * (Path::*)(Float, Float) const) & Path::slice),
        "segments",
        (SegmentView(Path::*)()) & Path::segments,
        "curves",
        (CurveView(Path::*)()) & Path::curves);

    tbl.new_usertype<Document>(
        "Document",
        sol::base_classes,
        sol::bases<Item>(),
        sol::call_constructor,
        sol::constructors<Document(), Document(const char *)>(),
        sol::meta_function::new_index,
        detail::dynamicPropertyNewIndex<Document>,
        sol::meta_function::index,
        detail::dynamicPropertyIndex<Document>,
        "createGroup",
        sol::overload([](Document & _self) { return _self.createGroup(); }, &Document::createGroup),
        "createPath",
        sol::overload([](Document & _self) { return _self.createPath(); }, &Document::createPath),
        "createEllipse",
        sol::overload([](Document & _self,
                         const Vec2f & _pos,
                         const Vec2f & _size) { return _self.createEllipse(_pos, _size); },
                      &Document::createEllipse),
        "createCircle",
        sol::overload([](Document & _self,
                         const Vec2f & _pos,
                         Float _rad) { return _self.createCircle(_pos, _rad); },
                      &Document::createCircle),
        "createRectangle",
        sol::overload([](Document & _self,
                         const Vec2f & _min,
                         const Vec2f & _max) { return _self.createRectangle(_min, _max); },
                      &Document::createRectangle),
        "createRoundedRectangle",
        sol::overload(
            [](Document & _self, const Vec2f & _min, const Vec2f & _max, const Vec2f & _rad) {
                return _self.createRoundedRectangle(_min, _max, _rad);
            },
            &Document::createRoundedRectangle),
        "setSize",
        &Document::setSize,
        "width",
        &Document::width,
        "height",
        &Document::height,
        "size",
        &Document::size,
        "loadSVG",
        sol::overload([](Document & _self, const char * _path) { return _self.loadSVG(_path); },
                      &Document::loadSVG),
        "parseSVG",
        sol::overload([](Document & _self, const char * _svg) { return _self.parseSVG(_svg); },
                      &Document::parseSVG),
        "parseBinary",
        [](Document & _self, stick::String _binaryStr) {
            return _self.parseBinary(reinterpret_cast<const UInt8 *>(_binaryStr.cString()),
                                     _binaryStr.length());
        },
        "createLinearGradient",
        &Document::createLinearGradient,
        "createRadialGradient",
        &Document::createRadialGradient,
        "createStyle",
        &Document::createStyle);

    tbl.set_function("upCastItem", [](Item * _item, sol::this_state _s) {
        sol::state_view lua(_s);
        if (_item->itemType() == ItemType::Path)
        {
            return sol::make_object(lua, static_cast<paper::Path *>(_item));
        }
        else if (_item->itemType() == ItemType::Group)
        {
            return sol::make_object(lua, static_cast<paper::Group *>(_item));
        }
        else if (_item->itemType() == ItemType::Symbol)
        {
            return sol::make_object(lua, static_cast<paper::Symbol *>(_item));
        }
        return sol::make_object(lua, _item);
        ;
    });

    tbl.new_usertype<RenderInterface>("RenderInterface",
                                      "new",
                                      sol::no_constructor,
                                      "setViewport",
                                      &RenderInterface::setViewport,
                                      "setDefaultProjection",
                                      &RenderInterface::setDefaultProjection,
                                      "setProjection",
                                      &RenderInterface::setProjection,
                                      "setTransform",
                                      &RenderInterface::setTransform,
                                      "document",
                                      &RenderInterface::document,
                                      "draw",
                                      &RenderInterface::draw);

    tbl.new_usertype<paper::tarp::TarpRenderer>(
        "TarpRenderer",
        sol::base_classes,
        sol::bases<RenderInterface>(),
        sol::call_constructor,
        sol::factories([](Document & _doc) {
            auto sp = stick::makeShared<paper::tarp::TarpRenderer>();
            // = std::move(paper::tarp::TarpRenderer());
            auto err = sp->init(_doc);
            if (err)
                return stick::SharedPtr<paper::tarp::TarpRenderer>();
            return std::move(sp);
        }));
}
#endif // PAPERLUASOL_IMPLEMENTATION

} // namespace paperLuaSol

namespace sol
{
namespace stack
{

template <>
struct pusher<paper::ColorStop>
{
    static int push(lua_State * L, const paper::ColorStop & _stop)
    {
        sol::table tbl(L, sol::new_table(0, 2));
        tbl["color"] = _stop.color;
        tbl["offset"] = _stop.offset;
        sol::stack::push(L, tbl);
        return 1;
    }
};

template <>
struct pusher<paper::Intersection>
{
    static int push(lua_State * L, const paper::Intersection & _stop)
    {
        sol::table tbl(L, sol::new_table(0, 2));
        tbl["location"] = _stop.location;
        tbl["position"] = _stop.position;
        sol::stack::push(L, tbl);
        return 1;
    }
};

template <>
struct pusher<paper::HitTestResult>
{
    static int push(lua_State * L, const paper::HitTestResult & _res)
    {
        sol::table tbl(L, sol::new_table(0, 2));
        tbl["item"] = _res.item;
        tbl["type"] = _res.type;
        sol::stack::push(L, tbl);
        return 1;
    }
};

// template <>
// struct checker<paper::HitTestSettings>
// {
//     static int push(lua_State * L, const paper::HitTestResult & _res)
//     {
//         sol::table tbl(L, sol::new_table(0, 2));
//         tbl["item"] = _res.item;
//         tbl["type"] = _stop.type;
//         sol::stack::push(L, tbl);
//         return 1;
//     }
// };

template <>
struct pusher<paper::svg::SVGImportResult>
{
    static int push(lua_State * L, const paper::svg::SVGImportResult & _result)
    {
        if (_result)
            sol::stack::push(L, _result.group());
        else
            sol::stack::push(L, _result.error());
        return 1;
    }
};

} // namespace stack

template <bool Const, class CT, class TO>
struct is_container<paper::detail::ContainerView<Const, CT, TO>> : std::false_type
{
};

} // namespace sol

#endif // PAPERLUA_PAPERLUA_HPP
