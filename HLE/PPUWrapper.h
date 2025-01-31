#ifndef _HLE_PPU_WRAPPER_H
#define _HLE_PPU_WRAPPER

#include <cstdio>

#include <HLE/Pointer.h>

#include <Memory/Memory.h>
#include <Cell/PowerProcessor.h>

#define DEFINE_TRUE_TYPE(_struct, type) \
    template<> struct _struct<type> : public std::true_type { static constexpr bool value = true; }

namespace AwoogaPS3::Detail {

template<typename T> struct is_powerprocessor : std::false_type {};

DEFINE_TRUE_TYPE(is_powerprocessor, PowerProcessor *);

template<typename T> struct is_int32 : std::false_type {};
DEFINE_TRUE_TYPE(is_int32, int32_t);
DEFINE_TRUE_TYPE(is_int32, uint32_t);
DEFINE_TRUE_TYPE(is_int32, const uint32_t);
DEFINE_TRUE_TYPE(is_int32, const int32_t);

template<typename T> struct is_int64 : std::false_type {};
DEFINE_TRUE_TYPE(is_int64, uint64_t);
DEFINE_TRUE_TYPE(is_int64, int64_t);
DEFINE_TRUE_TYPE(is_int64, const uint64_t);
DEFINE_TRUE_TYPE(is_int64, const int64_t);

template <typename T> inline constexpr bool is_int32_v = is_int32<T>::value;
template <typename T> inline constexpr bool is_int64_v = is_int64<T>::value;
template <typename T> inline constexpr bool is_ppc_v = is_powerprocessor<T>::value;

}

#undef DEFINE_TRUE_TYPE

template<class F, F f> struct PPUWrapper;

template<class R, class ... Args, R (*f)(Args...)>
struct PPUWrapper<R (*)(Args...), f> {
private:
    using T = std::tuple<Args...>;
public:
    template<size_t Index = 0, typename ... Types>
    constexpr static void prepareArguments(PowerProcessor *proc, std::tuple<Types...>& tup) {
        uint64_t address;
        uint64_t value;

        if constexpr (Index > 8) {
            printf("Requires the stack now\n");
            std::exit(0);
        }

        address = proc->readGPR((Index - 1) + 3);
        if constexpr (Index < sizeof...(Types)) {
            constexpr auto x = std::get<Index>(std::tuple<Types...> {});
            auto& tupleData = std::get<Index>(tup);

            if constexpr (std::is_pointer<decltype(x)>::value) { // FIXME use the actual powerprocessor type
                tupleData = proc;
            } else if constexpr (AwoogaPS3::Detail::is_int64_v<decltype(x)>) {
                tupleData = address;
            } else if constexpr (AwoogaPS3::Detail::is_int32_v<decltype(x)>) {
                tupleData = (decltype(x)) address;
            } else if constexpr (std::derived_from<decltype(x), _ptr_base> == true) {
                tupleData.address = address;
                tupleData.data = (typename decltype(x)::type *) proc->getMemory()->getPtr(address);
            }
            prepareArguments<Index + 1>(proc, tup);
        }
    }

    static void wrap(PowerProcessor *proc) {
        T arguments;
        prepareArguments(proc, arguments);
        if constexpr (std::is_same<void, R>::value) { std::apply(f, arguments); }
        else if constexpr (AwoogaPS3::Detail::is_int64_v<R>) { auto val = std::apply(f, arguments); }
        else if constexpr (AwoogaPS3::Detail::is_int32_v<R>) { auto val = std::apply(f, arguments); printf("return value %d", val); }
        // TODO: write this value
    }
};

template<class F, F f>
constexpr auto wrapPPUFunction = PPUWrapper<F, f>::wrap;

#define WRAP_PPU_FUNCTION(x) wrapPPUFunction<decltype(&x), x>

#endif