    
std::string example_geom_frag = """
layout(location = 0) out vec4 outDiffuse;
layout(location = 1) out vec4 outNormal;


main() {

    outDiffuse = vec4(1.0,0.0,1.0,1.0);
    outNormal = vNormal;
}

"""


std::string example_light_frag = """

main() {
    
    vec3 color = texture(diffuse, xy) * dot(light,texture(normal,xy));

    outNormal = vNormal;
}


"""

constructor() {
    GLUint diffuse,diffuseTexture;
    glCreateFramebuffers(1, &diffuse);
    glCreateTextures(GL_TEXTURE_2D, 1, &diffuseTexture);
    glTextureStorage2D(colorTexture, 1, GL_RGBA16F, width, height);
    glTextureParameteri(colorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(colorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(colorTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(colorTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(diffuse, GL_COLOR_ATTACHMENT0, diffuseTexture, 0);

    GLUint normal,normalTexture;
    glCreateFramebuffers(1, &normal);
    glCreateTextures(GL_TEXTURE_2D, 1, &normalTexture);
    glTextureStorage2D(colorTexture, 1, GL_RGB16F, width, height);
    glTextureParameteri(colorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(colorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(colorTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(colorTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glNamedFramebufferTexture(normal, GL_COLOR_ATTACHMENT1, normalTexture, 0);

    GLUint depth,depthTexture;
    glCreateFramebuffers(1, &depth);
    glCreateTextures(GL_TEXTURE_2D, 1, &depthTexture);
    glTextureStorage2D(depthTexture, 1, GL_DEPTH_COMPONENT32F, width, height);
    glTextureParameteri(depthTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(depthTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(depthTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(depthTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
}

void operator(
        const std::vector<>& renderComponent,
        const std::vector<>& transformComponent,
        const std::vector<>& lightComponent,
        const Camera &cam) {

        Framebuffer diffuseout, normalout, depthout; // Mismo tamaño que viewport

        // geometry pass
        bind(diffuseout);
        bind(normalout);
        bind(depthout);
        for(model,transform : renderComponent, transformComponent) {
            bind(transform);
            bind(modelTextures);
            glDrawElements(model);
        }


        for(light,lightTransform : lightComponent,transformComponent) {

            if(firstpass) { // Redundante porque es igual que en el render de sombras
                OverwritePassSettings();
            } else {
               BlendPassSettings();
            }
            updateViewport(default); // Framebuffer por defecto, la pantalla
            bind(default); // Framebuffer por defecto, la pantalla
            bind(lightProgram);
            bind(cameraTransform);
            bind(light);
            bind(ShadowTexture);

            glNamedFramebufferTexture(diffuseout, GL_COLOR_ATTACHMENT0, depthTexture, 0);
            glNamedFramebufferTexture(normalout, GL_COLOR_ATTACHMENT1, depthTexture, 0);
            glNamedFramebufferTexture(depthout, GL_DEPTH_ATTACHMENT, depthTexture, 0);

            bind(diffuseout);
            bind(normalout);
            bind(depthout);

            glDrawTriangles(quad);
            
        }
    }