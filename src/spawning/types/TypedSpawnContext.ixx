module;

#include <cstddef>
#include <vector>
#include <optional>

export module helios.gameplay.spawning.types:TypedSpawnContext;

import helios.engine.runtime.pooling.types;
import helios.engine.spatial.types;

import helios.math;

import :SpawnPolicyKey;

export namespace helios::gameplay::spawning::types {

    /**
     * @brief Context information for spawning operations.
     */
    template<typename TEmitterHandle, typename TSpawnHandle>
    struct TypedSpawnContext {

        engine::spatial::types::SpatialSnapshot spatialSnapshot;

        engine::runtime::pooling::types::PoolSnapshot poolSnapshot{};

        std::size_t requiredAmount{};
        std::size_t spawnedAmount{};

        std::size_t frame{};
        float deltaTime{};

        TEmitterHandle emitterHandle{};
    };


}