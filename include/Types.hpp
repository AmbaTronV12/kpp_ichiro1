#pragma once

struct Position {
    float x;
    float y;
};

enum class State { 
    SEARCH_BALL,
    APPROACH_BALL,
    ALIGN_TO_GOAL,
    KICK_BALL
};