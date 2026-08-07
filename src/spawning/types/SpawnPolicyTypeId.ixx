/**
 * @file SpawnPolicyTypeId.ixx
 * @brief Unique type identifier for Spawn Policy resources.
 */
module;

#include <functional>
#include <cstddef>

export module helios.gameplay.spawning.types:SpawnPolicyTypeId;

import helios.ecs.TypeIndexer;
import helios.ecs.types;

export namespace helios::gameplay::spawning::types {

    /**
     * @brief Unique type identifier for SpawnPolicy resources.
     *
     * All resource categories (Managers, CommandBuffers, CommandHandlers)
     * share a single index space.
     *
     * @see ResourceRegistry
     * @see TypeIndexer
     * @see ComponentTypeId
     * @see CommandTypeId
     */
    template<typename TEmitterHandle, typename TSpawnHandle = TEmitterHandle>
    class SpawnPolicyTypeId {

        /**
         * @brief Tag type for the TypeIndexer domain.
         */
        struct helios_engine_common_tag_SpawnTypes{};

        using ResourceType = helios_engine_common_tag_SpawnTypes;

        /**
         * @brief The underlying ID value.
         */
        size_t id_{0};


    public:


        /**
         * @brief Constructs a SpawnPolicyTypeId with a specific value.
         *
         * @param id The type ID value.
         */
        explicit SpawnPolicyTypeId(const size_t id) : id_(id) {}


        /**
         * @brief Constructs an uninitialized SpawnPolicyTypeId.
         *
         * @param no_init_t Tag to indicate no initialization.
         */
        explicit SpawnPolicyTypeId(helios::ecs::types::no_init_t) {}


        /**
         * @brief Returns the underlying ID value.
         *
         * @return The numeric type ID, suitable for use as an array index.
         */
        [[nodiscard]] size_t value() const noexcept {
            return id_;
        }


        /**
         * @brief Returns the SpawnPolicyTypeId for a specific type.
         *
         * @details Uses TypeIndexer to generate a unique ID per type.
         * The ID is generated once and cached.
         *
         * @tparam T The resource type.
         *
         * @return The unique SpawnPolicyTypeId for type T.
         */
        template <typename T>
        requires std::same_as<TEmitterHandle, typename T::EmitterHandle_type>
        && std::same_as<TSpawnHandle, typename T::SpawnHandle_type>
        [[nodiscard]] static SpawnPolicyTypeId id() {
            static const size_t tid = helios::ecs::TypeIndexer<ResourceType>::template typeIndex<T>();
            return SpawnPolicyTypeId(tid);
        }

        friend constexpr bool operator==(SpawnPolicyTypeId, SpawnPolicyTypeId) noexcept = default;
    };


}


/**
 * @brief Hash specialization for SpawnPolicyTypeId.
 */
template<typename TEmitterHandle, typename TSpawnHandle>
struct std::hash<helios::gameplay::spawning::types::SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle>> {
   std::size_t operator()(const helios::gameplay::spawning::types::SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle>& id) const noexcept {
        return id.value();
    }

};