/**
 * @file TypedSpawnPolicyRegistry.ixx
 * @brief Aggregated registry holding one SpawnPolicyRegistry per emitter/spawn handle pair.
 */
module;

#include <tuple>
#include <utility>
#include <cassert>

export module helios.gameplay.spawning.TypedSpawnPolicyRegistry;

import helios.core.concepts;
import helios.core.log;
import helios.core.common.types;

import helios.gameplay.spawning.types;
import helios.gameplay.spawning.concepts;
import helios.gameplay.spawning.SpawnPolicy;
import helios.gameplay.spawning.SpawnPolicyRegistry;

import helios.gameplay.spawning.types;

import helios.ecs.common.concepts;

import helios.core.container;

#define HELIOS_LOG_SCOPE "helios::gameplay::spawning::TypedSpawnPolicyRegistry"
export namespace helios::gameplay::spawning {

    /**
     * @brief Holds a `SpawnPolicyRegistry` for every `(TEmitter, TSpawn)` pair derivable from `TMemberHandles`.
     *
     * @tparam TStrongIdLookupStrategy Strategy template used to track unique runtime policy IDs per pair.
     * @tparam TMemberHandles Pack of all handle types that may act as emitters or spawn targets.
     */
    template<
        template<typename> typename TStrongIdLookupStrategy,
        typename ... TMemberHandles
    >
    requires (helios::core::concepts::IsStrongIdCollisionResolverLike<TStrongIdLookupStrategy<TMemberHandles>> && ...)
        && (sizeof ...(TMemberHandles) > 0)
    class TypedSpawnPolicyRegistry {
        
        /**
         * @brief Tag type used to create one lookup strategy instance per `(emitter, spawn)` pair.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        struct StrongIdLookupStrategyTag {};
        
        /**
         * @brief Tuple of lookup strategies for one emitter against all spawn handle types.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type pack.
         */
        template<typename TEmitterHandle, typename ... TSpawnHandle>
        using StrongIdLookupStrategiesForEmitter = std::tuple<
            TStrongIdLookupStrategy<StrongIdLookupStrategyTag<TEmitterHandle, TSpawnHandle>>...
        >;
        
        /**
         * @brief Flat tuple type containing all lookup strategy instances across handle combinations.
         *
         * @tparam THandles Handle type pack.
         */
        template<typename ... THandles>
        using AllStrongIdLookupStrategies = decltype(std::tuple_cat(
            std::declval<StrongIdLookupStrategiesForEmitter<THandles, THandles...>>()...
        ));
        
        /**
         * @brief Runtime storage for all lookup strategy instances.
         */
        AllStrongIdLookupStrategies<TMemberHandles...> strongIdLookupStrategies_;
        
        /**
         * @brief Returns the lookup strategy assigned to one `(emitter, spawn)` pair.
         *
         * @tparam TEmitter Emitter handle type.
         * @tparam TSpawn Spawn handle type.
         * @return Reference to the corresponding lookup strategy.
         */
        template<typename TEmitter, typename TSpawn>
        TStrongIdLookupStrategy<StrongIdLookupStrategyTag<TEmitter, TSpawn>>& strongIdLookupStrategy() noexcept {
            return std::get<TStrongIdLookupStrategy<StrongIdLookupStrategyTag<TEmitter, TSpawn>>>(strongIdLookupStrategies_);
        }

        static inline auto& logger_ = helios::core::log::LogManager::loggerForScope(HELIOS_LOG_SCOPE);
        
        /**
         * @brief Tuple type holding one `SpawnPolicyRegistry` per `(TEmitterHandle, TSpawnHandle)` combination.
         */
        template<typename TEmitterHandle, typename... TSpawnHandle>
        using RegistryForSource = std::tuple<
            SpawnPolicyRegistry<TEmitterHandle, TSpawnHandle>...
        >;

        /**
         * @brief Flat tuple of all `SpawnPolicyRegistry` instances across every handle combination.
         */
        template<typename ...THandles>
        using AllRegistries = decltype(
                std::tuple_cat(
                    std::declval<RegistryForSource<THandles, THandles...>>()...)
        );

        template<typename TEmitterHandle, typename TSpawnHandle>
        using ConceptModelCollectionKey = core::container::types::ConceptModelCollectionKey<types::SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle>>;

        /**
         * @brief Storage for all registries, indexed by type via `std::get`.
         */
        AllRegistries<TMemberHandles...> registries_;

    public:

