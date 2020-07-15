#pragma once

struct shaders {
    static const char *VertexShaderPoints() {
        return R"(
#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;

attribute vec4 a_position;
attribute vec4 a_color;

varying vec4 v_color;

void main()
{
    gl_Position = mvp_matrix * a_position;
    v_color = a_color;
}
)";
    }

    static const char *FragmentShaderPoints() { return R"(
#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

varying vec4 v_color;

void main()
{    
    gl_FragColor = v_color;
}
)";
    }
};