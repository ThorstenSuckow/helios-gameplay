module;

#include <cstddef>

export module helios.gameplay.spawning.components:SpawnRequestComponent;

import helios.engine.runtime.pooling.types;
import helios.gameplay.spawning.types;

export namespace helios::gameplay::spawning::components {

    template<typename TOwnerHandle, typename TSpawnHandle = TOwnerHandle>
    struct SpawnRequestComponent {

        using Handle_type = TOwnerHandle;

        engine::runtime::pooling::types::EntityPoolKey<TSpawnHandle> entityPoolKey;

        types::SpawnPolicyKey<TOwnerHandle, TSpawnHandle> spawnPolicyKey;

        std::size_t amount;
    };


}