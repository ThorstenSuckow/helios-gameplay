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

import helios.engine.runtime.concepts;

import helios.engine.runtime.pooling;


import helios.engine.runtime.world.UpdateContext;

import helios.engine.runtime.world.concepts;

import helios.core.log;

#define HELIOS_LOG_SCOPE "helios::gameplay::spawning::SpawnManager"
export namespace helios::gameplay::spawning {


    template<typename TEmitterHandle, typename TSpawnHandle = TEmitterHandle>
    class SpawnManager {


        using UpdateContext = engine::runtime::world::UpdateContext;
        using LogManager = core::log::LogManager;
        using CommandHandlerRegistry = ecs::command::CommandHandlerRegistry;
        using EcsDataContainer = ecs::common::container::EcsDataContainer;
        using SpawnCommand = commands::SpawnCommand<TEmitterHandle, TSpawnHandle>;
        using SpawnContext = types::SpawnContext;
        using EntityPoolRegistry = engine::runtime::pooling::EntityPoolRegistry;
        using PoolSnapshot = engine::runtime::pooling::types::PoolSnapshot;
        using EntityPool = engine::runtime::pooling::EntityPool;
        using SpawnEntityType = ecs::Entity<ecs::EntityManager<TSpawnHandle>>;
        using EmitterEntityType = ecs::Entity<ecs::EntityManager<TEmitterHandle>>;
        using TypedSpawnContext = types::TypedSpawnContext<TEmitterHandle, TSpawnHandle>;
        using HandleSpanRef = ecs::common::types::HandleSpanRef;
        using SpawnEntityManager = ecs::EntityManager<TSpawnHandle>;
        using EntityRef = ecs::EntityRef;
        using EntitySpanRef = ecs::EntitySpanRef;

        static inline auto& logger_ = LogManager::loggerForScope(HELIOS_LOG_SCOPE);


        std::vector<commands::SpawnCommand<TEmitterHandle, TSpawnHandle>> spawnCommands_;



        bool executeSpawnCommand(
            UpdateContext& updateContext,
            SpawnCommand& spawnCommand,
            SpawnPolicyRegistry& spawnPolicyRegistry,
            EntityPoolRegistry& entityPoolRegistry,
            SpawnEntityManager& spawnEntityManager,
            EcsDataContainer& ecsDataContainer) {

            auto key = spawnCommand.spawnPolicyKey;

            auto* spawnPolicy = spawnPolicyRegistry.item(key);

            if (!spawnPolicy) {
                logger_.warn("SpawnPolicy with typeId.value {0} and index {1} missing", key.typeId.value(), key.index);
                assert(false && "SpawnPolicy missing");
                return false;
            }


            auto entityPoolKey = spawnCommand.entityPoolKey;

            auto* entityPool = entityPoolRegistry.item(entityPoolKey);
            if (!entityPool || entityPool->typeId().value() != ecs::common::types::HandleTypeId::template id<TSpawnHandle>().value()) {
                logger_.warn("entityPool not found or type id mismatch!");
                assert(false && "entityPool not found or type id mismatch");
                return false;
            }

            auto typedSpawnContext = TypedSpawnContext{
               .requiredAmount = spawnCommand.amount,
               .frame = updateContext.frameCount(),
               .deltaTime = updateContext.deltaTime(),
                .poolSnapshot = PoolSnapshot{entityPool->activeCount(), entityPool->inactiveCount()}
           };

            return processSpawnPolicy(
                *spawnPolicy,
                SpawnContext::make<TEmitterHandle, TSpawnHandle>(std::move(typedSpawnContext)),
                *entityPool,
                spawnEntityManager,
                ecsDataContainer
                );


        }

        /**
         * @brief Executes one spawn policy and manages acquisition/release in its target pool.
         */
        bool processSpawnPolicy(
            SpawnPolicy& spawnPolicy,
            SpawnContext spawnContext,
            EntityPool& entityPool,
            SpawnEntityManager& spawnEntityManager,
            EcsDataContainer& ecsDataContainer) noexcept {


            if (!spawnPolicy.update(ecsDataContainer, spawnContext)) {
                return false;
            }

            auto amount = spawnPolicy.spawnCount(ecsDataContainer, spawnContext);
            if (amount == 0) {
                return false;
            }

            std::vector<SpawnEntityType> spawnedEntities;
            std::size_t spawned = 0;
            auto acquiredHandles = entityPool.acquire<TSpawnHandle>(amount);
            for (auto& handle : acquiredHandles) {
                if (auto entity = spawnEntityManager.entity(handle)) {
                    if (spawnPolicy.onBeforeSpawn(ecsDataContainer, entityPool, EntityRef{handle, spawnEntityManager})) {
                        spawnedEntities.push_back(*entity);
                        spawned++;
                    }
                } else {
                    entityPool.releaseAndRemove(handle);
                }

            }


            std::size_t used = spawnPolicy.spawn(ecsDataContainer, spawnContext, EntitySpanRef{spawnedEntities});
            assert(used <= spawned && "used must be less than or equal to spawned");
            spawnContext.addToSpawnedAmount(used);
            if (used != spawned) {
                const auto bound = spawned - used;
                auto i = 0;
                for (auto& entity : std::views::reverse(spawnedEntities)) {
                    entityPool.release(entity.handle());
                    if (++i == bound) {
                        break;
                    }
                }
            }




            return true;
        }

    public:


        SpawnManager(const SpawnManager&) = delete;
        SpawnManager& operator=(const SpawnManager&) = delete;
        SpawnManager(SpawnManager&&) = default;
        SpawnManager& operator=(SpawnManager&&) = default;

        explicit SpawnManager() = default;


        bool submit(commands::SpawnCommand<TEmitterHandle, TSpawnHandle>&& spawnCommand) noexcept {
            spawnCommands_.push_back(std::move(spawnCommand));

            return true;
        }

        /**
         * @brief Registers all supported spawn command handlers.
         *
         * @param commandHandlerRegistry Registry receiving command bindings.
         */
        bool init(ecs::command::CommandHandlerRegistry& commandHandlerRegistry) noexcept {

            commandHandlerRegistry.handleCommands<
                commands::SpawnCommand<TEmitterHandle, TSpawnHandle>
                >(*this);

            return true;
        }


        bool executeCommands(
            UpdateContext& updateContext, SpawnPolicyRegistry& spawnPolicyRegistry,
            EntityPoolRegistry& entityPoolRegistry, SpawnEntityManager& spawnEntityManager,
            EcsDataContainer& ecsDataContainer) noexcept {

            for (auto& spawnCommand : spawnCommands_) {
                executeSpawnCommand(
                    updateContext, spawnCommand, spawnPolicyRegistry, entityPoolRegistry, spawnEntityManager, ecsDataContainer
                );
            }

            spawnCommands_.clear();
            return true;
        }

        void reset() {
            /*intentionally noop*/
        }

    };

}