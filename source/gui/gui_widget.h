#pragma once

#include "gui_params.h"
#include "gui_effect.h"

namespace GUI {

    class CWidget;
    using VWidgets = std::vector<CWidget*>;

    struct ConfigParams {
        VEC2 minUV;
        VEC2 maxUV;
        VEC4 color;
        float var;
    };

    class CWidget {

    public:
        CWidget();

        void addChild(CWidget* wdgt);
        void removeChild(CWidget* wdgt);
        CWidget* getChild(const std::string& name, bool recursive = false) const;
		VWidgets getAllChilds();
        const std::string& getName() const;
        virtual TImageParams* getImageParams() { return nullptr; }
        virtual TBarParams* getBarParams() { return nullptr; }
        virtual TTextParams* getTextParams() { return nullptr; }
		virtual TSpriteParams* getSpriteParams() { return nullptr; }
		virtual TSubtitlesParams* getSubtitleParams() { return nullptr; }
        virtual TParams* getTParams() { return nullptr; }
		
        void addEffect(CEffect* fx);

        void computeLocal();
        void computeAbsolute();

        void updateAll(float delta);
        void renderAll();
        void enable(bool status = true);

        virtual void update(float delta);
        virtual void render();

        MAT44 getAbsolute() { return _absolute; };

        bool isEnabled() { return enabled; };

		void makeChildsFadeIn(float time_to_lerp, float time_to_start = 0.0f, bool getFromChildren = false);
		void makeChildsFadeOut(float time_to_lerp, float time_to_start = 0.0f, bool getFromChildren = false);

    protected:
        std::string _name;
        VWidgets _children;
        VEffects _effects;
        CWidget* _parent = nullptr;
        TParams _params;
        MAT44 _local;
        MAT44 _absolute;
        bool enabled;

        friend class CParser;
    };
}