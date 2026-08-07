/**
 * @file SpawnPolicyKey.ixx
 * @brief Composite key identifying a spawn policy by type and instance ID.
 */
module;


export module helios.gameplay.spawning.types:SpawnPolicyKey;

import :SpawnPolicyTypeId;
import :SpawnPolicyId;

export namespace helios::gameplay::spawning::types {

    /**
     * @brief Composite key that identifies a spawn policy.
     *
     * Combines a compile-time `SpawnPolicyTypeId` with a runtime `SpawnPolicyId`
     * to distinguish both the policy type and the specific registered instance.
     *
     * @tparam TEmitterHandle  Handle type of the entity that emits spawned objects.
     * @tparam TSpawnHandle    Handle type of the spawned entities.
     */
    template<typename TEmitterHandle, typename TSpawnHandle = TEmitterHandle>
    struct SpawnPolicyKey {

        /** @brief Identifies the concrete spawn policy type. */
        SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle> typeId;

        /** @brief Identifies the specific spawn policy instance. */
        SpawnPolicyId<TEmitterHandle, TSpawnHandle> id;

    };

}