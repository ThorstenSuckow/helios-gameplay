/**
 * @file SpawnPolicy.ixx
 * @brief Type-erased wrapper for spawn policy implementations.
 */
module;

#include <memory>
#include <vector>
#include <span>
#include <cassert>

export module helios.gameplay.spawning.SpawnPolicy;

import helios.ecs;

import helios.gameplay.spawning.types;
import helios.engine.runtime.gameloop.types;
import helios.engine.runtime.pooling.EntityPool;

using namespace helios::gameplay::spawning::types;

export namespace helios::gameplay::spawning {




    /**
     * @brief Type-erased container for a concrete spawn policy operating on `TEmitterHandle`/`TSpawnHandle` pairs.
     *
     * @tparam TEmitterHandle  Handle type of the entity that triggers spawning.
     * @tparam TSpawnHandle    Handle type of the entities to be spawned; defaults to `TEmitterHandle`.
     */
    class SpawnPolicy {

        /**
         * @brief Spawn context type alias for this handle pair.
         */
        using EntityPool = helios::engine::runtime::pooling::EntityPool;
        using EcsDataContainer = ecs::common::container::EcsDataContainer;
        using EntityRef = ecs::EntityRef;
        using EntitySpanRef = ecs::EntitySpanRef;

        template<typename TMutationSink = std::monostate>
        using EcsDataContainerArgumentResolver = ecs::common::container::EcsDataContainerArgumentResolver<TMutationSink>;

        using EcsDataContainerFunctionInvoker = ecs::common::container::EcsDataContainerFunctionInvoker;

        /**
         * @brief Abstract interface for the type-erased spawn policy.
         */
        class Concept {
        public:

            virtual ~Concept() = default;

            /**
             * @brief Returns the number of entities to spawn in the current frame.
             */
            virtual std::size_t spawnCount(EcsDataContainer& ecsDataContainer, SpawnContext& spawnContext) = 0;

            /**
             * @brief Executes the spawn logic; returns the number of successfully spawned entities.
             */
            virtual std::size_t spawn(EcsDataContainer& ecsDataContainer, SpawnContext& spawnContext, EntitySpanRef spawnEntitySpanRef) = 0;

            /**
             * @brief Executes the update logic for the submitted SpawnContext; returns `true` on success.
             */
            virtual bool update(EcsDataContainer& ecsDataContainer, SpawnContext& spawnContext) = 0;

            /**
             * @brief Hook for custom logic when an entity was acquired from the pool, before it is spawned.
             *
             * @return true to continue spawning, otherwise false.
             */
            virtual bool onBeforeSpawn(EcsDataContainer& ecsDataContainer, EntityPool& pool, EntityRef spawnEntity) = 0;

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
        class Model;

        template<
            template <typename, typename> typename T,
            typename TEmitterHandle,
            typename TSpawnHandle
        >
        class Model<T<TEmitterHandle, TSpawnHandle>> : public Concept {

            /**
             * @brief Stored concrete policy instance.
             */
            using TConcretePolicy = T<TEmitterHandle, TSpawnHandle>;
            
            TConcretePolicy policy_;


        public:

            /**
             * @brief Constructs the model by moving the given policy.
             */
            explicit Model(TConcretePolicy policy) : policy_(std::move(policy)) {}

            /**
             * @brief Delegates to `policy_.spawn()` to compute the spawn count.
             */
            std::size_t spawnCount(EcsDataContainer& ecsDataContainer, SpawnContext& spawnContext) override {
                auto& concreteSpawnContext = spawnContext.get<TEmitterHandle, TSpawnHandle>();

                return EcsDataContainerFunctionInvoker::invoke<&TConcretePolicy::spawnCount>(
                    policy_, ecsDataContainer, concreteSpawnContext
                );
            }

