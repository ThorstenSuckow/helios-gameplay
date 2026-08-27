/**
 * @file SpawnPolicyTypeId.ixx
 * @brief Unique type identifier for Spawn Policy resources.
 */
module;

export module helios.gameplay.spawning.types:SpawnPolicyTypeId;

import helios.core.common;

export namespace helios::gameplay::spawning::types {

    struct SpawnTypesDomainTag{};

    using SpawnPolicyTypeId = core::common::types::TypeId<SpawnTypesDomainTag>;

};