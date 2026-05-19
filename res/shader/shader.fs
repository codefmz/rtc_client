#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texY;
uniform sampler2D texUV;

void main()
{
    float y = texture(texY, TexCoord).r;
    vec2 uv = texture(texUV, TexCoord).rg - vec2(0.5, 0.5);
    float r = y + 1.402 * uv.y;
    float g = y - 0.344136 * uv.x - 0.714136 * uv.y;
    float b = y + 1.772 * uv.x;
    FragColor = vec4(r, g, b, 1.0);
}