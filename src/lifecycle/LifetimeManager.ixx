module;

#include <vector>

export module helios.gameplay.lifecycle:LifetimeManager;

import helios.gameplay.lifecycle.commands;
import helios.ecs.command;
import helios.ecs.entity;

export namespace helios::gameplay::lifecycle {

    template <typename THandle>
    class LifetimeManager {
    public:
        using LifetimeCommand = commands::LifetimeCommand<THandle>;
        using EntityManager = ecs::entity::EntityManager<THandle>;

    private:
        std::vector<LifetimeCommand> pendingCommands_;

    public:
        LifetimeManager() = default;

        bool init(ecs::command::CommandHandlerRegistry& commandHandlerRegistry) noexcept {
            commandHandlerRegistry.handleCommands<LifetimeCommand>(*this);
            return true;
        }

        void reset() noexcept {
            pendingCommands_.clear();
        }

        bool submit(LifetimeCommand&& command) noexcept {
            pendingCommands_.push_back(std::move(command));
            return true;
        }

        bool execute(EntityManager& entityManager) noexcept {
            bool result = true;

            for (const auto& command : pendingCommands_) {
                if (command.type != commands::LifetimeExpired) {
                    continue;
                }

                result = entityManager.destroy(command.handle) && result;
            }

            pendingCommands_.clear();
            return result;
        }
    };

}





