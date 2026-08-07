module;

#include <cstddef>
#include <span>

export module helios.gameplay.spawning.concepts:IsSpawnPolicyLike;

import helios.engine.runtime.world.UpdateContext;
import helios.gameplay.spawning.types;

export namespace helios::gameplay::spawning::concepts {


    template<typename T>
    concept IsSpawnPolicyLike = requires
    {
        typename T::EmitterHandle_type;
        typename T::SpawnHandle_type;
    } && requires(
        T& t,
        engine::runtime::world::UpdateContext& updateContext,
        types::SpawnContext<typename T::EmitterHandle_type, typename T::SpawnHandle_type>& spawnContext,
        std::span<const typename T::SpawnHandle_type> spawnHandles
    )
    {
        {t.spawnCount(updateContext, spawnContext)} -> std::same_as<std::size_t>;
        {t.spawn(updateContext, spawnContext, spawnHandles)} -> std::same_as<std::size_t>;
        {t.update(updateContext, spawnContext)} -> std::same_as<bool>;
    };


}