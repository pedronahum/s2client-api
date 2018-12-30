#include "sc2api/sc2_api.h"

#include "sc2utils/sc2_manage_process.h"

#include <iostream>

const char* kReplayFolder = "/StarCraft/StarCraftII/ReplaysTest/";

class Replay : public sc2::ReplayObserver {
public:
    std::vector<uint32_t> count_units_built_;
    uint32_t counter = 0; // per replay, keeps track of the number of players
    int32_t minerals = 0; // mineral count
    int32_t gas = 0; // gas count

    Replay() :
        sc2::ReplayObserver() {
    }

    void OnGameStart() final {
        const sc2::ObservationInterface* obs = Observation();
        assert(obs->GetUnitTypeData(true).size() > 0);
        count_units_built_.resize(obs->GetUnitTypeData(true).size());
        std::fill(count_units_built_.begin(), count_units_built_.end(), 0);

        // Get game info: map and race
        sc2::GameInfo game_info = obs->GetGameInfo(true);
        std::cout << std::endl;
        std::cout << "map_name: " << game_info.map_name << std::endl;
        std::cout << "player_id: " << obs->GetPlayerID() << std::endl;
        std::vector<sc2::PlayerInfo> player_information = game_info.player_info;

        for (uint32_t i = counter; i < player_information.size(); ++i){
            sc2::PlayerInfo player = player_information[i];
            sc2::Race race = player.race_actual;
            if(race == sc2::Race::Protoss) {
                std::cout << "player_id: " << player.player_id << "; race_actual: " << "Protoss" << std::endl;
            } else if(race == sc2::Race::Terran){
                std::cout << "player_id: " << player.player_id << "; race_actual: " <<"Terran" << std::endl;
            } else if(race == sc2::Race::Zerg){
                std::cout << "player_id: " << player.player_id << "; race_actual: " << "Zerg" << std::endl;
            } else {
                std::cout << "player_id: " << player.player_id << "; race_actual: " << "Random" << std::endl;
            }
        }
        std::cout << std::endl;

    }
    
    void OnUnitCreated(const sc2::Unit* unit) final {

        assert(uint32_t(unit->unit_type) < count_units_built_.size());
        ++count_units_built_[unit->unit_type];
        const sc2::ObservationInterface* obs = Observation();
        if (unit->unit_type != sc2::UNIT_TYPEID::ZERG_DRONE &&
            unit->unit_type != sc2::UNIT_TYPEID::ZERG_LARVA &&
            unit->unit_type != sc2::UNIT_TYPEID::TERRAN_SCV &&
            unit->unit_type != sc2::UNIT_TYPEID::PROTOSS_PROBE

        ) {
            std::cout << "- GameLoop: " << obs->GetGameLoop() << "; Minerals: " << minerals << "; Gas: " << gas << "; Supply: " << obs->GetFoodUsed() << " / "
                      << obs->GetFoodCap() << " " << UnitTypeToName(unit->unit_type) << std::endl;
        }
    }

    void OnStep() final {
        const sc2::ObservationInterface* obs = Observation();
        minerals = obs->GetMinerals();
        gas = obs->GetVespene();
    }

    void OnGameEnd() final {
        std::cout << std::endl;
        std::cout << "Units created:" << std::endl;
        const sc2::ObservationInterface* obs = Observation();
        const sc2::UnitTypes& unit_types = obs->GetUnitTypeData();
        for (uint32_t i = 0; i < count_units_built_.size(); ++i) {
            if (count_units_built_[i] == 0) {
                continue;
            }

            std::cout << unit_types[i].name << ": " << std::to_string(count_units_built_[i]) << std::endl;
        }
        std::cout << std::endl;
        std::cout << "Game Finished" << std::endl;

        // Who won?
        sc2::GameInfo game_info = obs->GetGameInfo();
        std::vector<sc2::PlayerInfo> player_information = game_info.player_info;
        std::vector<sc2::PlayerResult> player_result = obs->GetResults();

        for (uint32_t i = 0; i < player_result.size() && i < player_information.size();++i) {
            sc2::GameResult game_result = player_result[i].result;
            sc2::PlayerInfo player = player_information[i];

            if (game_result == sc2::GameResult::Loss) {
                std::cout << "player_id: " << player.player_id << " result: " << "Loss" << std::endl;
            } else if (game_result == sc2::GameResult::Win) {
                std::cout << "player_id: " << player.player_id << " result: " << "Win" << std::endl;
            } else if (game_result == sc2::GameResult::Undecided) {
                std::cout << "player_id: " << player.player_id << " result: " << "Undecided" << std::endl;
            } else {
                std::cout << "player_id: " << player.player_id << " result: " << "Tie" << std::endl;
            }
        }

        std::cout  << std::endl;

        // Add the counter to print only the current replay players races
        counter+=player_result.size();

        // Display additional information about the replay
        sc2::ReplayInfo replay_info = ReplayControl()->GetReplayInfo();
        std::cout << "duration_seconds: " << replay_info.duration << std::endl;
        std::cout << "players_1_race: " << replay_info.players[0].race << std::endl;
        std::cout << "players_2_race: " << replay_info.players[1].race << std::endl;
        std::cout << "duration_game_loops: " << replay_info.duration_gameloops << std::endl;
        std::cout << "game_version: " << replay_info.version << std::endl;
        std::cout << "data_build: " << replay_info.data_build << std::endl;
        std::cout << "base_build: " << replay_info.base_build << std::endl;
        std::cout << "data_version: " << replay_info.data_version << std::endl;
        std::cout  << std::endl;
        std::cout << "Additional player info " << std::endl;
        std::cout << "player_mmr: " << replay_info.players->mmr << std::endl;
        std::cout << "player_apm: " << replay_info.players->apm << std::endl;
        std::cout  << std::endl;
    }
};


int main(int argc, char* argv[]) {
    sc2::Coordinator coordinator;
    if (!coordinator.LoadSettings(argc, argv)) {
        return 1;
    }

    if (!coordinator.SetReplayPath(kReplayFolder)) {
        std::cout << "Unable to find replays." << std::endl;
        return 1;
    }

    Replay replay_observer;
    coordinator.AddReplayObserver(&replay_observer);
    // coordinator.SetStepSize(3);

    while (coordinator.Update());
    while (!sc2::PollKeyPress());
}