#ifndef PAPERLUASOL_PAPERLUASOL_HPP
#define PAPERLUASOL_PAPERLUASOL_HPP

#include <CrunchLuaSol/CrunchLuaSol.hpp>
#include <Paper2/Document.hpp>
#include <Paper2/Group.hpp>
#include <Paper2/Path.hpp>
#include <Paper2/Symbol.hpp>
#include <Paper2/Tarp/TarpRenderer.hpp>
#include <Stick/Path.hpp>

namespace paperLuaSol
{

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

} // namespace detail

STICK_API inline void registerPaper(sol::state_view & _lua, const stick::String & _namespace = "")
{
    using namespace paper;
    using namespace stick;

    sol::table tbl = _lua.globals();
    if (!_namespace.isEmpty())
    {
        auto tokens = path::segments(_namespace, '.');
        for (const String & token : tokens)
            tbl = tbl[token.cString()] = tbl.get_or(token.cString(), _lua.create_table());
    }

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

    tbl.new_usertype<Curve>("Curve",
                            "new",
                            sol::no_constructor,
                            "path",
                            &Curve::path,
                            "setPositionOne",
                            &Curve::setPositionOne,
                            "setHandleOne",
                            &Curve::setHandleOne,
                            "setPositionTwo",
                            &Curve::setPositionTwo,
                            "setHandleTwo",
                            &Curve::setHandleTwo,
                            "positionOne",
                            &Curve::positionOne,
                            "positionTwo",
                            &Curve::positionTwo,
                            "handleOne",
                            &Curve::handleOne,
                            "handleOneAbsolute",
                            &Curve::handleOneAbsolute,
                            "handleTwo",
                            &Curve::handleTwo,
                            "handleTwoAbsolute",
                            &Curve::handleTwoAbsolute,
                            "positionAt",
                            &Curve::positionAt,
                            "normalAt",
                            &Curve::normalAt,
                            "tangentAt",
                            &Curve::tangentAt,
                            "curvatureAt",
                            &Curve::curvatureAt,
                            "angleAt",
                            &Curve::angleAt,
                            "positionAtParameter",
                            &Curve::positionAtParameter,
                            "normalAtParameter",
                            &Curve::normalAtParameter,
                            "tangentAtParameter",
                            &Curve::tangentAtParameter,
                            "curvatureAtParameter",
                            &Curve::curvatureAtParameter,
                            "angleAtParameter",
                            &Curve::angleAtParameter,
                            "parameterAtOffset",
                            &Curve::parameterAtOffset,
                            "closestParameter",
                            (Float(Curve::*)(const Vec2f &) const) & Curve::closestParameter,
                            "lengthBetween",
                            &Curve::lengthBetween,
                            "pathOffset",
                            &Curve::pathOffset,
                            "closestCurveLocation",
                            &Curve::closestCurveLocation,
                            "curveLocationAt",
                            &Curve::curveLocationAt,
                            "curveLocationAtParameter",
                            &Curve::curveLocationAtParameter,
                            "isLinear",
                            &Curve::isLinear,
                            "isStraight",
                            &Curve::isStraight,
                            "isArc",
                            &Curve::isArc,
                            "isOrthogonal",
                            &Curve::isOrthogonal,
                            "isCollinear",
                            &Curve::isCollinear,
                            "length",
                            &Curve::length,
                            "area",
                            &Curve::area,
                            "divideAt",
                            &Curve::divideAt,
                            "divideAtParameter",
                            &Curve::divideAtParameter,
                            "bounds",
                            (const Rect & (Curve::*)() const) & Curve::bounds,
                            "boundsWithPadding",
                            (Rect(Curve::*)(Float) const) & Curve::bounds,
                            "index",
                            &Curve::index);

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

    tbl.new_usertype<Item>(
        "Item",
        "new",
        sol::no_constructor,
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
        &Item::remove,
        "removeChildren",
        &Item::removeChildren,
        "name",
        &Item::name,
        "parent",
        &Item::parent,
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
        sol::overload((void (Item::*)(const ColorRGBA &)) & Item::setStroke,
                      (void (Item::*)(const String &)) & Item::setStroke,
                      (void (Item::*)(const LinearGradientPtr &)) & Item::setStroke,
                      (void (Item::*)(const RadialGradientPtr &)) & Item::setStroke),
        "removeStroke",
        &Item::removeStroke,
        "setFill",
        sol::overload((void (Item::*)(const ColorRGBA &)) & Item::setFill,
                      (void (Item::*)(const String &)) & Item::setFill,
                      (void (Item::*)(const LinearGradientPtr &)) & Item::setFill,
                      (void (Item::*)(const RadialGradientPtr &)) & Item::setFill),
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
        "hasStroke",
        &Item::hasStroke,
        "hasFill",
        &Item::hasFill,
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
        &Item::removeTransform);

    tbl.new_usertype<Group>("Group",
                            sol::base_classes,
                            sol::bases<Item>(),
                            "new",
                            sol::no_constructor,
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
        [](Path & _self, const Vec2f & _point) {
            Float outDist;
            CurveLocation loc = _self.closestCurveLocation(_point, outDist);
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
        &Path::contains,
        "segment",
        (Segment(Path::*)(stick::Size)) & Path::segment,
        "curve",
        (Curve(Path::*)(stick::Size)) & Path::curve,
        "segmentCount",
        &Path::segmentCount,
        "curveCount",
        &Path::curveCount,
        "intersections",
        sol::overload((IntersectionArray(Path::*)() const) & Path::intersections,
                      (IntersectionArray(Path::*)(const Path *) const) & Path::intersections),
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
        &Document::size);

    tbl.set_function("createLinearGradient", createLinearGradient);
    tbl.set_function("createRadialGradient", createRadialGradient);
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
                                      "setProjection",
                                      &RenderInterface::setProjection,
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

} // namespace stack

template <bool Const, class CT, class TO>
struct is_container<paper::detail::ContainerView<Const, CT, TO>> : std::false_type
{
};

} // namespace sol

#endif // PAPERLUA_PAPERLUA_HPP
