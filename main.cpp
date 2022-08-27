#include <iostream>
#include <vector>
#include <concepts>
#include <ranges>
#include <list>
#include <array>

// No need for this in C++ 17+
/*namespace TupleHelpers
{
    template <typename TTuple, typename TTransform, size_t... indices>
    decltype(auto) transform_tuple_impl(TTuple &&tuple, TTransform &&transform, std::index_sequence<indices...>)
    {
        return std::make_tuple(
            (
                std::forward<TTransform>(transform)(
                    std::get<indices>(std::forward<TTuple>(tuple))
                )
            )...
        );
    }

    template <typename TTuple, typename TTransform>
    decltype(auto) transform_tuple(TTuple &&tuple, TTransform &&transform)
    {
        return transform_tuple_impl(std::forward<TTuple>(tuple), std::forward<TTuple>(transform), std::make_index_sequence<std::tuple_size_v<TTuple>>);
    }

    template <typename TTuple>
    [[nodiscard]] decltype(auto) deref_tuple(TTuple &&tuple)
    {
        return transform_tuple(std::forward<TTuple>(tuple), [](auto&& element) -> decltype(auto) {
            return *element;
        });
    }

    template <typename TTuple>
    decltype(auto) increment_tuple(TTuple &&tuple)
    {
        return transform_tuple(std::forward<TTuple>(tuple), [](auto&& element) -> decltype(auto) {
            return ++element;
        });

    }


}*/

template <typename... TSentinel>
struct ZipIteratorSentinel
{
    ZipIteratorSentinel(TSentinel... sentinel) : sentinels(std::move(sentinel)...)
    {
    }

    std::tuple<TSentinel...> sentinels;
};

// Explicit deduction guide
// template <typename... TSentinel>
//ZipIteratorSentinel(TSentinel...) -> ZipIteratorSentinel<std::decay_t<TSentinel>...>;

template <std::forward_iterator... Iterator>
class ZipIterator
{
public:
    ZipIterator(Iterator... iterators) : m_iterators(std::move(iterators)...)
    {
    }

    decltype(auto) operator*()
    {
        return std::apply([]<typename... T>(T &&...elements)
                          { return std::make_tuple((*std::forward<T>(elements))...); },
                          m_iterators);
    }

    ZipIterator &operator++()
    {
        std::apply([]<typename... T>(T&... elements)
                   { ((++elements), ...); },
                   m_iterators);

        return *this;
    }

    template <typename... TSentinel>
    bool operator==(ZipIteratorSentinel<TSentinel...> sentinel)
    {
        auto compare = [&]<size_t... indices>(std::index_sequence<indices...>)
        {
            return ((std::get<indices>(m_iterators) == std::get<indices>(sentinel.sentinels)) || ...);
        };

        return compare(std::make_index_sequence<sizeof...(Iterator)>());
    }

private:
    std::tuple<Iterator...> m_iterators;
};

// Explicit deduction guide
// template <std::forward_iterator... Iterator>
// ZipIterator(Iterator...) -> ZipIterator<std::decay_t<Iterator>...>;

int main()
{
    auto x = std::vector{1, 2, 3, 4};
    auto y = std::list<std::string>{"a", "b", "c", "d", "e"};
    auto z = std::array{'A', 'B', 'C', 'D', 'E', 'F'};

    auto begin = ZipIterator(x.begin(), y.begin(), z.begin());
    auto end = ZipIteratorSentinel(x.end(), y.end(), z.end());

    while (begin != end)
    {
        std::apply([](auto &&...elements)
                   {
            auto print = [](auto&& element) { std::cout << std::forward<decltype(element)>(element) << " "; };
            ((print(std::forward<decltype(elements)>(elements)), 0) + ...); },
                   *begin);
        std::cout << std::endl;
        ++begin;
    }

    return 0;
}