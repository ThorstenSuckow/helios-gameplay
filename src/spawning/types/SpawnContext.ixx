module;

#include <cstddef>
#include <memory>
#include <vector>
#include <optional>
#include <cassert>
#include <exception>
#include <sys/stat.h>

export module helios.gameplay.spawning.types:SpawnContext;

import helios.engine.runtime.pooling.types;
import helios.engine.spatial.types;
import :TypedSpawnContext;
import helios.ecs.common.types;

import helios.math;

import :SpawnPolicyKey;

export namespace helios::gameplay::spawning::types {

    class SpawnContext {

        class Concept {

        public:
            virtual ~Concept() = default;

            [[nodiscard]] virtual void* get()  noexcept = 0;

            virtual void addToSpawnedAmount(std::size_t amount) noexcept = 0;

        };


        template<typename TEmitterHandle, typename TSpawnHandle>
        class Model final : public Concept {

        public:

            TypedSpawnContext<TEmitterHandle, TSpawnHandle> concreteSpawnContext_;

            Model(TypedSpawnContext<TEmitterHandle, TSpawnHandle> concreteSpawnContext) : concreteSpawnContext_(std::move(concreteSpawnContext)) {}

            void addToSpawnedAmount(std::size_t amount) noexcept override {
                concreteSpawnContext_.spawnedAmount += amount;
            }

            [[nodiscard]] void* get()  noexcept override {
                return static_cast<void*>(&concreteSpawnContext_);
            }

        };



        template<typename TEmitterHandle, typename TSpawnHandle>
        SpawnContext(TypedSpawnContext<TEmitterHandle, TSpawnHandle>&& concreteSpawnContext)
           : model_(std::make_unique<Model<TEmitterHandle, TSpawnHandle>>(std::move(concreteSpawnContext))),
            emitterHandleTypeId_(ecs::common::types::HandleTypeId::template id<TEmitterHandle>()),
            spawnHandleTypeId_(ecs::common::types::HandleTypeId::template id<TSpawnHandle>())
        {}


        std::unique_ptr<Concept> model_;

        ecs::common::types::HandleTypeId emitterHandleTypeId_;
        ecs::common::types::HandleTypeId spawnHandleTypeId_;

    public:


        template<typename TEmitterHandle, typename TSpawnHandle, typename ... TArgs>
        static SpawnContext make(TArgs&& ...args) {

            return SpawnContext(
                TypedSpawnContext<TEmitterHandle, TSpawnHandle>(std::forward<TArgs>(args)...)
            );
        }

        template<typename TEmitterHandle, typename TSpawnHandle>
        [[nodiscard]] TypedSpawnContext<TEmitterHandle, TSpawnHandle>& get() noexcept {
            if (ecs::common::types::HandleTypeId::template id<TEmitterHandle>() != emitterHandleTypeId_ ||
                ecs::common::types::HandleTypeId::template id<TSpawnHandle>() != spawnHandleTypeId_) {
                assert(false && "SpawnContext does not contain the requested handle types.");
                std::terminate();
            }

            return *static_cast<TypedSpawnContext<TEmitterHandle, TSpawnHandle>*>(model_->get());
        }

        void addToSpawnedAmount(std::size_t amount) noexcept {
            model_->addToSpawnedAmount(amount);
        }

    };


}