        /**
         * @brief Returns the mutable `SpawnPolicyRegistry` for the given emitter/spawn handle pair.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @return Reference to the matching registry.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        SpawnPolicyRegistry<TEmitterHandle, TSpawnHandle>& registry() noexcept {
            return std::get<SpawnPolicyRegistry<TEmitterHandle, TSpawnHandle>>(registries_);
        }

        /**
         * @brief Returns the const `SpawnPolicyRegistry` for the given emitter/spawn handle pair.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @return Const reference to the matching registry.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        const SpawnPolicyRegistry<TEmitterHandle, TSpawnHandle>& registry() const noexcept {
            return std::get<SpawnPolicyRegistry<TEmitterHandle, TSpawnHandle>>(registries_);
        }

        /**
         * @brief Returns all registered policy collections for the given handle pair (const).
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @return Const grouped view of policy wrappers.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        [[nodiscard]] auto collections() const noexcept {
            return registry<TEmitterHandle, TSpawnHandle>().collections();
        }

        /**
         * @brief Returns all registered policy collections for the given handle pair.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @return Mutable grouped view of policy wrappers.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        [[nodiscard]] auto collections() noexcept {
            return registry<TEmitterHandle, TSpawnHandle>().collections();
        }


        /**
         * @brief Constructs and registers a `TSpawnPolicy` in the matching registry.
         *
         * @tparam TSpawnPolicy Concrete policy type satisfying `IsSpawnPolicyLike`.
         * @tparam TSpawnPolicyId Runtime strong-id type used to identify one policy instance.
         * @tparam Args Constructor argument types.
         * @param spawnPolicyId Runtime policy ID.
         * @param args Arguments forwarded to the policy constructor.
         * @return Valid `SpawnPolicyKey` on success; invalid key on duplicate ID.
         */
        template<typename TSpawnPolicy, typename TSpawnPolicyId, typename... Args>
        requires concepts::IsSpawnPolicyLike<TSpawnPolicy>
        auto createPolicy(TSpawnPolicyId spawnPolicyId, Args&&... args) {
            using EmitterType = typename TSpawnPolicy::EmitterHandle_type;
            using SpawnType = typename TSpawnPolicy::SpawnHandle_type;

            auto& strongIdLookupStrat = strongIdLookupStrategy<EmitterType, SpawnType>();

            if (strongIdLookupStrat.has(spawnPolicyId.value())) {
                logger_.error("Spawn policy ID already exists for the given emitter/spawn handle pair.");
                assert(false && "Spawn policy ID already exists for the given emitter/spawn handle pair.");
                return types::SpawnPolicyKey<EmitterType, SpawnType>();
            }
            
            strongIdLookupStrat.add(spawnPolicyId.value());

            auto key = registry<EmitterType, SpawnType>().template add<TSpawnPolicy>(std::forward<Args>(args)...);

            return types::SpawnPolicyKey<EmitterType, SpawnType>(
                key.typeId,
                key.index,
                spawnPolicyId
            );
        }

        /**
         * @brief Returns the registered policy wrapper addressed by `SpawnPolicyKey`.
         *
         * @detail Additionally checks if the runtime id is existing with this registry, which might produces
         * unnecessary overhead. Use item(ConceptModelCollectionKey) instead.
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @param key Composite spawn policy key.
         * @return Pointer to the stored wrapper, or `nullptr` if validation fails.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        auto* item(const types::SpawnPolicyKey<TEmitterHandle, TSpawnHandle> key) noexcept {

            if (!key.isValid()) {
                logger_.warn("SpawnPolicyKey is invalid");
                assert(false && "SpawnPolicyKey is invalid");
                return nullptr;
            }

            if (auto& lookupStrategy = strongIdLookupStrategy<TEmitterHandle, TSpawnHandle>();
                !lookupStrategy.has(key.typeId().value())) {
                logger_.warn("SpawnPolicyKey with this strong ID not registered.");
                assert(false && "SpawnPolicyKey with this strong ID not registered.");
                return nullptr;
            }

            return registry<TEmitterHandle, TSpawnHandle>().item(
                ConceptModelCollectionKey<TEmitterHandle, TSpawnHandle>{key.typeId(), key.index()
            });
        }

        /**
         * @brief Returns the registered policy wrapper addressed by concept-model collection key.
         *
         * @details If further runtime identification is required, use item(SpawnPolicyKey) instead
         *
         * @tparam TEmitterHandle Emitter handle type.
         * @tparam TSpawnHandle Spawn handle type.
         * @param key Type-id and index key from the underlying collection.
         * @return Pointer to the stored wrapper, or `nullptr` if not found.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        auto* item(const ConceptModelCollectionKey<TEmitterHandle, TSpawnHandle> key) noexcept {

            return registry<TEmitterHandle, TSpawnHandle>().item(key);
        }


    };

}