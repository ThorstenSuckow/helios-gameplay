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

    using SpawnPolicyRegistry = helios::core::common::container::ConceptModelInstanceCollectionRegistry<
        SpawnPolicy, types::SpawnPolicyTypeId
    >;

}