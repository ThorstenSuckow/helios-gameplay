/**
 * @file LifetimeComponent.ixx
 * @brief Lifetime component for tracking age and lifetime.
 */
module;


export module helios.gameplay.lifecycle.components:LifetimeComponent;


export namespace helios::gameplay::lifecycle::components {

    class LifetimeComponent {
        float age_{};

        float lifetime_{};

    public:


        explicit LifetimeComponent(const float lifetime)
        : lifetime_(lifetime) {}

        void tick (const float deltaTime) noexcept {
            age_ += deltaTime;
        }

        [[nodiscard]] float value() const noexcept {
            return age_;
        }

        [[nodiscard]] float lifetime() const noexcept {
            return lifetime_;
        }

        [[nodiscard]] bool isExpired() const noexcept {
            return age_ >= lifetime_;
        }

    };

}