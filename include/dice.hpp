#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>

class Dice {
   private:
    int32_t faces_;
    int32_t amount_;

    Dice() {}

   public:
    explicit Dice(int32_t faces = 4, int32_t amount = 1) {
        assert(faces >= 4 && amount >= 1);

        faces_ = faces;
        amount_ = amount;
    }

    int32_t get_faces() const { return faces_; }

    int32_t get_amount() const { return amount_; }
};

int dice(const Dice& dice_instance, int32_t modifier);

int dice(const Dice* dices, size_t size, int32_t modifier);

template <typename T>
T get_random(T min, T max) {
    static_assert(std::is_arithmetic_v<T>, "Template parameter must be arithmetic");

    assert(min < max);

    static thread_local std::mt19937 gen(std::random_device{}());

    if constexpr (std::is_integral_v<T>) {
        using dist_t = std::uniform_int_distribution<std::make_signed_t<T>>;
        dist_t dist(static_cast<std::make_signed_t<T>>(min), static_cast<std::make_signed_t<T>>(max));
        return static_cast<T>(dist(gen));
    } else {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(gen);
    }
}
