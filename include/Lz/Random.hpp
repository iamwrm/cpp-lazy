#pragma once

#ifndef LZ_RANDOM_HPP
#    define LZ_RANDOM_HPP

#    include "detail/BasicIteratorView.hpp"
#    include "detail/RandomIterator.hpp"

#    include <random>

namespace lz {
namespace internal {
template<std::size_t N>
class SeedSequence {
public:
    using result_type = std::seed_seq::result_type;

private:
    using SeedArray = std::array<result_type, N>;
    SeedArray _seed{};

    template<class Iter>
    LZ_CONSTEXPR_CXX_20 void create(Iter begin, Iter end) {
        using ValueType = ValueType<Iter>;
        std::transform(begin, end, _seed.begin(), [](const ValueType val) { return static_cast<result_type>(val); });
    }

    result_type T(const result_type x) const { // NOLINT
        return x ^ (x >> 27u);
    }

public:
    constexpr SeedSequence() = default;

    explicit SeedSequence(std::random_device& rd) {
        std::generate(_seed.begin(), _seed.end(), [&rd]() { return static_cast<result_type>(rd()); });
    }

    template<class T>
    LZ_CONSTEXPR_CXX_20 SeedSequence(std::initializer_list<T> values) {
        create(values.begin(), values.end());
    }

    template<class Iter>
    LZ_CONSTEXPR_CXX_20 SeedSequence(Iter first, Iter last) {
        create(first, last);
    }

    SeedSequence(const SeedSequence&) = delete;
    SeedSequence& operator=(const SeedSequence&) = delete;

    template<class Iter>
    LZ_CONSTEXPR_CXX_20 void generate(Iter begin, Iter end) const {
        if (begin == end) {
            return;
        }

        using IterValueType = ValueType<Iter>;

        std::fill(begin, end, 0x8b8b8b8b);
        const auto n = static_cast<std::size_t>(end - begin);
        constexpr auto s = N;
        const std::size_t m = std::max(s + 1, n);
        const std::size_t t = (n >= 623) ? 11 : (n >= 68) ? 7 : (n >= 39) ? 5 : (n >= 7) ? 3 : (n - 1) / 2;
        const std::size_t p = (n - t) / 2;
        const std::size_t q = p + t;

        IterValueType mask = static_cast<IterValueType>(1) << 31;
        mask <<= 1;
        mask -= 1;

        for (std::size_t k = 0; k < m - 1; k++) {
            const std::size_t kModN = k % n;
            const std::size_t kPlusPModN = (k + p) % n;
            const result_type r1 = 1664525 * T(begin[kModN] ^ begin[kPlusPModN] ^ begin[(k - 1) % n]);

            result_type r2;
            if (k == 0) {
                r2 = static_cast<result_type>((r1 + s) & mask);
            }
            else if (k <= s) {
                r2 = static_cast<result_type>((r1 + kModN + _seed[k - 1]) & mask);
            }
            else {
                r2 = static_cast<result_type>((r1 + kModN) & mask);
            }

            begin[kPlusPModN] += (r1 & mask);
            begin[(k + q) % n] += (r2 & mask);
            begin[kModN] = r2;
        }

        for (std::size_t k = m; k < m + n - 1; k++) {
            const std::size_t kModN = k % n;
            const std::size_t kPlusPModN = (k + p) % n;
            const result_type r3 = 1566083941 * T(begin[kModN] + begin[kPlusPModN] + begin[(k - 1) % n]);
            const auto r4 = static_cast<result_type>((r3 - kModN) & mask);

            begin[kPlusPModN] ^= (r3 & mask);
            begin[(k + q) % n] ^= (r4 & mask);
            begin[kModN] = r4;
        }
    }

    template<class Iter>
    LZ_CONSTEXPR_CXX_20 void param(Iter outputIterator) const {
        std::copy(_seed.begin(), _seed.end(), outputIterator);
    }

    static constexpr std::size_t size() {
        return N;
    }
};

inline std::mt19937 createMtEngine() {
    std::random_device rd;
    SeedSequence<8> seedSeq(rd);
    return std::mt19937(seedSeq);
}
} // namespace internal

template<LZ_CONCEPT_ARITHMETIC Arithmetic, class Distribution, class Generator>
class Random final : public internal::BasicIteratorView<internal::RandomIterator<Arithmetic, Distribution, Generator>> {
public:
    using iterator = internal::RandomIterator<Arithmetic, Distribution, Generator>;
    using const_iterator = iterator;
    using value_type = typename iterator::value_type;

    Random(const Distribution& distribution, Generator& generator, const std::ptrdiff_t amount, const bool isWhileTrueLoop) :
        internal::BasicIteratorView<iterator>(iterator(distribution, generator, 0, isWhileTrueLoop),
                                              iterator(distribution, generator, amount, isWhileTrueLoop)) {
    }

    Random() = default;

    /**
     * Creates a new random number, not taking into account its size. This is for pure convenience. Example:
     * ```cpp
     * auto rand = lz::random(0, 5);
     * for (int i = 0; i < 50_000; i++) {
     *     int myRandom = rand.nextRandom();
     * }
     * ```
     * @return A new random `value_type` between [min, max].
     */
    LZ_NODISCARD value_type nextRandom() const {
        return *this->begin();
    }

    /**
     * Gets the minimum random value.
     * @return The min value
     */
    LZ_NODISCARD value_type minRandom() const {
        return (this->begin().min)();
    }

