module;

#include <vector>
#include <cassert>
#include <optional>
#include <ranges>

export module helios.gameplay.spawning.SpawnManager;

import helios.gameplay.spawning.commands;
import helios.gameplay.spawning.types;
import helios.gameplay.spawning.concepts;
import helios.gameplay.spawning.SpawnPolicyRegistry;
import helios.gameplay.spawning.SpawnPolicy;

import helios.engine.core.types;

import helios.engine.spatial.components;

import helios.engine.runtime.particle.types;

import helios.engine.runtime.pooling.TypedEntityPoolRegistry;
import helios.engine.runtime.pooling.types;

import helios.gameplay.spawning.TypedSpawnPolicyRegistry;

import helios.engine.runtime.world.UpdateContext;
import helios.engine.runtime.world.tags.ManagerRole;
import helios.engine.runtime.world.types;

import helios.engine.util.log;

import helios.engine.runtime.messaging.command.CommandHandlerRegistry;

#define HELIOS_LOG_SCOPE "helios::gameplay::spawning::SpawnManager"
export namespace helios::gameplay::spawning {

    template<typename TEntityPoolRegistry>
    class SpawnManager;

    template<
        template<typename> typename TLookupStrategy,
        typename ...TMemberHandles
    >
    class SpawnManager<engine::runtime::pooling::TypedEntityPoolRegistry<TLookupStrategy, TMemberHandles...>> {


        using UpdateContext = engine::runtime::world::UpdateContext;
        using LogManager = engine::util::log::LogManager;
        using CommandHandlerRegistry = engine::runtime::messaging::command::CommandHandlerRegistry;
        using EngineEntityPoolRegistry = engine::runtime::pooling::TypedEntityPoolRegistry<TLookupStrategy, TMemberHandles...>;
        using EngineSpawnPolicyRegistry = spawning::TypedSpawnPolicyRegistry<TMemberHandles...>;

        static inline auto& logger_ = LogManager::loggerForScope(HELIOS_LOG_SCOPE);


        EngineEntityPoolRegistry& entityPoolRegistry_;

        EngineSpawnPolicyRegistry& spawnPolicyRegistry_;

        template<typename TEmitterHandle, typename TSpawnHandle>
        struct SpawnContextSlot {
            engine::runtime::pooling::types::EntityPoolKey<TSpawnHandle> entityPoolKey;
            types::SpawnContext<TEmitterHandle, TSpawnHandle> spawnContext;
        };

        template<typename TEmitterHandle, typename TSpawnHandle>
        struct SpawnPolicySlot {
            types::SpawnPolicyKey<TEmitterHandle, TSpawnHandle> spawnPolicyKey;
            std::vector<engine::runtime::pooling::types::EntityPoolKey<TSpawnHandle>> entityPoolKeys;
            std::vector<SpawnContextSlot<TEmitterHandle, TSpawnHandle>> spawnContextSlots;
        };

        template<typename TEmitterHandle, typename TSpawnHandle>
        using SpawnPolicySlots = std::vector<std::optional<SpawnPolicySlot<TEmitterHandle, TSpawnHandle>>>;

        template<typename TEmitterHandle, typename ... TSpawnHandle>
        using SpawnPolicySinkForHandlePair = std::tuple<SpawnPolicySlots<TEmitterHandle, TSpawnHandle> ...>;

        template<typename ... THandles>
        using SpawnPolicySlotVectors = decltype(std::tuple_cat(
            std::declval<SpawnPolicySinkForHandlePair<THandles, THandles...>>()...)
        );

        SpawnPolicySlotVectors<TMemberHandles...> spawnPolicySlotVectors_;

        template<typename TEmitterHandle, typename ...TSpawnHandle>
        using SpawnCommandSinkForHandlePair = std::tuple<
            std::vector<commands::SpawnCommand<TEmitterHandle, TSpawnHandle>> ...
        >;

        template<typename ... THandles>
        using SpawnCommandVectors = decltype(std::tuple_cat(
            std::declval<SpawnCommandSinkForHandlePair<THandles, THandles...>>()...)
        );

        SpawnCommandVectors<TMemberHandles...> spawnCommandVectors_;


        template<typename ... THandles>
        void registerAllCommands(CommandHandlerRegistry& commandHandlerRegistry) {
            (registerCommandsForEmitter<THandles, THandles...>(commandHandlerRegistry), ...);
        }

        template<typename TEmitterHandle, typename ... TSpawnHandle>
        void registerCommandsForEmitter(CommandHandlerRegistry& commandHandlerRegistry) {
            (commandHandlerRegistry.registerHandler<
                spawning::commands::SpawnCommand<TEmitterHandle, TSpawnHandle>
            >(*this), ...);
        }

