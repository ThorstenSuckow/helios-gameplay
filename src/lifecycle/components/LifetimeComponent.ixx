/**
 * @file LifetimeComponent.ixx
 * @brief Lifetime component for tracking age and lifetime.
 */
module;

#include <cstdint>
#include <cmath>

export module helios.gameplay.lifecycle.components:LifetimeComponent;


export namespace helios::gameplay::lifecycle::components {

    class LifetimeComponent {
        float age_{};


        std::uint32_t tickLifetime_{};

        std::uint32_t ticks_{};

    public:


        static LifetimeComponent fromSeconds(const float lifetimeInSeconds, const float tickDurationInSeconds) {
            return LifetimeComponent(std::lround(lifetimeInSeconds / tickDurationInSeconds));
        }

        explicit LifetimeComponent(const std::uint32_t tickLifetime)
       : tickLifetime_(tickLifetime)
        {}

        void tick () noexcept {
            ticks_++;
        }

        [[nodiscard]] std::uint32_t value() const noexcept {
            return ticks_;
        }


        [[nodiscard]] std::uint32_t lifetime() const noexcept {
            return tickLifetime_;
        }

        [[nodiscard]] bool isExpired() const noexcept {
            return ticks_ >= tickLifetime_;
        }

        [[nodiscard]] float normalizedAge() const noexcept {
            return isExpired() ? 1.0F : static_cast<float>(ticks_) / static_cast<float>(tickLifetime_);
        }

    };

}
