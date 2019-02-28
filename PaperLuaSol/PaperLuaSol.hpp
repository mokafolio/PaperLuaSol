#ifndef PAPERLUASOL_PAPERLUASOL_HPP
#define PAPERLUASOL_PAPERLUASOL_HPP

#include <Paper2/Path.hpp>
#include <Paper2/SVG/SVGImportResult.hpp>

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

STICK_API void registerPaper(sol::state_view _lua, const stick::String & _namespace = "");
STICK_API void registerPaper(sol::state_view _lua, sol::table _tbl);

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
struct pusher<paper::svg::SVGImportResult>
{
    static int push(lua_State * L, const paper::svg::SVGImportResult & _result)
    {
        if(_result)
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
