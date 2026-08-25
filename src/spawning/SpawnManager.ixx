/**
 * @file SpawnManager.ixx
 * @brief Manager that consumes spawn commands and executes spawn policies using entity pools.
 */
module;

#include <vector>
#include <cassert>
#include <optional>
#include <ranges>

export module helios.gameplay.spawning.SpawnManager;

import helios.ecs;

import helios.gameplay.spawning.commands;
import helios.gameplay.spawning.types;
import helios.gameplay.spawning.concepts;
import helios.gameplay.spawning.SpawnPolicyRegistry;
import helios.gameplay.spawning.SpawnPolicy;

import helios.engine.core.types;

import helios.engine.spatial.components;

import helios.engine.runtime.particle.types;
import helios.engine.runtime.concepts;

import helios.engine.runtime.pooling.TypedEntityPoolRegistry;
import helios.engine.runtime.pooling.types;

import helios.gameplay.spawning.TypedSpawnPolicyRegistry;

import helios.engine.runtime.world.UpdateContext;

import helios.engine.runtime.world.types;
import helios.engine.runtime.world.concepts;

import helios.core.log;

#define HELIOS_LOG_SCOPE "helios::gameplay::spawning::SpawnManager"
export namespace helios::gameplay::spawning {

    /**
     * @brief Forward declaration for `SpawnManager` specializations.
     *
     * @tparam TSpawnPolicyRegistry Registry type used for spawn policies.
     * @tparam TEntityPoolRegistry Registry type used for entity pools.
     */
    template<typename TSpawnPolicyRegistry, typename TEntityPoolRegistry>
    class SpawnManager;

    /**
     * @brief Spawn manager specialization for typed spawn-policy and entity-pool registries.
     *
     * @tparam TSpawnPolicyStrongIdLookupStrategy Strong-ID lookup strategy template used by policy registry.
     * @tparam TEntityPoolStrongIdLookupStrategy Strong-ID lookup strategy template used by pool registry.
     * @tparam TMemberHandles Handle types supported as emitter/spawn domains.
     */
    template<
        template<typename> typename TSpawnPolicyStrongIdLookupStrategy,
        template<typename> typename TEntityPoolStrongIdLookupStrategy,
        typename ...TMemberHandles
    >
    class SpawnManager<
        gameplay::spawning::TypedSpawnPolicyRegistry<TSpawnPolicyStrongIdLookupStrategy, TMemberHandles...>,
        engine::runtime::pooling::TypedEntityPoolRegistry<TEntityPoolStrongIdLookupStrategy, TMemberHandles...>
    > {


        /**
         * @brief Alias for world update context.
         */
        using UpdateContext = engine::runtime::world::UpdateContext;

        /**
         * @brief Alias for engine log manager.
         */
        using LogManager = core::log::LogManager;

        /**
         * @brief Alias for command handler registry.
         */
        using CommandHandlerRegistry = ecs::command::CommandHandlerRegistry;

        /**
         * @brief Typed registry alias for entity pools.
         */
        using EngineEntityPoolRegistry = engine::runtime::pooling::TypedEntityPoolRegistry<TEntityPoolStrongIdLookupStrategy, TMemberHandles...>;

        /**
         * @brief Typed registry alias for spawn policies.
         */
        using EngineSpawnPolicyRegistry = spawning::TypedSpawnPolicyRegistry<TSpawnPolicyStrongIdLookupStrategy, TMemberHandles...>;

        static inline auto& logger_ = LogManager::loggerForScope(HELIOS_LOG_SCOPE);


        /**
         * @brief Reference to the global entity pool registry.
         */
        EngineEntityPoolRegistry& entityPoolRegistry_;

        /**
         * @brief Reference to the global spawn policy registry.
         */
        EngineSpawnPolicyRegistry& spawnPolicyRegistry_;

        /**
         * @brief Spawn context data bound to one pool key.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        struct SpawnContextSlot {
            /**
             * @brief Target pool key for spawned entities.
             */
            engine::runtime::pooling::types::EntityPoolKey<TSpawnHandle> entityPoolKey;

            /**
             * @brief Mutable runtime spawn context.
             */
            types::SpawnContext<TEmitterHandle, TSpawnHandle> spawnContext;
        };

        /**
         * @brief Spawn policy entry with associated pools and cached contexts.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        struct SpawnPolicySlot {
            /**
             * @brief Key used to resolve the spawn policy wrapper.
             */
            types::SpawnPolicyKey<TEmitterHandle, TSpawnHandle> spawnPolicyKey;

            /**
             * @brief Pool keys usable by this policy.
             */
            std::vector<engine::runtime::pooling::types::EntityPoolKey<TSpawnHandle>> entityPoolKeys;

