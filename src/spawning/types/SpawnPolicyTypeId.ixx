/**
 * @file SpawnPolicyTypeId.ixx
 * @brief Unique type identifier for Spawn Policy resources.
 */
module;

export module helios.gameplay.spawning.types:SpawnPolicyTypeId;

import helios.core.common;

export namespace helios::gameplay::spawning::types {
    template <typename TEmitterHandle, typename TSpawnHandle>
    struct SpawnTypesDomainTag{};

    /**
     * @brief Unique type identifier for SpawnPolicy resources.
     */
    template<typename TEmitterHandle, typename TSpawnHandle = TEmitterHandle>
    using SpawnPolicyTypeId = core::common::TypeId<SpawnTypesDomainTag<TEmitterHandle, TSpawnHandle>>;

};