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

    // Ambient (increased for debugging dark walls)
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * topLightColor;

    // Diffuse (Top Light) - using abs() to work with inverted normals
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(topLightPos - FragPos);
    float diff = abs(dot(norm, lightDir));
    vec3 diffuse = diff * topLightColor * lightIntensity;

    // Flashlight (Spotlight)
    vec3 flashlight = vec3(0.0);
    if(flashLightOn) {
        // Direction from fragment to camera (light source)
        vec3 lightDirFlash = normalize(viewPos - FragPos);
        
        // Spotlight cone check (using -flashLightDir as it was working before)
        float theta = dot(lightDirFlash, normalize(-flashLightDir));
        
        // Smooth falloff at cone edges
        float epsilon = flashLightCutoff - flashLightOuterCutoff;
        float intensity = clamp((theta - flashLightOuterCutoff) / epsilon, 0.0, 1.0);
        
        if(theta > flashLightOuterCutoff) {
            // Use abs() to make light work on both sides of surfaces (fixes inverted normals)
            float diffFlash = abs(dot(norm, lightDirFlash));
            flashlight = vec3(1.0, 1.0, 1.0) * diffFlash * intensity * 2.0;
        }
    }

    vec3 result = (ambient + diffuse + flashlight) * objectColor;
    FragColor = vec4(result, 1.0);
}