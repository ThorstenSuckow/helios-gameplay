/**
 * @file SpawnPolicyRegistry.ixx
 * @brief Registry mapping spawn policy types to their implementations.
 */
module;

export module helios.gameplay.spawning.SpawnPolicyRegistry;


import helios.gameplay.spawning.SpawnPolicy;
import helios.gameplay.spawning.types;
import helios.core.common.container;


export namespace helios::gameplay::spawning {

    /**
     * @brief `ConceptModelInstanceCollectionRegistry` specialisation that stores `SpawnPolicy` instances in buckets indexed by
     * `SpawnPolicyTypeId`.
     *
     * @tparam TEmitterHandle  Handle type of the emitting entity.
     * @tparam TSpawnHandle    Handle type of the spawned entities.
     */
    template<typename TEmitterHandle, typename TSpawnHandle>
    using SpawnPolicyRegistry = helios::core::common::container::ConceptModelInstanceCollectionRegistry<
        SpawnPolicy<TEmitterHandle, TSpawnHandle>, types::SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle>
    >;

}