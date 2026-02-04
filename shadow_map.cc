

void setFirstPassMultiPass() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);  // puede ser GL_LEQUAL
    glDepthMask(GL_TRUE);  // escribe en zbuffer
    
    glDisable(GL_BLEND);  
}
void setSecondPassMultiPass() {
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_EQUAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
}

void setupFrameBuffer() {
    unsigned int FBO;
    unsigned int depthTexture;
    unsigned int width;
    unsigned int height;
    
    glCreateFramebuffers(1, &FBO);


    // glCreateTextures(GL_TEXTURE_2D, 1, &colorTexture);
    // glTextureStorage2D(colorTexture, 1, GL_RGBA16F, width, height);
    // glTextureParameteri(colorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTextureParameteri(colorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTextureParameteri(colorTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTextureParameteri(colorTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, colorTexture, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &depthTexture);
    glTextureStorage2D(depthTexture, 1, GL_DEPTH_COMPONENT32F, width, height);
    glTextureParameteri(depthTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(depthTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(depthTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(depthTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTextureParameterfv(depthTexture, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Attach depth texture to framebuffer using DSA
    glNamedFramebufferTexture(FBO, GL_DEPTH_ATTACHMENT, depthTexture, 0);

    glNamedFramebufferDrawBuffer(FBO, GL_NONE);
    glNamedFramebufferReadBuffer(FBO, GL_NONE);

    GLenum status = glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) std::abort();
}

void useFB() {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, width, height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    glUniform1i(glGetUniformLocation(shader, "diffuse"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depth);
    glUniform1i(glGetUniformLocation(shader, "depth"), 1);

    //...
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
}
// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
// https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions
// https://ktstephano.github.io/rendering/opengl/dsa
// https://juandiegomontoya.github.io/modern_opengl.html

void PhongRenderSystem(const std::vector<RenderComponent>& rcv,
    const std::vector<TransformComponent>& tcv,
    const std::vector<LightComponent>& lcv) {

        std::vector<std::pair<TransformComponent,LightComponent>> lightCache;
        for(auto [tc,lc] : getComponents(tcv,lcv) {
            if(!tc || !lc) continue;
            lightCache.push_back(std::make_pair(rc,lc));
        }


        setProgram(phongShader_);

        setBlend(false);
        setzwrite(true);
        glClear(...);
        
        bool first = true;
        for(auto [light,lightTransform] : lightCache) {

            // bind uniforms from light
            // bind uniforms transform from lightTransform

            for(auto [rc,tc] : getComponents(rcv,tcv) {
                if(!rc || !rc) continue;

                // bind uniforms from rc.Mesh
                // bind uniforms from rc.texture

                   glDraw(...);
            }
            if(first) {
                first = false;
                glBlend(true);
                disableZWrite();
                ...
            }
        }

    }



    void ShadowMapRenderSystem(const std::vector<RenderComponent>& rcv,
    const std::vector<TransformComponent>& tcv,
    const std::vector<LightComponent>& lcv) {

        std::vector<std::pair<TransformComponent,LightComponent>> lightCache;
        for(auto [tc,lc] : getComponents(tcv,lcv) {
            if(!tc || !lc) continue;
            lightCache.push_back(std::make_pair(rc,lc));
        }


        setProgram(phongShader_);

        setBlend(false);
        setzwrite(true);
        glClear(...);
        
        bool first = true;
        for(auto [light,lightTransform] : lightCache) {

            // bind uniforms transform from lightTransform AS CAMERA
            // Bind texture as shadow framebuffer
            for(auto [rc,tc] : getComponents(rcv,tcv) {
                if(!rc || !rc) continue;

                // bind uniforms from rc.Mesh
                // bind uniforms from rc.texture

                   glDraw(...);
            }

            // Unbind shadow framebuffer
            // bind default framebuffer
            // bind shadow texture as texture
            // bind uniforms from light
            // bind uniforms transform from lightTransform
            for(auto [rc,tc] : getComponents(rcv,tcv) {
                if(!rc || !rc) continue;

                // bind uniforms from rc.Mesh
                // bind uniforms from rc.texture

                   glDraw(...);
            }
            if(first) {
                first = false;
                glBlend(true);
                disableZWrite();
                ...
            }
        }

    }