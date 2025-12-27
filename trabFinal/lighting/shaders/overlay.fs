#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 overlayColor;
uniform float alpha;
uniform float time;
uniform int useTexture;
uniform sampler2D imageTexture;

void main()
{
    if (useTexture == 1) {
        // Modo textura - renderizar imagem
        vec4 texColor = texture(imageTexture, TexCoord);
        FragColor = vec4(texColor.rgb, texColor.a * alpha);
    } else {
        // Modo overlay animado (vitória)
        // Create a vignette effect - darker at edges, brighter in center
        vec2 center = vec2(0.5, 0.5);
        float dist = distance(TexCoord, center);
        float vignette = smoothstep(0.8, 0.3, dist);
        
        // Pulsing effect based on time
        float pulse = sin(time * 2.0) * 0.2 + 0.8;
        
        // Radial wave effect
        float wave = sin(dist * 10.0 - time * 3.0) * 0.1 + 0.9;
        
        vec3 finalColor = overlayColor * vignette * pulse * wave;
        FragColor = vec4(finalColor, alpha * vignette);
    }
}
