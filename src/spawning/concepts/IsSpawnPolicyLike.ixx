module;

#include <cstddef>
#include <span>

export module helios.gameplay.spawning.concepts:IsSpawnPolicyLike;

import helios.ecs.common.types;
import helios.gameplay.spawning.types;


export namespace helios::gameplay::spawning::concepts {


    template<typename T>
    concept IsSpawnPolicyLike = true;


}