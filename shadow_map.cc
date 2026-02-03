

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