#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  

uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform float lightIntensity; 
uniform vec3 topLightPos; 
uniform vec3 objectColor;

// Flashlight uniforms
uniform vec3 viewPos;
uniform vec3 flashLightDir;
uniform float flashLightCutoff;
uniform float flashLightOuterCutoff;
uniform bool flashLightOn;

void main(){
    vec3 topLightColor = vec3(1.0, 1.0, 0.8);

    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * topLightColor;

    // Diffuse (Top Light)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(topLightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * topLightColor * lightIntensity;

    // Flashlight (Spotlight)
    vec3 flashlight = vec3(0.0);
    if(flashLightOn) {
        vec3 flashDir = normalize(viewPos - FragPos); // Direction from frag to eye (light source)
        // Actually, for spotlight, we need direction from light to frag.
        // But standard spotlight math:
        vec3 lightDirFlash = normalize(viewPos - FragPos);
        float theta = dot(lightDirFlash, normalize(-flashLightDir));
        float epsilon = flashLightCutoff - flashLightOuterCutoff;
        float intensity = clamp((theta - flashLightOuterCutoff) / epsilon, 0.0, 1.0);
        
        if(theta > flashLightOuterCutoff) {
            float diffFlash = max(dot(norm, lightDirFlash), 0.0);
            flashlight = vec3(1.0, 1.0, 1.0) * diffFlash * intensity * 2.0; // 2.0 brightness
        }
    }

    vec3 result = (ambient + diffuse + flashlight) * objectColor;
    FragColor = vec4(result, 1.0);
}