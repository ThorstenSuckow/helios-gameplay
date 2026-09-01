module;

#include <cstddef>

export module helios.gameplay.spawning.components:SpawnRequestComponent;

import helios.engine.runtime.pooling.types;
import helios.gameplay.spawning.types;

export namespace helios::gameplay::spawning::components {

    template<typename TOwnerHandle, typename TSpawnHandle = TOwnerHandle>
    struct SpawnRequestComponent {

        using HandleType = TOwnerHandle;

        engine::runtime::pooling::types::EntityPoolKey entityPoolKey;

        types::SpawnPolicyKey spawnPolicyKey;

        std::size_t amount;
    };


}