    /**
     * Gets the maximum random value.
     * @return The max value
     */
    LZ_NODISCARD value_type maxRandom() const {
        return (this->begin().max)();
    }
};

/**
 * @addtogroup ItFns
 * @{
 */

/**
 * Creates a random number generator with specified generator and distribution.
 * @param distribution A number distribution, for e.g. std::uniform_<type>_distribution<type>.
 * @param generator A random number generator, for e.g. std::mt19937.
 * @param amount The amount of numbers to create.
 * @return A random view object that generates a sequence of `Generator::result_type`
 */
template<class Generator, class Distribution>
LZ_NODISCARD Random<typename Distribution::result_type, Distribution, Generator>
random(const Distribution& distribution, Generator& generator,
       const std::size_t amount = (std::numeric_limits<std::size_t>::max)()) {
    return { distribution, generator, static_cast<std::ptrdiff_t>(amount), amount == (std::numeric_limits<std::size_t>::max)() };
}

#    ifdef __cpp_if_constexpr
/**
 * @brief Returns an iterator view object that generates a sequence of random numbers, using an uniform distribution.
 * @details This random access iterator view object can be used to generate a sequence of random numbers between
 * [`min, max`]. It uses the std::mt19937 random engine and a seed sequence of 8 x `std::random_device` as seed. The
 * seed sequence is a custom implementation of `std::seed_seq`. Internally, it uses a `std::array` instead of a `std::vector` and
 * tends to be more faster than its `std::seed_seq` implementation.
 * @param min The minimum value, included.
 * @param max The maximum value, included.
 * @tparam Distribution The distribution for generating the random numbers. `std::uniform_int_distribution` by default.
 * @tparam Generator The random number generator. `std::mt19937` by default.
 * @param amount The amount of numbers to create. If left empty or equal to `std::numeric_limits<std::size_t>::max()`
 * it is interpreted as a `while-true` loop.
 * @return A random view object that generates a sequence of random numbers
 */
template<LZ_CONCEPT_ARITHMETIC Arithmetic>
LZ_NODISCARD auto
random(const Arithmetic min, const Arithmetic max, const std::size_t amount = (std::numeric_limits<std::size_t>::max)()) {
#        ifndef LZ_HAS_CONCEPTS
    static_assert(std::is_arithmetic_v<Arithmetic>, "min/max type should be arithmetic");
#        endif // LZ_HAS_CONCEPTS
    static std::mt19937 gen = internal::createMtEngine();
    if constexpr (std::is_integral_v<Arithmetic>) {
        std::uniform_int_distribution<Arithmetic> dist(min, max);
        return random(dist, gen, amount);
    }
    else {
        std::uniform_real_distribution<Arithmetic> dist(min, max);
        return random(dist, gen, amount);
    }
}
#    else
/**
 * @brief Returns an iterator view object that generates a sequence of random numbers, using an uniform distribution.
 * @details This random access iterator view object can be used to generate a sequence of random numbers between
 * [`min, max`]. It uses the std::mt19937 random engine and a seed sequence of 8 x `std::random_device` as seed. The
 * seed sequence is a custom implementation of `std::seed_seq`. Internally, it uses a `std::array` instead of a `std::vector` and
 * tends to be more faster than its `std::seed_seq` implementation.
 * @param min The minimum value , included.
 * @param max The maximum value, included.
 * @tparam Distribution The distribution for generating the random numbers. `std::uniform_int_distribution` by default.
 * @tparam Generator The random number generator. `std::mt19937` by default.
 * @param amount The amount of numbers to create. If left empty or equal to `std::numeric_limits<std::size_t>::max()`
 * it is interpreted as a `while-true` loop.
 * @return A random view object that generates a sequence of random numbers
 */
template<class Integral>
LZ_NODISCARD
internal::EnableIf<std::is_integral<Integral>::value, Random<Integral, std::uniform_int_distribution<Integral>, std::mt19937>>
random(const Integral min, const Integral max, const std::size_t amount = (std::numeric_limits<std::size_t>::max)()) {
    static std::mt19937 gen = internal::createMtEngine();
    std::uniform_int_distribution<Integral> dist(min, max);
    return random(dist, gen, amount);
}

/**
 * @brief Returns an output view object that generates a sequence of floating point doubles, using a uniform
 * distribution.
 * @details This random access iterator view object can be used to generate a sequence of random doubles between
 * [`min, max`]. It uses the std::mt19937 random engine and a seed of `std::random_device` as seed.
 * @tparam Distribution The distribution for generating the random numbers. `std::uniform_real_distribution` by default.
 * @tparam Generator The random number generator. `std::mt19937` by default.
 * @param min The minimum value, included.
 * @param max The maximum value, included.
 * @param amount The amount of numbers to create. If left empty or equal to `std::numeric_limits<std::size_t>::max()`
 * it is interpreted as a `while-true` loop.
 * @return A random view object that generates a sequence of random floating point values.
 */
template<class Floating>
LZ_NODISCARD internal::EnableIf<std::is_floating_point<Floating>::value,
                                Random<Floating, std::uniform_real_distribution<Floating>, std::mt19937>>
random(const Floating min, const Floating max, const std::size_t amount = (std::numeric_limits<std::size_t>::max)()) {
    static std::mt19937 gen = internal::createMtEngine();
    std::uniform_real_distribution<Floating> dist(min, max);
    return random(dist, gen, amount);
}

#    endif // __cpp_if_constexpr

// End of group
/**
 * @}
 */

} // namespace lz

#endif