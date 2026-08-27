/**
 * @file SpawnPolicyKey.ixx
 * @brief Composite key identifying a spawn policy by type and instance ID.
 */
module;


export module helios.gameplay.spawning.types:SpawnPolicyKey;

import helios.core.common.container;

import :SpawnPolicyTypeId;


export namespace helios::gameplay::spawning::types {

    using SpawnPolicyKey = helios::core::common::container::types::ConceptModelCollectionKey<SpawnPolicyTypeId>;

}