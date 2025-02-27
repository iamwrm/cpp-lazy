#pragma once

#ifndef LZ_CARTESIAN_PRODUCT_HPP
#define LZ_CARTESIAN_PRODUCT_HPP

#include "detail/BasicIteratorView.hpp"
#include "detail/CartesianProductIterator.hpp"

namespace lz {
template<class... Iterators>
class CartesianProduct final : public internal::BasicIteratorView<internal::CartesianProductIterator<Iterators...>> {
public:
    using iterator = internal::CartesianProductIterator<Iterators...>;
    using const_iterator = iterator;

    constexpr CartesianProduct() = default;

    LZ_CONSTEXPR_CXX_20 CartesianProduct(std::tuple<Iterators...> begin, std::tuple<Iterators...> end) :
        internal::BasicIteratorView<iterator>(iterator(begin, begin, end), iterator(end, begin, end)) {
    }
};

/**
 * @addtogroup ItFns
 * @{
 */

/**
 * Creates an iterator view object that, when iterated over, gets all possible combinations of all its values of the iterators.
 * @attention Please note that this is not an actual random access iterator. It uses the 'strongest' operator to increase/decrease
 * the iterators with. If the current iterator is bidirectional, it uses ++/--. If it is random access, it uses +/-. So if all
 * the iterators passed are random access, then this iterator is also true random access. If one of the iterators is bidirectional
 * then that iterator is incremented/decremented using the ++ and -- operators.
 * @param begin The tuple containing all the beginnings of the sequences.
 * @param end The ending containing all the endings of the sequences.
 * @return A cartesian product view object.
 */
template<LZ_CONCEPT_ITERATOR... Iterators>
LZ_NODISCARD LZ_CONSTEXPR_CXX_20 CartesianProduct<Iterators...>
cartesianRange(std::tuple<Iterators...> begin, std::tuple<Iterators...> end) {
    return { std::move(begin), std::move(end) };
}

/**
 * Creates an iterator view object that, when iterated over, gets all possible combinations of all its values of the iterables.
 * @attention Please note that this is not an actual random access iterator. It uses the 'strongest' operator to increase/decrease
 * the iterators with. If the current iterator is bidirectional, it uses ++/--. If it is random access, it uses +/-. So if all
 * the iterators passed are random access, then this iterator is also true random access. If one of the iterators is bidirectional
 * then that iterator is incremented/decremented using the ++ and -- operators, and in that case this iterator wouldn't be true
 * random access.
 * @param iterables The iterables to make all of the possible combinations with.
 * @return A cartesian product view object.
 */
template<LZ_CONCEPT_ITERABLE... Iterables>
LZ_NODISCARD LZ_CONSTEXPR_CXX_20 CartesianProduct<internal::IterTypeFromIterable<Iterables>...>
cartesian(Iterables&&... iterables) {
    return cartesianRange(std::make_tuple(internal::begin(std::forward<Iterables>(iterables))...),
                          std::make_tuple(internal::end(std::forward<Iterables>(iterables))...));
}

// End of group
/**
 * @}
 */
} // namespace lz

#endif // LZ_CARTESIAN_PRODUCT_HPP