            /**
             * @brief Cached spawn contexts for policy execution.
             */
            std::vector<SpawnContextSlot<TEmitterHandle, TSpawnHandle>> spawnContextSlots;
        };

        /**
         * @brief Optional slot collection for one emitter/spawn pair.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        using SpawnPolicySlots = std::vector<std::optional<SpawnPolicySlot<TEmitterHandle, TSpawnHandle>>>;

        /**
         * @brief Tuple fragment containing all spawn-policy slot vectors for one emitter.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type pack.
         */
        template<typename TEmitterHandle, typename ... TSpawnHandle>
        using SpawnPolicySinkForHandlePair = std::tuple<SpawnPolicySlots<TEmitterHandle, TSpawnHandle> ...>;

        /**
         * @brief Flat tuple type of all spawn-policy slot vectors.
         *
         * @tparam THandles Handle type pack.
         */
        template<typename ... THandles>
        using SpawnPolicySlotVectors = decltype(std::tuple_cat(
            std::declval<SpawnPolicySinkForHandlePair<THandles, THandles...>>()...)
        );

        /**
         * @brief Storage for all spawn-policy slot vectors.
         */
        SpawnPolicySlotVectors<TMemberHandles...> spawnPolicySlotVectors_;

        /**
         * @brief Tuple fragment containing command queues for one emitter.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type pack.
         */
        template<typename TEmitterHandle, typename ...TSpawnHandle>
        using SpawnCommandSinkForHandlePair = std::tuple<
            std::vector<commands::SpawnCommand<TEmitterHandle, TSpawnHandle>> ...
        >;

        /**
         * @brief Flat tuple type of all command queues.
         *
         * @tparam THandles Handle type pack.
         */
        template<typename ... THandles>
        using SpawnCommandVectors = decltype(std::tuple_cat(
            std::declval<SpawnCommandSinkForHandlePair<THandles, THandles...>>()...)
        );

        /**
         * @brief Storage for all queued spawn commands.
         */
        SpawnCommandVectors<TMemberHandles...> spawnCommandVectors_;


        /**
         * @brief Registers all spawn command handlers for every supported handle pair.
         *
         * @tparam THandles Handle type pack used as emitters.
         * @param commandHandlerRegistry Command handler registry to bind this manager to.
         */
        template<typename ... THandles>
        void registerAllCommands(CommandHandlerRegistry& commandHandlerRegistry) {
            (registerCommandsForEmitter<THandles, THandles...>(commandHandlerRegistry), ...);
        }

        /**
         * @brief Registers spawn command handlers for one emitter type against all spawn types.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type pack.
         * @param commandHandlerRegistry Command handler registry to register into.
         */
        template<typename TEmitterHandle, typename ... TSpawnHandle>
        void registerCommandsForEmitter(CommandHandlerRegistry& commandHandlerRegistry) {
            (commandHandlerRegistry.registerHandler<
                spawning::commands::SpawnCommand<TEmitterHandle, TSpawnHandle>
            >(*this), ...);
        }

        /**
         * @brief Returns the policy-slot vector for a specific emitter/spawn pair.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @return Reference to the slot vector.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        auto& spawnPolicyVector() {
            return std::get<SpawnPolicySlots<TEmitterHandle, TSpawnHandle>>(spawnPolicySlotVectors_);
        }

        /**
         * @brief Returns the command queue for a specific emitter/spawn pair.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @return Reference to the command vector.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        auto& spawnCommandVector() {
            return std::get<std::vector<commands::SpawnCommand<TEmitterHandle, TSpawnHandle>>>(spawnCommandVectors_);
        }


        /**
         * @brief Flushes queued spawn commands for one emitter/spawn pair.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @param updateContext Current world update context.
         * @param entityManager Entity manager for the spawned entity type.
         * @param spawnCommands Queue to process.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        void executeSpawnCommands(
            UpdateContext& updateContext,
            ecs::EntityManager<TSpawnHandle>& entityManager,
            std::vector<commands::SpawnCommand<TEmitterHandle, TSpawnHandle>>& spawnCommands) {

            for (auto& spawnCommand: spawnCommands) {

                auto key = spawnCommand.spawnPolicyKey.toConceptModelCollectionKey();

                auto* spawnPolicy = spawnPolicyRegistry_.item(key);

                if (!spawnPolicy) {
                    logger_.warn("SpawnPolicy with typeId.value {0} and index {1} missing", key.typeId.value(), key.index);
                    assert(false && "SpawnPolicy missing");
                    continue;
                }

                auto spawnContext = types::SpawnContext<TEmitterHandle, TSpawnHandle>{
                    .requiredAmount = spawnCommand.amount,
                    .frame = updateContext.frameCount(),
                    .deltaTime = updateContext.deltaTime()
                };

                auto entityPoolKey = spawnCommand.entityPoolKey;

                auto* pool = entityPoolRegistry_.template pool<TSpawnHandle>(entityPoolKey);
                if (!pool) {
                    logger_.warn("pool not found!");
                    assert(false && "pool not found");
                    continue;
                }

                processSpawnPolicy(entityManager, spawnPolicy, updateContext, spawnContext, pool, spawnCommand.emitterHandle);
            }

        }

        /**
         * @brief Executes one spawn policy and manages acquisition/release in its target pool.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @param entityManager Entity manager for the spawned entity type.
         * @param spawnPolicy Spawn policy wrapper to execute.
         * @param updateContext Current world update context.
         * @param spawnContext Mutable spawn context for this execution.
         * @param pool Target entity pool.
         * @param emitterHandle Handle of the command emitter.
         * @return `true` if the policy produced spawn activity, otherwise `false`.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        bool processSpawnPolicy(
            ecs::EntityManager<TSpawnHandle>& entityManager,
            auto* spawnPolicy, UpdateContext& updateContext,
            types::SpawnContext<TEmitterHandle, TSpawnHandle>& spawnContext, auto* pool, TEmitterHandle emitterHandle) noexcept {

            using SpawnEntityType = ecs::Entity<ecs::EntityManager<TSpawnHandle>>;

            spawnContext.poolSnapshot = engine::runtime::pooling::types::PoolSnapshot{pool->activeCount(), pool->inactiveCount()};

            if (!spawnPolicy->update(updateContext, spawnContext)) {
                return false;
            }

            if (auto amount = spawnPolicy->spawnCount(updateContext, spawnContext); amount > 0) {
                std::vector<SpawnEntityType> spawnEntities;
                TSpawnHandle spawnHandle{};
                std::size_t spawned = 0;
                while (spawned < amount && pool->acquire(spawnHandle)) {
                    if (auto entity = entityManager.entity(spawnHandle)) {
                        if (spawnPolicy->onBeforeSpawn(*pool, *entity)) {
                            spawnEntities.push_back(*entity);
                            ++spawned;
                        }
                    } else {
                        pool->releaseAndRemove(spawnHandle);
                    }
                }

                std::size_t used = spawnPolicy->spawn(updateContext, spawnContext, spawnEntities);
                assert(used <= spawned && "used must be less than or equal to spawned");
                spawnContext.spawnedAmount += used;
                if (used != spawned) {
                    const auto bound = spawned - used;
                    auto i = 0;
                    for (auto& entity : std::views::reverse(spawnEntities)) {
                        pool->release(entity.handle());
                        if (++i == bound) {
                            break;
                        }
                    }
                }

            } else {
                return false;
            }


            return true;
        }

    public:



        /**
         * @brief Constructs the spawn manager with registries for policies and pools.
         *
         * @param spawnPolicyRegistry Registry used to resolve spawn policies.
         * @param entityPoolRegistry Registry used to resolve target pools.
         */
        explicit SpawnManager(
            EngineSpawnPolicyRegistry& spawnPolicyRegistry,
            EngineEntityPoolRegistry& entityPoolRegistry)
        : spawnPolicyRegistry_(spawnPolicyRegistry), entityPoolRegistry_(entityPoolRegistry) {}


        /**
         * @brief Enqueues one spawn command for deferred processing in `flush()`.
         *
         * @tparam TOwnerHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @param spawnCommand Spawn command to enqueue.
         * @return `true` when the command was accepted.
         */
        template<typename TOwnerHandle, typename TSpawnHandle = TOwnerHandle>
        bool submit(commands::SpawnCommand<TOwnerHandle, TSpawnHandle>&& spawnCommand) noexcept {
            auto& vec = spawnCommandVector<TOwnerHandle, TSpawnHandle>();
            vec.push_back(std::move(spawnCommand));

            return true;
        }

        /**
         * @brief Registers all supported spawn command handlers.
         *
         * @param commandHandlerRegistry Registry receiving command bindings.
         */
        bool init(ecs::command::CommandHandlerRegistry& commandHandlerRegistry) noexcept {
            registerAllCommands<TMemberHandles...>(commandHandlerRegistry);
            return true;
        }

        /**
         * @brief Processes all queued spawn commands and clears all queues.
         *
         * @param updateContext Current world update context.
         */
        bool executeCommands(engine::runtime::world::UpdateContext& updateContext, ecs::EcsWorld& ecsWorld) noexcept {

            std::apply([&updateContext, &ecsWorld, this](auto& ... spawnCommands) {
                ([&] () {
                    using SpawnCommandVectorType = std::remove_cvref_t<decltype(spawnCommands)>::value_type;
                    using SpawnCommandType = std::remove_cvref_t<SpawnCommandVectorType>;
                    executeSpawnCommands(
                        updateContext,
                        ecsWorld.entityManager<typename SpawnCommandType::SpawnHandleType>(),
                        spawnCommands
                    );
                }(), ...);
            }, spawnCommandVectors_);

            std::apply([](auto& ...spawnCommands) noexcept {
                (spawnCommands.clear(), ...);
            }, spawnCommandVectors_);

            return true;
        }

        void reset() {
            /*intentionally noop*/
        }

    };

}