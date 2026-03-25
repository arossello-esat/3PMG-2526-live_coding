float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness)
         * GeometrySchlickGGX(NdotL, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


void main() {
    vec4 albedoMetallic   = texture(gAlbedoMetallic, TexCoords);
    vec4 normalRoughness  = texture(gNormalRoughness, TexCoords);
    vec4 positionAO       = texture(gPositionAO, TexCoords);

    vec3  albedo    = albedoMetallic.rgb;
    float metallic  = albedoMetallic.a;
    vec3  N         = normalize(normalRoughness.rgb);
    float roughness = normalRoughness.a;
    vec3  fragPos   = positionAO.rgb;
    float ao        = positionAO.a;

    vec3 V = normalize(uCamPos - fragPos);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 L = normalize(uLightPos - fragPos);
    vec3 H = normalize(V + L);
    float distance    = length(uLightPos - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3  radiance    = uLightColor * uLightIntensity * attenuation;

    // BRDF
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3  F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    // Specular
    vec3 numerator  = D * G * F;
    float denom     = 4.0 * max(dot(N, V), 0.0)
                          * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular   = numerator / denom;

    // Diffuse (metals have no diffuse)
    vec3 kd = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kd * albedo / PI;

    float NdotL = max(dot(N, L), 0.0);

    vec3 Lo = (diffuse + specular) * radiance * NdotL * (1.0 - shadow);

    // Ambient EXTRA
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;
}


// LearnOpenGL PBR: https://learnopengl.com/PBR/Theory