        template<typename TEmitterHandle, typename TSpawnHandle>
        auto& spawnPolicyVector() {
            return std::get<SpawnPolicySlots<TEmitterHandle, TSpawnHandle>>(spawnPolicySlotVectors_);
        }

        template<typename TEmitterHandle, typename TSpawnHandle>
        auto& spawnCommandVector() {
            return std::get<std::vector<commands::SpawnCommand<TEmitterHandle, TSpawnHandle>>>(spawnCommandVectors_);
        }


        template<typename TEmitterHandle, typename TSpawnHandle>
        void flushSpawnCommands(UpdateContext& updateContext,
            std::vector<commands::SpawnCommand<TEmitterHandle, TSpawnHandle>>& spawnCommands) {

            for (auto& spawnCommand: spawnCommands) {

                auto spawnContext = types::SpawnContext<TEmitterHandle, TSpawnHandle>{
                    .requiredAmount = spawnCommand.amount};

                auto* spawnPolicy = spawnPolicyRegistry_.item(spawnCommand.spawnPolicyKey.typeId);

                if (!spawnPolicy) {
                    logger_.warn("SpawnPolicy with typeId.value {0} missing", spawnCommand.spawnPolicyKey.typeId.value());
                    assert(false && "SpawnPolicy missing");
                    continue;
                }

                auto entityPoolKey = spawnCommand.entityPoolKey;

                auto* pool = entityPoolRegistry_.template pool<TSpawnHandle>(entityPoolKey);
                if (!pool) {
                    logger_.warn("pool not found!");
                    assert(false && "pool not found");
                    continue;
                }

                processSpawnPolicy(spawnPolicy, updateContext, spawnContext, pool, spawnCommand.emitterHandle);
            }

        }

        template<typename TEmitterHandle, typename TSpawnHandle>
        bool processSpawnPolicy(
            auto* spawnPolicy, UpdateContext& updateContext,
            types::SpawnContext<TEmitterHandle, TSpawnHandle>& spawnContext, auto* pool, TEmitterHandle emitterHandle) noexcept {

            spawnContext.poolSnapshot = engine::runtime::pooling::types::PoolSnapshot{pool->activeCount(), pool->inactiveCount()};

            if (!spawnPolicy->update(updateContext, spawnContext)) {
                return false;
            }

            if (auto amount = spawnPolicy->spawnCount(updateContext, spawnContext); amount > 0) {
                std::vector<TSpawnHandle> spawnHandles;
                TSpawnHandle spawnHandle{};
                std::size_t spawned = 0;
                while (spawned < amount && pool->acquire(spawnHandle)) {
                    ++spawned;
                    spawnHandles.push_back(spawnHandle);
                }

                std::size_t used = spawnPolicy->spawn(updateContext, spawnContext, spawnHandles);
                assert(used <= spawned && "used must be less than or equal to spawned");
                spawnContext.spawnedAmount += used;
                if (used != spawned) {
                    const auto bound = spawned - used;
                    auto i = 0;
                    for (auto& value : std::views::reverse(spawnHandles)) {
                        pool->release(value);
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

        using EngineRoleTag = engine::runtime::world::tags::ManagerRole;

        explicit SpawnManager(
            EngineSpawnPolicyRegistry& spawnPolicyRegistry,
            EngineEntityPoolRegistry& entityPoolRegistry)
        : spawnPolicyRegistry_(spawnPolicyRegistry), entityPoolRegistry_(entityPoolRegistry) {}


        template<typename TOwnerHandle, typename TSpawnHandle = TOwnerHandle>
        bool submit(commands::SpawnCommand<TOwnerHandle, TSpawnHandle>&& spawnCommand) noexcept {
            auto& vec = spawnCommandVector<TOwnerHandle, TSpawnHandle>();
            vec.push_back(std::move(spawnCommand));

            return true;
        }

        void init(engine::runtime::messaging::command::CommandHandlerRegistry& commandHandlerRegistry) noexcept {
            registerAllCommands<TMemberHandles...>(commandHandlerRegistry);
        }

        void flush(engine::runtime::world::UpdateContext& updateContext) noexcept {

            std::apply([&updateContext, this](auto& ... spawnCommands) {
                (flushSpawnCommands(updateContext, spawnCommands), ...);
            }, spawnCommandVectors_);

            std::apply([](auto& ...spawnCommands) noexcept {
                (spawnCommands.clear(), ...);
            }, spawnCommandVectors_);
        }


    };

}