            /**
             * @brief Delegates to `policy_.spawn()` to execute spawning.
             */
            std::size_t spawn(EcsDataContainer& ecsDataContainer, SpawnContext& spawnContext, EntitySpanRef spawnEntities) override {

                auto& concreteSpawnContext = spawnContext.get<TEmitterHandle, TSpawnHandle>();
                auto concreteSpawnEntities = spawnEntities.get<TSpawnHandle>();

                return EcsDataContainerFunctionInvoker::invoke<&TConcretePolicy::spawn>(
                    policy_, ecsDataContainer, concreteSpawnContext, concreteSpawnEntities
                );
            }

            /**
             * @brief Delegates to `policy_.update()` to execute updating.
             */
            bool update(EcsDataContainer& ecsDataContainer, SpawnContext& spawnContext) override {
                auto& concreteSpawnContext = spawnContext.get<TEmitterHandle, TSpawnHandle>();

                return EcsDataContainerFunctionInvoker::invoke<&TConcretePolicy::update>(
                    policy_, ecsDataContainer, concreteSpawnContext
                );
            }

            /**
             * @copydoc Concept::onBeforeSpawn
             */
            bool onBeforeSpawn(EcsDataContainer& ecsDataContainer, EntityPool& pool, EntityRef spawnEntity) override {

                auto concreteEntity = spawnEntity.get<TSpawnHandle>();

                return EcsDataContainerFunctionInvoker::invoke<&TConcretePolicy::onBeforeSpawn>(
                    policy_, ecsDataContainer, pool, concreteEntity
                );
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
         * @brief Constructs a `SpawnPolicy` wrapping a concrete policy of type `TConcretePolicy`.
         *
         * @tparam TConcretePolicy           Concrete spawn policy type.
         * @param spawnPolicy  Policy instance to wrap (moved into storage).
         */
        template<typename TConcretePolicy>
        SpawnPolicy(TConcretePolicy spawnPolicy)
        : pimpl_(std::make_unique<Model<TConcretePolicy>>(
            std::move(spawnPolicy)

        )) {}

        SpawnPolicy(const SpawnPolicy& other) = delete;
        SpawnPolicy& operator=(const SpawnPolicy& other) = delete;
        SpawnPolicy(SpawnPolicy&& other) noexcept = default;
        SpawnPolicy& operator=(SpawnPolicy&& other) noexcept = default;

        /**
         * @brief Returns the number of entities to spawn in the current frame.
         */
        std::size_t spawnCount(EcsDataContainer& ecsDataContainer, SpawnContext& spawnContext) {
            return pimpl_->spawnCount(ecsDataContainer, spawnContext);
        }

        /**
         * @brief Executes the spawn logic; returns the number of successfully spawned entities.
         */
        std::size_t spawn(EcsDataContainer& ecsDataContainer, SpawnContext& spawnContext, EntitySpanRef spawnEntities) {
            return pimpl_->spawn(ecsDataContainer, spawnContext, spawnEntities);
        }

        /**
         * @copydoc Concept::onBeforeSpawn
         */
        bool onBeforeSpawn(EcsDataContainer& ecsDataContainer, EntityPool& pool, EntityRef spawnEntityRef) {
            return pimpl_->onBeforeSpawn(ecsDataContainer, pool, spawnEntityRef);
        }

        /**
         * @bried Updates the submitted SpawnContext.
         *
         * @param updateContext
         * @param context
         * @return
         */
        bool update(EcsDataContainer& ecsDataContainer, SpawnContext& spawnContext) {
            return pimpl_->update(ecsDataContainer, spawnContext);
        }

        /**
         * @brief Delegates to `policy_.underlying()` to return a raw pointer to the stored policy.
         */
        [[nodiscard]] void* underlying() noexcept {
            return pimpl_->underlying();
        }

        /**
         * @brief Delegates to `policy_.underlying()` to return a const raw pointer to the stored policy.
         */
        [[nodiscard]] void* underlying() const noexcept {
            return pimpl_->underlying();
        }

    };


}