#pragma once

struct VertexWithColor {
    float x, y, z, w;
    float r, g, b, a;
};

struct VertexWithUV {
    float x, y, z, w;
    float u, v;
};

static const VertexWithColor triangleData[] = {
        {0.0f,  1.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0},
        {1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0},
        {-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0},
};

static const VertexWithColor squareData[] = {
        {-0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0},
        {0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0},
        {0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0},
        {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0},
};

uint16_t squareIndices[] = { 0,3,1, 3,2,1 }; // 6 indices