#pragma once

#include "particle.h"
#include "utils/track.h"
#include "resources/resource.h"

class FastNoiseSIMD;

namespace Particles
{
    using TParticlesHandle = int;
    using VParticles = std::vector<TParticle>;
    
    struct TNBurst {
        float   time;
        int     count;
        int     cycles;
        float   interval;
        float   i_elapsed = 0;
    };

    struct TCoreSystem : public IResource
    {
        void destroy() override;
        void onFileChanged(const std::string& filename) override;

        struct TNSystem 
        {
            float duration = 1.f;  
            bool  looping = false; 
            float start_delay = 0.f;
            float start_lifetime = 0.f;
            float start_speed = 0.f;

            VEC3  start_size;
            VEC3  start_rotation;
            float random_rotation = 0.f;
            VEC4  start_color;

            float gravity = 0.2f;
            float simulation_speed = 1.f;
            int   max_particles = 1; 

            VEC3 offset;
        };

        struct TNEmission
        {
            float rate_time = 10.f;
            float rate_distance = 0.f;
            float variation = 0.1f;

            float time_ratio;
            std::vector<TNBurst> bursts;
            // Add Bursts in the future.
        };

        struct TNShape {

            enum EType { Point, Line, Square, Box, Sphere, Circle, Cone };
            EType type = Point;             // type of emissor
            VEC3 size = VEC3(1, 1, 1);      // emissor size
            float angle = 0.f;              // emission angle
            bool shell_emit = false;
        };

        struct TNVelocity {

            enum EType { LOCAL = 0, WORLD };
            EType type = LOCAL;             // type of emissor

            VEC3 constant_velocity = VEC3::Zero;
            TTrack<VEC3> velocity;
            TTrack<VEC3> rotation;

            float angular = 0.f;
            float acceleration = 0.f;
            float wind = 0.f;
            bool inherit_velocity = false;
        };

        struct TNColor {

            TTrack<VEC4> colors;            // track of colors along the particle lifetime
            float opacity = 1.f;            // opacity factor
        };

        struct TNSize
        {
            TTrack<VEC3> sizes;            // track of sizes along the particle lifetime
            float scale = 1.f;              // scale factor
            float scale_variation = 0.f;    // variation of scale at generation
        };

        struct TNoise
        {       
            FastNoiseSIMD * noise_core = nullptr;
            float* noise_values = nullptr;

            const CTexture* texture = nullptr; // particle texture
            float strength = 0.f;
            float frequency = 0.5f;
            float scroll_speed = 0.f;
            bool damping = true;
            int octaves = 1;
        };

        struct TNCollision
        {
            bool collision = true;
        };

        struct TNLight {

        };

        struct TNRenderer
        {
            enum EMODE { BILLBOARD = 0, HORIZONTAL, VERTICAL, STRETCHED };
            EMODE mode = BILLBOARD;

            const CTexture* texture = nullptr; // particle texture
            VEC2 frameSize = VEC2(1, 1);       // size of frame in the texture (in UV coords)
            int numFrames = 1;                 // number of animation frames
            int initialFrame = 0;              // initial frame
            float frameSpeed = 0.f;            // frame change speed
            float length = 1;
            float softness = 1;

            std::string tech;
        };

        TNSystem        n_system;
        TNEmission      n_emission;
        TNShape         n_shape;
        TNVelocity      n_velocity;
        TNColor         n_color;
        TNSize          n_size;
        TNoise          n_noise;
        TNCollision     n_collision;
        TNRenderer      n_renderer;
    };

    class CSystem
    {
    public:
        
        // Blending options
        enum EType { COM, ADD, MUL };
        EType type;

        bool                _destroy_entity = false;

        CSystem(const TCoreSystem* core, CHandle entity);

        bool update(float delta);
        void render();
        void launch();
        void debugInMenu();

        void fadeOut(float duration);
        void setActive(bool active);
        TParticlesHandle getHandle() const;

    private:

        static TParticlesHandle _lastHandle;

        void emit(int amount);
        void updateSystem(float delta);
        void updateFading(float delta);
        void updateEmission(float delta);
        void updateCollision(float delta);

        VEC3 generatePosition() const;
        VEC3 generateVelocity() const;
        VEC3 generateDirection() const;
        void getRenderMode(VEC3 & camera_pos, VEC3 & camera_up, float & length);

        VEC3 AddNoiseOnAngle(float min, float max);

        CHandle             _entity;
        TParticlesHandle    _handle;
        VParticles          _particles;
        const TCoreSystem*  _core = nullptr;

        float               _time = 0.f;
        float               _deploy_time = 0.f;
        float               _fadeDuration = 0.f;
        float               _fadeTime = 0.f;
        float               _fadeRatio = 0.f;
        bool                _enabled;

        // Temp variables
        float time_ratio;
        float _deploy_distance;
        VEC3 _lastSystemPosition;

        std::vector<TNBurst> bursts;
    };

}
