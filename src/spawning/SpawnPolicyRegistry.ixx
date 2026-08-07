/**
 * @file SpawnPolicyRegistry.ixx
 * @brief Registry mapping spawn policy types to their implementations.
 */
module;

export module helios.gameplay.spawning.SpawnPolicyRegistry;


import helios.gameplay.spawning.SpawnPolicy;
import helios.gameplay.spawning.types;
import helios.engine.core.container.ConceptModelRegistry;


export namespace helios::gameplay::spawning {

    /**
     * @brief `ConceptModelRegistry` specialisation that stores `SpawnPolicy` instances
     *        keyed by `SpawnPolicyTypeId`.
     *
     * @tparam TEmitterHandle  Handle type of the emitting entity.
     * @tparam TSpawnHandle    Handle type of the spawned entities.
     */
    template<typename TEmitterHandle, typename TSpawnHandle>
    using SpawnPolicyRegistry = helios::engine::core::container::ConceptModelRegistry<
        SpawnPolicy<TEmitterHandle, TSpawnHandle>, types::SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle>
    >;

}