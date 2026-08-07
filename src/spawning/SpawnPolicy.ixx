/**
 * @file SpawnPolicy.ixx
 * @brief Type-erased wrapper for spawn policy implementations.
 */
module;

#include <memory>
#include <vector>

export module helios.gameplay.spawning.SpawnPolicy;

import helios.gameplay.spawning.types;
import helios.engine.runtime.world.UpdateContext;

using namespace helios::gameplay::spawning::types;
using namespace helios::engine::runtime::world;
export namespace helios::gameplay::spawning {

    /**
     * @brief Type-erased container for a concrete spawn policy operating on `TEmitterHandle`/`TSpawnHandle` pairs.
     *
     * @tparam TEmitterHandle  Handle type of the entity that triggers spawning.
     * @tparam TSpawnHandle    Handle type of the entities to be spawned; defaults to `TEmitterHandle`.
     */
    template<typename TEmitterHandle, typename TSpawnHandle = TEmitterHandle>
    class SpawnPolicy {

        /**
         * @brief Spawn context type alias for this handle pair.
         */
        using SpawnContext = SpawnContext<TEmitterHandle, TSpawnHandle>;

        /**
         * @brief Abstract interface for the type-erased spawn policy.
         */
        class Concept {
        public:

            virtual ~Concept() = default;

            /**
             * @brief Returns the number of entities to spawn in the current frame.
             */
            virtual std::size_t spawnCount(UpdateContext& updateContext, const SpawnContext& context) = 0;

            /**
             * @brief Executes the spawn logic; returns the number of successfully spawned entities.
             */
            virtual std::size_t spawn(UpdateContext& updateContext, const SpawnContext& context, std::span<const TSpawnHandle> spawnHandles) = 0;

            /**
             * @brief Executes the update logic for the submitted SpawnContext; returns `true` on success.
             */
            virtual bool update(UpdateContext& updateContext, SpawnContext& context) = 0;


            /**
             * @brief Returns a raw pointer to the underlying concrete policy.
             */
            [[nodiscard]] virtual void* underlying() noexcept = 0;

            /**
             * @brief Returns a const raw pointer to the underlying concrete policy.
             */
            [[nodiscard]] virtual const void* underlying() const noexcept = 0;
        };

        /**
         * @brief Concrete model wrapping a value of type `T`.
         *
         * @tparam T  Concrete spawn policy type.
         */
        template<typename T>
        class Model : public Concept {

            /**
             * @brief Stored concrete policy instance.
             */
            T policy_;

        public:

            /**
             * @brief Constructs the model by moving the given policy.
             */
            explicit Model(T policy) : policy_(std::move(policy)) {}

            /**
             * @brief Delegates to `policy_.spawn()` to compute the spawn count.
             */
            std::size_t spawnCount(UpdateContext& updateContext, const SpawnContext& spawnContext) override {
                return policy_.spawnCount(updateContext, spawnContext);
            }

            /**
             * @brief Delegates to `policy_.spawn()` to execute spawning.
             */
            std::size_t spawn(UpdateContext& updateContext, const SpawnContext& spawnContext, std::span<const TSpawnHandle> spawnHandles) override {
                return policy_.spawn(updateContext, spawnContext, spawnHandles);
            }

            /**
             * @brief Delegates to `policy_.update()` to execute updating.
             */
            bool update(UpdateContext& updateContext, SpawnContext& spawnContext) override {
                return policy_.update(updateContext, spawnContext);
            }

            /**
             * @brief Returns a raw pointer to the stored policy.
             */
            [[nodiscard]] void* underlying() noexcept override {
                return &policy_;
            }

            /**
             * @brief Returns a const raw pointer to the stored policy.
             */
            [[nodiscard]] const void* underlying() const noexcept override {
                return &policy_;
            }

        };

        /**
         * @brief Owning pointer to the type-erased policy model.
         */
        std::unique_ptr<Concept> pimpl_;

    public:

        /**
         * @brief Constructs a `SpawnPolicy` wrapping a concrete policy of type `T`.
         *
         * @tparam T           Concrete spawn policy type.
         * @param spawnPolicy  Policy instance to wrap (moved into storage).
         */
        template<typename T>
        SpawnPolicy(T spawnPolicy)
        : pimpl_(std::make_unique<Model<T>>(
            std::move(spawnPolicy)

        )) {}

        SpawnPolicy(const SpawnPolicy& other) = delete;
        SpawnPolicy& operator=(const SpawnPolicy& other) = delete;
        SpawnPolicy(SpawnPolicy&& other) noexcept = default;
        SpawnPolicy& operator=(SpawnPolicy&& other) noexcept = default;

        /**
         * @brief Returns the number of entities to spawn in the current frame.
         */
        std::size_t spawnCount(UpdateContext& updateContext, const SpawnContext& context) {
            return pimpl_->spawnCount(updateContext, context);
        }

        /**
         * @brief Executes the spawn logic; returns the number of successfully spawned entities.
         */
        std::size_t spawn(UpdateContext& updateContext, const SpawnContext& context, std::span<const TSpawnHandle> spawnHandles) {
            return pimpl_->spawn(updateContext, context, spawnHandles);
        }

        /**
         * @bried Updates the submitted SpawnContext.
         *
         * @param updateContext
         * @param context
         * @return
         */
        bool update(UpdateContext& updateContext, SpawnContext& context) {
            return pimpl_->update(updateContext, context);
        }

        /**
         * @brief Delegates to `policy_.underlying()` to return a raw pointer to the stored policy.
         */
        void* underlying() noexcept {
            return pimpl_->underlying();
        }

        /**
         * @brief Delegates to `policy_.underlying()` to return a const raw pointer to the stored policy.
         */
        const void* underlying() const noexcept {
            return pimpl_->underlying();
        }

    };


}