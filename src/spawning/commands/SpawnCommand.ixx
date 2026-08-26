module;

#include <cstddef>

export module helios.gameplay.spawning.commands:SpawnCommand;

import helios.engine.runtime.pooling.types;
import helios.gameplay.spawning.types;

export namespace helios::gameplay::spawning::commands {

    template<typename TEmitterHandle, typename TSpawnHandle = TEmitterHandle>
    struct SpawnCommand {

        using SpawnHandleType = TSpawnHandle;
        using EmitterHandleType = TEmitterHandle;

        TEmitterHandle emitterHandle;

        engine::runtime::pooling::types::EntityPoolKey<TSpawnHandle> entityPoolKey;

        types::SpawnPolicyKey<TEmitterHandle, TSpawnHandle> spawnPolicyKey;

        std::size_t amount;

    };

}