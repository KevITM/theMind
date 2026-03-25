#ifndef THEMIND_PLAY_RESULT_HPP
#define THEMIND_PLAY_RESULT_HPP

namespace themind {

    enum class PlayResult {
        ValidPlay,
        LevelUp,
        LostLife,
        GameOver,
        InvalidCard,
        LostLifeAndLevelUp
    };

}

#endif // THEMIND_PLAY_RESULT_HPP
