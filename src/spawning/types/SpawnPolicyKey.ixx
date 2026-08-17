/**
 * @file SpawnPolicyKey.ixx
 * @brief Composite key identifying a spawn policy by type and instance ID.
 */
module;

#include <cstddef>

export module helios.gameplay.spawning.types:SpawnPolicyKey;

import helios.core.container;

import :SpawnPolicyTypeId;
import :SpawnPolicyId;

export namespace helios::gameplay::spawning::types {

    /**
     * @brief Composite key that identifies a spawn policy.
     *
     * Combines a compile-time `SpawnPolicyTypeId` with a runtime `SpawnPolicyId`
     * to distinguish both the policy type and the specific registered instance.
     *
     * @tparam TEmitterHandle Handle type of the entity that emits spawned objects.
     * @tparam TSpawnHandle Handle type of the spawned entities.
     */
    template<typename TEmitterHandle, typename TSpawnHandle = TEmitterHandle>
    class SpawnPolicyKey {

        /**
         * @brief Internal key type used by concept-model collections.
         */
        using ConceptModelKey = helios::core::container::types::ConceptModelCollectionKey<SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle>>;

        /**
         * @brief Identifies the concrete spawn policy type.
         */
        SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle> typeId_{helios::core::common::types::no_init};

        /**
         * @brief Index within the per-type collection.
         */
        std::size_t index_;

        /**
         * @brief Identifies the specific spawn policy instance.
         */
        SpawnPolicyId<TEmitterHandle, TSpawnHandle> spawnPolicyId_;

        /**
         * @brief Indicates whether this key was initialized through the explicit constructor.
         */
        bool isValid_ = false;

    public:
        /**
         * @brief Constructs an invalid key.
         */
        SpawnPolicyKey() = default;

        /**
         * @brief Constructs a valid key from type, collection index, and runtime ID.
         *
         * @param typeId Compile-time policy type identifier.
         * @param index Per-type index in the backing collection.
         * @param spawnPolicyId Runtime identifier of the policy instance.
         */
        explicit SpawnPolicyKey(
            SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle> typeId,
            const std::size_t index,
            SpawnPolicyId<TEmitterHandle, TSpawnHandle> spawnPolicyId) :
            index_(index), typeId_(typeId), spawnPolicyId_(spawnPolicyId), isValid_(true) {}

        /**
         * @brief Converts this key into a concept-model collection key.
         *
         * @return Collection key composed of `typeId` and `index`.
         */
        [[nodiscard]] ConceptModelKey toConceptModelCollectionKey() const noexcept {
            return {typeId_, index_};
        }

        /**
         * @brief Returns whether this key is valid.
         *
         * @return `true` if this key was explicitly initialized, otherwise `false`.
         */
        [[nodiscard]] bool isValid() const noexcept {
            return isValid_;
        }

        /**
         * @brief Returns the spawn policy type identifier.
         *
         * @return Reference to the type identifier.
         */
        [[nodiscard]] const SpawnPolicyTypeId<TEmitterHandle, TSpawnHandle>& typeId() const noexcept {
            return typeId_;
        }

        /**
         * @brief Returns the runtime spawn policy identifier.
         *
         * @return Reference to the runtime identifier.
         */
        [[nodiscard]] const SpawnPolicyId<TEmitterHandle, TSpawnHandle>& spawnPolicyId() const noexcept {
            return spawnPolicyId_;
        }

        /**
         * @brief Returns the index within the per-type policy collection.
         *
         * @return Zero-based collection index.
         */
        [[nodiscard]] std::size_t index() const noexcept {
            return index_;
        }


    };

}