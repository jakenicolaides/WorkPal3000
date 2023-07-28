#pragma once

#include <vector>
#include <fstream>

namespace UnseenEngine {

    const int maxNumberOfEntities = 100;
    const std::string ENGINE_TITLE = "WorkPal 3000 - [^_^]";
    const std::string GAME_TITLE = "Spirit Lens";
    void engineLoop();
    void initEngine();
    void launchGame();

    struct Entities {
        int id[maxNumberOfEntities];
    };

}