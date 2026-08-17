/**
 * @file SpawnPolicyId.ixx
 * @brief Strongly-typed identifier for spawn policies.
 */
module;


export module helios.gameplay.spawning.types:SpawnPolicyId;

import helios.core.common.types;

export namespace helios::gameplay::spawning::types {

    template<typename TEmitterHandle, typename TSpawnHandle = TEmitterHandle>
    struct SpawnPolicyIdTag;

    /**
     * @brief Strongly-typed string identifier for a spawn policy.
     *
     * @tparam TEmitterHandle  Handle type of the entity that emits spawned objects.
     * @tparam TSpawnHandle    Handle type of the spawned entities.
     */
    template<typename TEmitterHandle, typename TSpawnHandle = TEmitterHandle>
    using SpawnPolicyId = helios::core::common::types::StrongId<SpawnPolicyIdTag<TEmitterHandle, TSpawnHandle>>;

}