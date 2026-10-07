#pragma once

struct Position {
    float x;
    float y;
};

enum class State { 
    SEARCH_BALL,
    APPORACH_BALL,
    ALIGN_TO_GOAL,
    KICK_BALL
};