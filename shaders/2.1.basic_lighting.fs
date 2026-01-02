#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;

uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform float lightIntensity; 
uniform vec3 topLightPos; 
uniform vec3 objectColor;
uniform sampler2D wallTexture;
uniform sampler2D floorTexture;
uniform sampler2D gateTexture;
uniform int objectType; // 0 = Labirinto, 1 = Portão

// Uniformes da Lanterna
uniform vec3 viewPos;
uniform vec3 flashLightDir;
uniform float flashLightCutoff;
uniform float flashLightOuterCutoff;
uniform bool flashLightOn;

void main(){
    vec3 topLightColor = vec3(1.0, 1.0, 0.8);

    // Cor inicial da textura
    vec3 texColor;
    
    if (objectType == 1) {
        // Portão
        texColor = texture(gateTexture, TexCoords).rgb;
    } else {
        // Labirinto (Paredes/Chão)
        if (Normal.y > 0.5) {
            texColor = texture(floorTexture, TexCoords).rgb;
        } else {
            texColor = texture(wallTexture, TexCoords).rgb;
        }
    }
    
    // Ambiente (aumentado para depuração de paredes escuras)
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * topLightColor;

    // Difusa (Luz Superior) - usando abs() para funcionar com normais invertidas
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(topLightPos - FragPos);
    float diff = abs(dot(norm, lightDir));
    vec3 diffuse = diff * topLightColor * lightIntensity;

    // Lanterna (Spotlight)
    vec3 flashlight = vec3(0.0);
    if(flashLightOn) {
        // Direção do fragmento para a câmara (fonte de luz)
        vec3 lightDirFlash = normalize(viewPos - FragPos);
        
        // Verificação do cone da spotlight (usando -flashLightDir como estava a funcionar antes)
        float theta = dot(lightDirFlash, normalize(-flashLightDir));
        
        // Suavização nas bordas do cone
        float epsilon = flashLightCutoff - flashLightOuterCutoff;
        float intensity = clamp((theta - flashLightOuterCutoff) / epsilon, 0.0, 1.0);
        
        if(theta > flashLightOuterCutoff) {
            // Usar abs() para fazer a luz funcionar em ambos os lados das superfícies (corrige normais invertidas)
            float diffFlash = abs(dot(norm, lightDirFlash));
            flashlight = vec3(1.0, 1.0, 1.0) * diffFlash * intensity * 2.0;
        }
    }

    // Multiplicar iluminação com cor da textura
    // Usar texColor diretamente dá melhor visibilidade da textura
    vec3 result = (ambient + diffuse + flashlight) * texColor;
    FragColor = vec4(result, 1.0);
}