/**
 * @file TypedSpawnPolicyRegistry.ixx
 * @brief Aggregated registry holding one SpawnPolicyRegistry per emitter/spawn handle pair.
 */
module;

#include <tuple>
#include <utility>

export module helios.gameplay.spawning.TypedSpawnPolicyRegistry;

import helios.gameplay.spawning.concepts;
import helios.gameplay.spawning.SpawnPolicy;
import helios.gameplay.spawning.SpawnPolicyRegistry;

import helios.gameplay.spawning.types;

import helios.engine.core.container.ConceptModelRegistry;


export namespace helios::gameplay::spawning {

    /**
     * @brief Holds a `SpawnPolicyRegistry` for every `(TEmitter, TSpawn)` pair derivable from `TMemberHandles`.
     *
     * @tparam TMemberHandles  Pack of all handle types that may act as emitters or spawn targets.
     */
    template<typename ... TMemberHandles>
    class TypedSpawnPolicyRegistry {

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

        /**
         * @brief Storage for all registries, indexed by type via `std::get`.
         */
        AllRegistries<TMemberHandles...> registries_;

    public:

        /**
         * @brief Returns the mutable `SpawnPolicyRegistry` for the given emitter/spawn handle pair.
         *
         * @tparam TEmitterHandle  Emitter handle type.
         * @tparam TSpawnHandle    Spawn handle type.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        SpawnPolicyRegistry<TEmitterHandle, TSpawnHandle>& registry() noexcept {
            return std::get<SpawnPolicyRegistry<TEmitterHandle, TSpawnHandle>>(registries_);
        }

        /**
         * @brief Returns the const `SpawnPolicyRegistry` for the given emitter/spawn handle pair.
         *
         * @tparam TEmitterHandle  Emitter handle type.
         * @tparam TSpawnHandle    Spawn handle type.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        const SpawnPolicyRegistry<TEmitterHandle, TSpawnHandle>& registry() const noexcept {
            return std::get<SpawnPolicyRegistry<TEmitterHandle, TSpawnHandle>>(registries_);
        }

        /**
         * @brief Returns all registered policy items for the given handle pair (const).
         *
         * @tparam TEmitterHandle  Emitter handle type.
         * @tparam TSpawnHandle    Spawn handle type.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        [[nodiscard]] auto items() const noexcept {
            return registry<TEmitterHandle, TSpawnHandle>().items();
        }

        /**
         * @brief Returns all registered policy items for the given handle pair.
         *
         * @tparam TEmitterHandle  Emitter handle type.
         * @tparam TSpawnHandle    Spawn handle type.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        [[nodiscard]] auto items() noexcept {
            return registry<TEmitterHandle, TSpawnHandle>().items();
        }


        /**
         * @brief Constructs and registers a `TSpawnPolicy` in the matching registry.
         *
         * @tparam TSpawnPolicy  Concrete policy type satisfying `IsSpawnPolicyLike`.
         * @tparam Args          Constructor argument types.
         * @param args           Arguments forwarded to the policy constructor.
         *
         * @return Reference to the newly added policy.
         */
        template<typename TSpawnPolicy, typename... Args>
        requires concepts::IsSpawnPolicyLike<TSpawnPolicy>
        TSpawnPolicy& add(Args&&... args) {
            using EmitterType = typename TSpawnPolicy::EmitterHandle_type;
            using SpawnType = typename TSpawnPolicy::SpawnHandle_type;

            return registry<EmitterType, SpawnType>().template add<TSpawnPolicy>(std::forward<Args>(args)...);
         }


        /**
         * @brief Registers a `TSpawnPolicy` from an existing `SpawnPolicy` wrapper.
         *
         * @tparam TSpawnPolicy  Concrete policy type satisfying `IsSpawnPolicyLike`.
         * @param wrapper        Rvalue `SpawnPolicy` wrapper to move into the registry.
         *
         * @return Reference to the registered policy.
         */
        template<typename TSpawnPolicy>
        requires concepts::IsSpawnPolicyLike<TSpawnPolicy>
        TSpawnPolicy& add(
            SpawnPolicy<
                typename TSpawnPolicy::EmitterHandle_type,
                typename TSpawnPolicy::SpawnHandle_type
            >&& wrapper) {
            using EmitterType = typename TSpawnPolicy::EmitterHandle_type;
            using SpawnType = typename TSpawnPolicy::SpawnHandle_type;

            return registry<EmitterType, SpawnType>().template add<TSpawnPolicy>(std::move(wrapper));
        }


        /**
         * @brief Returns `true` if a policy of type `TSpawnPolicy` is registered.
         *
         * @tparam TSpawnPolicy  Concrete policy type satisfying `IsSpawnPolicyLike`.
         */
        template<typename TSpawnPolicy>
        requires concepts::IsSpawnPolicyLike<TSpawnPolicy>
        [[nodiscard]] bool has() const {
            using EmitterType = typename TSpawnPolicy::EmitterHandle_type;
            using SpawnType = typename TSpawnPolicy::SpawnHandle_type;

            return registry<EmitterType, SpawnType>().template has<TSpawnPolicy>();
        }


        /**
         * @brief Returns a pointer to the registered `TSpawnPolicy`, or `nullptr` if absent.
         *
         * @tparam TSpawnPolicy  Concrete policy type satisfying `IsSpawnPolicyLike`.
         */
        template<typename TSpawnPolicy>
        requires concepts::IsSpawnPolicyLike<TSpawnPolicy>
        [[nodiscard]] TSpawnPolicy* item() const {
            using EmitterType = typename TSpawnPolicy::EmitterHandle_type;
            using SpawnType = typename TSpawnPolicy::SpawnHandle_type;

            return registry<EmitterType, SpawnType>().template item<TSpawnPolicy>();
        }

        template<typename TSpawnPolicy>
        requires concepts::IsSpawnPolicyLike<TSpawnPolicy>
        [[nodiscard]] const TSpawnPolicy* item() const {
            using EmitterType = typename TSpawnPolicy::EmitterHandle_type;
            using SpawnType = typename TSpawnPolicy::SpawnHandle_type;

            return registry<EmitterType, SpawnType>().template item<TSpawnPolicy>();
        }


        /**
         * @brief Returns a policy by its `SpawnPolicyTypeId`, or `nullptr` if not found.
         *
         * @tparam TEmitterHandle  Emitter handle type.
         * @tparam TSpawnHandle    Spawn handle type.
         * @param typeId           Runtime type identifier of the policy.
         */
        template<typename TEmitterHandle, typename TSpawnHandle>
        [[nodiscard]] auto* item(types::SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle> typeId) noexcept {
            return registry<TEmitterHandle, TSpawnHandle>().item(typeId);
        }


    };

}